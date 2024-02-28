//
// Created by consti10 on 13.09.23.
//

#include "wb_link_manager.h"

#include <cstring>
#include <sstream>

#include "openhd_global_constants.hpp"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_time.h"

static constexpr uint8_t MNGMNT_PACKET_ID_CHANNEL_WIDTH = 0;
static constexpr uint8_t MNGMNT_PACKET_ID_SENSITVITY_STATUS = 1;
struct DataManagementTxBandwidth {
  uint32_t center_frequency_mhz;
  uint8_t bandwidth_mhz;
} __attribute__((packed));
struct DataManagementSensitivityStatus {
  uint16_t dummy_0;
  uint16_t dummy_1;
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
    DataManagementTxBandwidth packet{};
    std::memcpy(&packet, &data[1], data_len - 1);
    if (packet.bandwidth_mhz == 20 || packet.bandwidth_mhz == 40) {
      m_air_reported_curr_channel_width = packet.bandwidth_mhz;
      m_air_reported_curr_frequency = packet.center_frequency_mhz;
    } else {
      m_console->warn("Air reports invalid bandwidth {}", packet.bandwidth_mhz);
    }
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
    // m_console->debug("Sent sensitivity management frame");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int ManagementGround::get_last_received_packet_ts_ms() {
  return m_last_received_packet_timestamp_ms;
}
