/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "wb_link_manager.h"

#include <cstring>
#include <sstream>

#include "openhd_global_constants.hpp"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_time.h"

static constexpr uint8_t MNGMNT_PACKET_ID_CHANNEL_WIDTH = 0;
static constexpr uint8_t MNGMNT_PACKET_ID_SENSITVITY_STATUS = 1;
static constexpr uint8_t MNGMNT_PACKET_ID_FREQUENCY_CHANGE = 2;
static constexpr uint8_t MNGMNT_PACKET_ID_FREQUENCY_ACK = 3;
static constexpr uint8_t FREQUENCY_CHANGE_PHASE_PREPARE = 1;
static constexpr uint8_t FREQUENCY_CHANGE_PHASE_COMMIT = 2;
static constexpr uint8_t FREQUENCY_ACK_READY = 1;
static constexpr uint8_t FREQUENCY_ACK_CONFIRMED = 2;
static constexpr uint8_t FREQUENCY_ACK_FAILED = 3;
struct DataManagementTxBandwidth {
  uint32_t center_frequency_mhz;
  uint8_t bandwidth_mhz;
} __attribute__((packed));
struct DataManagementSensitivityStatus {
  uint16_t dummy_0;
  uint16_t dummy_1;
} __attribute__((packed));
struct DataManagementFrequencyChange {
  uint32_t transaction_id;
  uint32_t old_frequency_mhz;
  uint32_t target_frequency_mhz;
  uint8_t bandwidth_mhz;
  uint8_t phase;
} __attribute__((packed));
struct DataManagementFrequencyAck {
  uint32_t transaction_id;
  uint32_t target_frequency_mhz;
  uint8_t state;
} __attribute__((packed));
static std::vector<uint8_t> pack_management_frame(
    const DataManagementTxBandwidth &data) {
  std::vector<uint8_t> ret;
  ret.resize(1 + sizeof(data));
  ret[0] = MNGMNT_PACKET_ID_CHANNEL_WIDTH;
  std::memcpy(&ret[1], (void *)&data, sizeof(DataManagementTxBandwidth));
  return ret;
}
static std::vector<uint8_t> pack_management_frame(
    const DataManagementSensitivityStatus &data) {
  std::vector<uint8_t> ret;
  ret.resize(1 + sizeof(data));
  ret[0] = MNGMNT_PACKET_ID_SENSITVITY_STATUS;
  std::memcpy(&ret[1], (void *)&data, sizeof(DataManagementSensitivityStatus));
  return ret;
}
static std::vector<uint8_t> pack_management_frame(
    const DataManagementFrequencyChange &data) {
  std::vector<uint8_t> ret(1 + sizeof(data));
  ret[0] = MNGMNT_PACKET_ID_FREQUENCY_CHANGE;
  std::memcpy(&ret[1], &data, sizeof(data));
  return ret;
}
static std::vector<uint8_t> pack_management_frame(
    const DataManagementFrequencyAck &data) {
  std::vector<uint8_t> ret(1 + sizeof(data));
  ret[0] = MNGMNT_PACKET_ID_FREQUENCY_ACK;
  std::memcpy(&ret[1], &data, sizeof(data));
  return ret;
}

static std::string management_frame_to_string(
    const DataManagementTxBandwidth &data) {
  return fmt::format("Center: {}Mhz BW:{}Mhz", (int)data.center_frequency_mhz,
                     (int)data.bandwidth_mhz);
}

ManagementAir::ManagementAir(std::shared_ptr<WBTxRx> wb_tx_rx,
                             int initial_freq_mhz, int inital_channel_width_mhz)
    : m_wb_txrx(std::move(wb_tx_rx)),
      m_curr_frequency_mhz(initial_freq_mhz),
      m_curr_channel_width_mhz(inital_channel_width_mhz),
      m_last_change_timestamp_ms{openhd::util::steady_clock_time_epoch_ms()} {
  auto transaction_seed = static_cast<uint32_t>(
      openhd::util::steady_clock_time_epoch_ms());
  if (transaction_seed == 0) transaction_seed = 1;
  m_next_frequency_transaction_id = transaction_seed;
  m_console = openhd::log::create_or_get("wb_mngmt_air");
  auto cb_packet = [this](uint64_t nonce, int wlan_index, const uint8_t *data,
                          const int data_len) {
    this->on_new_management_packet(data, data_len);
  };
  auto mgmt_handler = std::make_shared<WBTxRx::StreamRxHandler>(
      openhd::MANAGEMENT_RADIO_PORT_GND_TX, cb_packet, nullptr);
  m_wb_txrx->rx_register_stream_handler(mgmt_handler);
}

int ManagementAir::get_last_received_packet_ts_ms() {
  return m_last_received_packet_timestamp_ms;
}
void ManagementAir::set_frequency(int frequency) {
  m_curr_frequency_mhz = frequency;
  m_last_change_timestamp_ms = openhd::util::steady_clock_time_epoch_ms();
}

void ManagementAir::set_channel_width(uint8_t bw) {
  m_curr_channel_width_mhz = bw;
  m_last_change_timestamp_ms = openhd::util::steady_clock_time_epoch_ms();
}

uint32_t ManagementAir::begin_frequency_change(int target_frequency,
                                               uint8_t channel_width) {
  uint32_t transaction_id = m_next_frequency_transaction_id.fetch_add(1);
  if (transaction_id == 0) {
    transaction_id = m_next_frequency_transaction_id.fetch_add(1);
  }
  m_ground_ready_transaction_id = 0;
  m_ground_confirmed_transaction_id = 0;
  m_ground_failed_transaction_id = 0;
  m_frequency_transaction_old_mhz = m_curr_frequency_mhz.load();
  m_frequency_transaction_target_mhz = target_frequency;
  m_frequency_transaction_width_mhz = channel_width;
  m_frequency_transaction_id = transaction_id;
  m_frequency_transaction_phase = FREQUENCY_CHANGE_PHASE_PREPARE;
  m_last_change_timestamp_ms = openhd::util::steady_clock_time_epoch_ms();
  return transaction_id;
}

void ManagementAir::commit_frequency_change(uint32_t transaction_id) {
  if (m_frequency_transaction_id.load() != transaction_id) return;
  m_frequency_transaction_phase = FREQUENCY_CHANGE_PHASE_COMMIT;
  // Legacy grounds only understand packet 0. Advertising the target during the
  // commit grace period lets them move before air changes its radio.
  m_curr_frequency_mhz = m_frequency_transaction_target_mhz.load();
  m_last_change_timestamp_ms = openhd::util::steady_clock_time_epoch_ms();
}

void ManagementAir::finish_frequency_change(uint32_t transaction_id,
                                            bool success,
                                            int fallback_frequency) {
  if (m_frequency_transaction_id.load() != transaction_id) return;
  m_curr_frequency_mhz =
      success ? m_frequency_transaction_target_mhz.load() : fallback_frequency;
  m_frequency_transaction_phase = 0;
  m_frequency_transaction_id = 0;
  m_last_change_timestamp_ms = openhd::util::steady_clock_time_epoch_ms();
}

bool ManagementAir::is_frequency_change_ready(
    uint32_t transaction_id) const {
  return m_ground_ready_transaction_id.load() == transaction_id;
}

bool ManagementAir::is_frequency_change_confirmed(
    uint32_t transaction_id) const {
  return m_ground_confirmed_transaction_id.load() == transaction_id;
}

bool ManagementAir::has_frequency_change_failed(
    uint32_t transaction_id) const {
  return m_ground_failed_transaction_id.load() == transaction_id;
}

void ManagementAir::start() {
  m_tx_thread_run = true;
  m_tx_thread = std::make_unique<std::thread>(&ManagementAir::loop, this);
}

ManagementAir::~ManagementAir() {
  m_wb_txrx->rx_unregister_stream_handler(openhd::MANAGEMENT_RADIO_PORT_GND_TX);
  m_tx_thread_run = false;
  m_tx_thread->join();
  m_tx_thread = nullptr;
}

void ManagementAir::loop() {
  while (m_tx_thread_run) {
    // Air: Continuously broadcast channel width
    // Calculate the interval in which we broadcast the channel width management
    // frame
    auto management_frame_interval =
        std::chrono::milliseconds(500);  // default 2Hz
    const auto elapsed_since_last_change_ms =
        openhd::util::steady_clock_time_epoch_ms() - m_last_change_timestamp_ms;
    if (elapsed_since_last_change_ms < 2 * 1000) {
      // If the last change is recent, send in higher interval
      management_frame_interval = std::chrono::milliseconds(20);
    }
    const auto elapsed_since_last_management_frame =
        std::chrono::steady_clock::now() - m_air_last_management_frame;
    if (elapsed_since_last_management_frame < management_frame_interval) {
      continue;
    }
    DataManagementTxBandwidth managementFrame{m_curr_frequency_mhz.load(),
                                              m_curr_channel_width_mhz.load()};
    auto data = pack_management_frame(managementFrame);
    auto radiotap_header = m_tx_header->thread_safe_get();
    m_wb_txrx->tx_inject_packet(openhd::MANAGEMENT_RADIO_PORT_AIR_TX,
                                data.data(), data.size(), radiotap_header,
                                true);
    const uint8_t transaction_phase = m_frequency_transaction_phase.load();
    if (transaction_phase != 0) {
      DataManagementFrequencyChange change{
          m_frequency_transaction_id.load(),
          m_frequency_transaction_old_mhz.load(),
          m_frequency_transaction_target_mhz.load(),
          m_frequency_transaction_width_mhz.load(), transaction_phase};
      auto change_data = pack_management_frame(change);
      m_wb_txrx->tx_inject_packet(openhd::MANAGEMENT_RADIO_PORT_AIR_TX,
                                  change_data.data(), change_data.size(),
                                  radiotap_header, true);
    }
    std::this_thread::sleep_for(management_frame_interval);
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void ManagementAir::on_new_management_packet(const uint8_t *data,
                                             int data_len) {
  if (data_len == sizeof(DataManagementSensitivityStatus) + 1 &&
      data[0] == MNGMNT_PACKET_ID_SENSITVITY_STATUS) {
    m_last_received_packet_timestamp_ms =
        openhd::util::steady_clock_time_epoch_ms();
    DataManagementSensitivityStatus packet{};
    std::memcpy(&packet, &data[1], data_len - 1);
    // TODO
  } else if (data_len == sizeof(DataManagementFrequencyAck) + 1 &&
             data[0] == MNGMNT_PACKET_ID_FREQUENCY_ACK) {
    m_last_received_packet_timestamp_ms =
        openhd::util::steady_clock_time_epoch_ms();
    DataManagementFrequencyAck packet{};
    std::memcpy(&packet, &data[1], sizeof(packet));
    const uint32_t active_transaction = m_frequency_transaction_id.load();
    if (packet.transaction_id != active_transaction ||
        packet.target_frequency_mhz !=
            m_frequency_transaction_target_mhz.load()) {
      return;
    }
    if (packet.state == FREQUENCY_ACK_READY) {
      m_ground_ready_transaction_id = packet.transaction_id;
    } else if (packet.state == FREQUENCY_ACK_CONFIRMED) {
      m_ground_confirmed_transaction_id = packet.transaction_id;
    } else if (packet.state == FREQUENCY_ACK_FAILED) {
      m_ground_failed_transaction_id = packet.transaction_id;
    }
  }
}

ManagementGround::ManagementGround(std::shared_ptr<WBTxRx> wb_tx_rx)
    : m_wb_txrx(std::move(wb_tx_rx)) {
  m_console = openhd::log::create_or_get("wb_mngmt_gnd");
  auto cb_packet = [this](uint64_t nonce, int wlan_index, const uint8_t *data,
                          const int data_len) {
    this->on_new_management_packet(data, data_len);
  };
  auto mgmt_handler = std::make_shared<WBTxRx::StreamRxHandler>(
      openhd::MANAGEMENT_RADIO_PORT_AIR_TX, cb_packet, nullptr);
  m_wb_txrx->rx_register_stream_handler(mgmt_handler);
}

ManagementGround::~ManagementGround() {
  m_wb_txrx->rx_unregister_stream_handler(openhd::MANAGEMENT_RADIO_PORT_AIR_TX);
  m_tx_thread_run = false;
  m_tx_thread->join();
  m_tx_thread = nullptr;
}

void ManagementGround::start() {
  m_tx_thread_run = true;
  m_tx_thread = std::make_unique<std::thread>(&ManagementGround::loop, this);
}

void ManagementGround::on_new_management_packet(const uint8_t *data,
                                                int data_len) {
  if (data_len == sizeof(DataManagementTxBandwidth) + 1 &&
      data[0] == MNGMNT_PACKET_ID_CHANNEL_WIDTH) {
    m_last_received_packet_timestamp_ms =
        openhd::util::steady_clock_time_epoch_ms();
    DataManagementTxBandwidth packet{};
    std::memcpy(&packet, &data[1], data_len - 1);
    if (packet.bandwidth_mhz == 10 || packet.bandwidth_mhz == 20 ||
        packet.bandwidth_mhz == 40) {
      m_air_reported_curr_channel_width = packet.bandwidth_mhz;
      m_air_reported_curr_frequency = packet.center_frequency_mhz;
    } else {
      m_console->warn("Air reports invalid bandwidth {}", packet.bandwidth_mhz);
    }
  } else if (data_len == sizeof(DataManagementFrequencyChange) + 1 &&
             data[0] == MNGMNT_PACKET_ID_FREQUENCY_CHANGE) {
    m_last_received_packet_timestamp_ms =
        openhd::util::steady_clock_time_epoch_ms();
    DataManagementFrequencyChange packet{};
    std::memcpy(&packet, &data[1], sizeof(packet));
    if ((packet.phase != FREQUENCY_CHANGE_PHASE_PREPARE &&
         packet.phase != FREQUENCY_CHANGE_PHASE_COMMIT) ||
        (packet.bandwidth_mhz != 10 && packet.bandwidth_mhz != 20 &&
         packet.bandwidth_mhz != 40) ||
        packet.target_frequency_mhz <= 100 || packet.transaction_id == 0) {
      m_console->warn("Invalid frequency change management packet");
      return;
    }
    m_frequency_transaction_old_mhz = packet.old_frequency_mhz;
    m_frequency_transaction_target_mhz = packet.target_frequency_mhz;
    m_frequency_transaction_width_mhz = packet.bandwidth_mhz;
    m_frequency_transaction_id = packet.transaction_id;
    // Publish the phase last so readers see a complete request.
    m_frequency_transaction_phase = packet.phase;
  }
}

void ManagementGround::loop() {
  while (m_tx_thread_run) {
    auto tmp = DataManagementSensitivityStatus{0, 0};
    auto data = pack_management_frame(tmp);
    auto radiotap_header = m_tx_header->thread_safe_get();
    m_wb_txrx->tx_inject_packet(openhd::MANAGEMENT_RADIO_PORT_GND_TX,
                                data.data(), data.size(), radiotap_header,
                                true);
    const uint8_t ack_state = m_ack_state.load();
    if (ack_state != 0) {
      DataManagementFrequencyAck ack{m_ack_transaction_id.load(),
                                    m_ack_target_frequency_mhz.load(),
                                    ack_state};
      auto ack_data = pack_management_frame(ack);
      m_wb_txrx->tx_inject_packet(openhd::MANAGEMENT_RADIO_PORT_GND_TX,
                                  ack_data.data(), ack_data.size(),
                                  radiotap_header, true);
    }
    // m_console->debug("Sent sensitivity management frame");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int ManagementGround::get_last_received_packet_ts_ms() {
  return m_last_received_packet_timestamp_ms;
}

std::optional<FrequencyChangeRequest>
ManagementGround::get_prepare_request() const {
  if (m_frequency_transaction_phase.load() !=
      FREQUENCY_CHANGE_PHASE_PREPARE) {
    return std::nullopt;
  }
  return FrequencyChangeRequest{
      m_frequency_transaction_id.load(),
      static_cast<int>(m_frequency_transaction_old_mhz.load()),
      static_cast<int>(m_frequency_transaction_target_mhz.load()),
      static_cast<int>(m_frequency_transaction_width_mhz.load())};
}

std::optional<FrequencyChangeRequest>
ManagementGround::get_commit_request() const {
  if (m_frequency_transaction_phase.load() != FREQUENCY_CHANGE_PHASE_COMMIT) {
    return std::nullopt;
  }
  return FrequencyChangeRequest{
      m_frequency_transaction_id.load(),
      static_cast<int>(m_frequency_transaction_old_mhz.load()),
      static_cast<int>(m_frequency_transaction_target_mhz.load()),
      static_cast<int>(m_frequency_transaction_width_mhz.load())};
}

void ManagementGround::mark_frequency_change_ready(uint32_t transaction_id,
                                                   int target_frequency) {
  m_ack_target_frequency_mhz = target_frequency;
  m_ack_transaction_id = transaction_id;
  m_ack_state = FREQUENCY_ACK_READY;
}

void ManagementGround::mark_frequency_change_switched(uint32_t transaction_id,
                                                      int target_frequency,
                                                      bool success) {
  m_ack_target_frequency_mhz = target_frequency;
  m_ack_transaction_id = transaction_id;
  m_ack_state = success ? FREQUENCY_ACK_READY : FREQUENCY_ACK_FAILED;
}

void ManagementGround::mark_frequency_change_confirmed(
    uint32_t transaction_id, int target_frequency) {
  m_ack_target_frequency_mhz = target_frequency;
  m_ack_transaction_id = transaction_id;
  m_ack_state = FREQUENCY_ACK_CONFIRMED;
}
