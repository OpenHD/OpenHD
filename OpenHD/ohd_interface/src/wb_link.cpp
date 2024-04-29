#include "wb_link.h"

#include "wifi_command_helper.h"
// #include "wifi_command_helper2.h"

#include <utility>

#include "openhd_bitrate.h"
#include "openhd_config.h"
#include "openhd_global_constants.hpp"
#include "openhd_platform.h"
#include "openhd_reboot_util.h"
#include "openhd_spdlog.h"
#include "openhd_thermal.h"
#include "openhd_util_filesystem.h"
#include "wb_link_helper.h"
#include "wb_link_rate_helper.hpp"
#include "wifi_card.h"

static constexpr auto WB_LINK_ARM_CHANGED_TX_POWER_TAG = "wb_link_tx_power";

WBLink::WBLink(OHDProfile profile, std::vector<WiFiCard> broadcast_cards)
    : m_profile(std::move(profile)),
      m_broadcast_cards(std::move(broadcast_cards)),
      m_recommended_max_fec_blk_size_for_this_platform(
          get_fec_max_block_size_for_platform()) {
  m_console = openhd::log::create_or_get("wb_streams");
  assert(m_console);
  m_frame_drop_helper.set_console(m_console);
  m_console->info("Broadcast cards:{}", debug_cards(m_broadcast_cards));
  // sanity checks
  if (m_broadcast_cards.empty() ||
      (m_profile.is_air && m_broadcast_cards.size() > 1)) {
    // NOTE: Here we crash, since it would be a programmer(s) error
    // Air needs exactly one wifi card
    // ground supports rx diversity, therefore can have more than one card
    m_console->error(
        "Without at least one wifi card, the stream(s) cannot be started");
    exit(1);
  }
  // this fetches the last settings, otherwise creates default ones
  m_settings = std::make_unique<openhd::WBLinkSettingsHolder>(
      m_profile, m_broadcast_cards);
  WBTxRx::Options txrx_options{};
  txrx_options.session_key_packet_interval = SESSION_KEY_PACKETS_INTERVAL;
  txrx_options.use_gnd_identifier = m_profile.is_ground();
  txrx_options.debug_rssi = 0;
  txrx_options.debug_multi_rx_packets_variance = false;
  txrx_options.tx_without_pcap = true;
  txrx_options.set_tx_sock_qdisc_bypass = true;
  txrx_options.enable_auto_switch_tx_card = false;  // TODO remove me
  txrx_options.max_sane_injection_time = std::chrono::milliseconds(1);
  txrx_options.rx_radiotap_debug_level =
      openhd::load_config().GEN_RF_METRICS_LEVEL;
  // txrx_options.advanced_debugging_rx= true;
  // txrx_options.debug_decrypt_time= true;
  // txrx_options.debug_encrypt_time= true;
  // txrx_options.debug_packet_gaps= true;
  if (OHDFilesystemUtil::exists(openhd::SECURITY_KEYPAIR_FILENAME)) {
    txrx_options.secure_keypair =
        wb::read_keypair_from_file(openhd::SECURITY_KEYPAIR_FILENAME);
    m_console->debug("Using key from file {}",
                     openhd::SECURITY_KEYPAIR_FILENAME);
  } else {
    txrx_options.secure_keypair = std::nullopt;
    m_console->debug("Using key from default bind phrase");
  }
  // txrx_options.log_all_received_packets= true;
  // txrx_options.log_all_received_validated_packets= true;
  // txrx_options.advanced_latency_debugging_rx=true;
  // const auto card_names = openhd::wb::get_card_names(m_broadcast_cards);
  // assert(!card_names.empty());
  std::vector<wifibroadcast::WifiCard> tmp_wifi_cards;
  for (const auto& card : m_broadcast_cards) {
    if (card.type == WiFiCardType::OPENHD_EMULATED) {
      assert(m_broadcast_cards.size() == 1);
      tmp_wifi_cards.push_back(
          wifibroadcast::create_card_emulate(m_profile.is_air));
    } else {
      int wb_type = card.type == WiFiCardType::OPENHD_RTL_88X2AU ? 1 : 0;
      tmp_wifi_cards.push_back(
          wifibroadcast::WifiCard{card.device_name, wb_type});
    }
  }
  m_tx_header_1 = std::make_shared<RadiotapHeaderTxHolder>();
  m_tx_header_2 = std::make_shared<RadiotapHeaderTxHolder>();
  {
    const auto settings = m_settings->get_settings();
    auto mcs_index = static_cast<int>(settings.wb_air_mcs_index);
    if (m_profile.is_ground()) {
      // Always use mcs 0 on ground
      mcs_index = openhd::WB_GND_UPLINK_MCS_INDEX;
    }
    int tx_channel_width = static_cast<int>(settings.wb_air_tx_channel_width);
    if (m_profile.is_ground()) {
      // Always use 20Mhz for injection on ground
      tx_channel_width = 20;
    }
    // const bool set_flag_tx_no_ack = m_profile.is_ground() ? false :
    // !settings.wb_tx_use_ack;
    //  On ground, we default to a high retransmit count.
    //  On air, we default to no retransmit, unless explicitly enabled
    bool tx_set_high_retransmit_count = true;
    if (m_profile.is_air) {
      if (m_settings->get_settings().wb_dev_air_set_high_retransmit_count) {
        tx_set_high_retransmit_count = true;
      } else {
        tx_set_high_retransmit_count = false;
      }
    }
    const bool set_flag_tx_no_ack = !tx_set_high_retransmit_count;
    auto tmp_params =
        RadiotapHeaderTx::UserSelectableParams{tx_channel_width,
                                               settings.wb_enable_short_guard,
                                               settings.wb_enable_stbc,
                                               settings.wb_enable_ldpc,
                                               mcs_index,
                                               set_flag_tx_no_ack};
    m_console->debug("{}", RadiotapHeaderTx::user_params_to_string(tmp_params));
    m_tx_header_1->thread_safe_set(tmp_params);
    auto tmp_params2 =
        RadiotapHeaderTx::UserSelectableParams{20,
                                               settings.wb_enable_short_guard,
                                               settings.wb_enable_stbc,
                                               settings.wb_enable_ldpc,
                                               mcs_index,
                                               true};
    m_tx_header_2->thread_safe_set(tmp_params2);
  }
  m_wb_txrx =
      std::make_shared<WBTxRx>(tmp_wifi_cards, txrx_options, m_tx_header_2);
  m_wb_txrx->m_fatal_error_cb = [this](int error) {
    on_wifi_card_fatal_error();
  };
  auto dummy = m_wb_txrx->get_dummy_link();
  if (dummy) {
    dummy->set_drop_mode(DIRTY_emulate_drop_mode);
  }
  {
    // Setup the tx & rx instances for telemetry. Telemetry is bidirectional,aka
    // tx radio port on air is the same as rx on ground and verse visa
    const auto radio_port_rx =
        m_profile.is_air ? openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT
                         : openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT;
    const auto radio_port_tx =
        m_profile.is_air ? openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT
                         : openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT;
    auto cb_rx = [this](const uint8_t* data, int data_len) {
      m_last_received_packet_ts_ms = openhd::util::steady_clock_time_epoch_ms();
      auto shared =
          std::make_shared<std::vector<uint8_t>>(data, data + data_len);
      on_receive_telemetry_data(shared);
    };
    WBStreamRx::Options options_tele_rx{};
    options_tele_rx.enable_fec = false;
    options_tele_rx.radio_port = radio_port_rx;
    options_tele_rx.enable_threading = true;
    // receive queue: On air, up to 16 packets
    // On ground, up to 32 packets
    options_tele_rx.packet_queue_size = m_profile.is_air ? 16 : 32;
    m_wb_tele_rx = std::make_unique<WBStreamRx>(m_wb_txrx, options_tele_rx);
    m_wb_tele_rx->set_callback(cb_rx);
    WBStreamTx::Options options_tele_tx{};
    options_tele_tx.enable_fec = false;
    options_tele_tx.radio_port = radio_port_tx;
    // Transmission queue: On air, up to 32 packets
    // On ground, up to 16 packets
    options_tele_tx.packet_data_queue_size = m_profile.is_air ? 32 : 16;
    m_wb_tele_tx =
        std::make_unique<WBStreamTx>(m_wb_txrx, options_tele_tx, m_tx_header_1);
    m_wb_tele_tx->set_encryption(true);
  }
  {
    // Video is unidirectional, aka always goes from air pi to ground pi
    if (m_profile.is_air) {
      // we transmit video
      WBStreamTx::Options options_video_tx{};
      options_video_tx.enable_fec = true;
      // For now, have a fifo of X frame(s) to smooth out extreme edge cases of
      // bitrate overshoot
      // TODO: In ohd_video,  differentiate between "frame" and NALU (nalu can
      // also be config data) such that we can make this queue smaller.
      options_video_tx.block_data_queue_size = 2;
      options_video_tx.radio_port = openhd::VIDEO_PRIMARY_RADIO_PORT;
      auto primary = std::make_unique<WBStreamTx>(m_wb_txrx, options_video_tx,
                                                  m_tx_header_1);
      options_video_tx.radio_port = openhd::VIDEO_SECONDARY_RADIO_PORT;
      auto secondary = std::make_unique<WBStreamTx>(m_wb_txrx, options_video_tx,
                                                    m_tx_header_1);
      primary->set_encryption(false);
      secondary->set_encryption(false);
      m_wb_video_tx_list.push_back(std::move(primary));
      m_wb_video_tx_list.push_back(std::move(secondary));
      WBStreamTx::Options options_audio_tx{};
      options_audio_tx.enable_fec = false;
      options_audio_tx.radio_port = openhd::AUDIO_WIFIBROADCAST_PORT;
      options_audio_tx.packet_data_queue_size = 16;
      m_wb_audio_tx = std::make_unique<WBStreamTx>(m_wb_txrx, options_audio_tx,
                                                   m_tx_header_1);
    } else {
      // we receive video
      auto cb1 = [this](const uint8_t* data, int data_len) {
        on_receive_video_data(0, data, data_len);
      };
      auto cb2 = [this](const uint8_t* data, int data_len) {
        on_receive_video_data(1, data, data_len);
      };
      auto cb_audio = [this](const uint8_t* data, int data_len) {
        on_receive_audio_data(data, data_len);
      };
      WBStreamRx::Options options_video_rx{};
      // options_video_rx.enable_fec_debug_log=true;
      options_video_rx.enable_fec = true;
      options_video_rx.radio_port = openhd::VIDEO_PRIMARY_RADIO_PORT;
      options_video_rx.forward_gapped_fragments =
          DIRTY_forward_gapped_fragments;
      auto primary = std::make_unique<WBStreamRx>(m_wb_txrx, options_video_rx);
      primary->set_callback(cb1);
      options_video_rx.radio_port = openhd::VIDEO_SECONDARY_RADIO_PORT;
      auto secondary =
          std::make_unique<WBStreamRx>(m_wb_txrx, options_video_rx);
      secondary->set_callback(cb2);
      if (DIRTY_add_aud_nal) {
        auto block_cb = [this](uint64_t block_idx, int n_fragments_total,
                               int n_fragments_forwarded) {
          static int64_t last_block = 0;
          if (last_block + 1 != block_idx) {
            const int n_missing = block_idx - last_block;
            // m_console->debug("Missing {}",n_missing);
          }
          // m_console->debug("Got {} {}
          // {}",block_idx,n_fragments_total,n_fragments_forwarded);
          last_block = block_idx;
          // m_console->debug("Got {} {}
          // {}",block_idx,n_fragments_total,n_fragments_forwarded);
          /*if(n_fragments_forwarded>2){
              auto aud_buffer=get_h264_aud();
              on_receive_video_data(0,aud_buffer->data(),aud_buffer->size());
          }*/
        };
        primary->set_on_fec_block_done_cb(block_cb);
      }
      m_wb_video_rx_list.push_back(std::move(primary));
      m_wb_video_rx_list.push_back(std::move(secondary));
      WBStreamRx::Options options_audio_rx{};
      options_audio_rx.radio_port = openhd::AUDIO_WIFIBROADCAST_PORT;
      options_audio_rx.enable_fec = false;
      options_audio_rx.enable_threading = true;
      options_audio_rx.packet_queue_size = 16;
      m_wb_audio_rx = std::make_unique<WBStreamRx>(m_wb_txrx, options_audio_rx);
      m_wb_audio_rx->set_callback(cb_audio);
    }
  }
  apply_frequency_and_channel_width_from_settings();
  apply_txpower();
  if (m_profile.is_ground()) {
    m_management_gnd = std::make_unique<ManagementGround>(m_wb_txrx);
    m_management_gnd->m_tx_header = m_tx_header_1;
    m_management_gnd->start();
    m_gnd_curr_rx_frequency =
        static_cast<int>(m_settings->unsafe_get_settings().wb_frequency);
  } else {
    m_management_air = std::make_unique<ManagementAir>(
        m_wb_txrx, m_settings->get_settings().wb_frequency,
        m_settings->get_settings().wb_air_tx_channel_width);
    m_management_air->m_tx_header = m_tx_header_2;
    m_management_air->start();
  }
  m_wb_txrx->start_receiving();
  m_work_thread_run = true;
  m_work_thread = std::make_unique<std::thread>(&WBLink::loop_do_work, this);
  std::function<bool(openhd::LinkActionHandler::ScanChannelsParam)> cb_scan =
      [this](openhd::LinkActionHandler::ScanChannelsParam param) {
        return request_start_scan_channels(param);
      };
  openhd::LinkActionHandler::instance().wb_cmd_scan_channels = cb_scan;
  std::function<bool(int)> cb_analyze = [this](int channels_to_scan) {
    return request_start_analyze_channels(channels_to_scan);
  };
  openhd::LinkActionHandler::instance().wb_cmd_analyze_channels = cb_analyze;
  if (m_profile.is_air) {
    // MCS is only changed on air
    auto cb_channel = [this](const std::array<int, 18>& rc_channels) {
      m_rc_channel_helper.set_rc_channels(rc_channels);
    };
    openhd::FCRcChannelsHelper::instance().action_on_any_rc_channel_register(
        cb_channel);
  }
  auto cb_arm = [this](bool armed) { update_arming_state(armed); };
  openhd::ArmingStateHelper::instance().register_listener(
      WB_LINK_ARM_CHANGED_TX_POWER_TAG, cb_arm);
  std::function<std::vector<uint16_t>(void)> wb_get_supported_channels =
      [this]() {
        std::vector<uint16_t> ret;
        const auto frequencies =
            m_broadcast_cards.at(0).get_supported_frequencies_2G_5G();
        ret.reserve(frequencies.size());
        for (const auto freq : frequencies) {
          ret.push_back(static_cast<uint16_t>(freq));
        }
        return ret;
      };
  openhd::LinkActionHandler::instance().wb_get_supported_channels =
      wb_get_supported_channels;
}

WBLink::~WBLink() {
  m_console->debug("WBLink::~WBLink() begin");
  if (m_work_thread) {
    m_work_thread_run = false;
    m_work_thread->join();
  }
  m_management_air = nullptr;
  m_management_gnd = nullptr;
  openhd::FCRcChannelsHelper::instance().action_on_any_rc_channel_register(
      nullptr);
  openhd::ArmingStateHelper::instance().unregister_listener(
      WB_LINK_ARM_CHANGED_TX_POWER_TAG);
  openhd::LinkActionHandler::instance().wb_cmd_scan_channels = nullptr;
  openhd::LinkActionHandler::instance().wb_cmd_analyze_channels = nullptr;
  m_wb_txrx->stop_receiving();
  // stop all the receiver/transmitter instances, after that, give card back to
  // network manager
  m_wb_tele_rx.reset();
  m_wb_tele_tx.reset();
  m_wb_video_tx_list.resize(0);
  m_wb_video_rx_list.resize(0);
  m_wb_audio_tx.reset();
  m_wb_audio_rx.reset();
  m_wb_txrx = nullptr;
  wifi::commandhelper::cleanup_openhd_driver_overrides();
  m_console->debug("WBLink::~WBLink() end");
}

bool WBLink::request_set_frequency(int frequency) {
  m_console->debug("request_set_frequency {}", frequency);
  if (!openhd::wb::validate_frequency_change(
          frequency, m_settings->get_settings().wb_air_tx_channel_width,
          m_broadcast_cards, m_console)) {
    return false;
  }
  if (OHDPlatform::instance().is_x20() && frequency < 5180) {
    m_console->warn("X20 only supports 5G");
    return false;
  }
  auto work_item = std::make_shared<WorkItem>(
      fmt::format("SET_FREQ:{}", frequency),
      [this, frequency]() {
        m_settings->unsafe_get_settings().wb_frequency = frequency;
        m_settings->persist();
        if (m_profile.is_air) {
          // temporarily disable video streaming to free up BW
          m_air_close_video_in = true;
          m_management_air->set_frequency(frequency);
          // We need to delay the change to make sure at least one management
          // packet goes through ..
          std::this_thread::sleep_for(std::chrono::seconds(2));
          m_air_close_video_in = false;
        }
        apply_frequency_and_channel_width_from_settings();
        m_rate_adjustment_frequency_changed = true;
      },
      std::chrono::steady_clock::now());
  return try_schedule_work_item(work_item);
}

bool WBLink::request_set_air_tx_channel_width(int channel_width) {
  assert(m_profile.is_air);  // Channel width is only ever changed on air
  m_console->debug("request_set_air_tx_channel_width {}", channel_width);
  if (!openhd::wb::validate_air_channel_width_change(
          channel_width, m_broadcast_cards.at(0), m_console)) {
    return false;
  }
  auto work_item = std::make_shared<WorkItem>(
      fmt::format("SET_CHWIDTH:{}", channel_width),
      [this, channel_width]() {
        // temporarily disable video streaming to free up BW
        m_air_close_video_in = true;
        m_settings->unsafe_get_settings().wb_air_tx_channel_width =
            channel_width;
        m_settings->persist();
        m_management_air->set_channel_width(channel_width);
        // On ASUS, we have to reduce the TX power when on 40Mhz
        apply_txpower();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // Ground will automatically apply the right channel width once first
        // (broadcast) management frame is received.
        apply_frequency_and_channel_width_from_settings();
        m_air_close_video_in = false;
      },
      std::chrono::steady_clock::now());
  return try_schedule_work_item(work_item);
}

bool WBLink::request_set_tx_power_mw(int tx_power_mw, bool armed) {
  m_console->debug("request_set_tx_power_mw {}mW", tx_power_mw);
  if (!(openhd::is_valid_tx_power_milli_watt(tx_power_mw) ||
        (armed && tx_power_mw == 0))) {
    m_console->warn("Invalid tx power:{}mW", tx_power_mw);
    return false;
  }
  auto tag = fmt::format("SET_TX_POWER_MW_{}:{}", armed ? "ARMED" : "DISARMED",
                         tx_power_mw);
  auto work_item = std::make_shared<WorkItem>(
      tag,
      [this, tx_power_mw, armed]() {
        if (armed) {
          m_settings->unsafe_get_settings().wb_tx_power_milli_watt_armed =
              tx_power_mw;
        } else {
          m_settings->unsafe_get_settings().wb_tx_power_milli_watt =
              tx_power_mw;
        }
        m_settings->persist();
        m_request_apply_tx_power = true;
      },
      std::chrono::steady_clock::now());
  return try_schedule_work_item(work_item);
}

bool WBLink::request_set_tx_power_rtl8812au(int tx_power_index_override,
                                            bool armed) {
  m_console->debug("request_set_tx_power_rtl8812au {}index",
                   tx_power_index_override);
  if (!openhd::validate_wb_rtl8812au_tx_pwr_idx_override(
          tx_power_index_override)) {
    return false;
  }
  auto tag = fmt::format("SET_TX_POWER_INDEX_{}:{}",
                         armed ? "ARMED" : "DISARMED", tx_power_index_override);
  auto work_item = std::make_shared<WorkItem>(
      tag,
      [this, tx_power_index_override, armed]() {
        if (armed) {
          m_settings->unsafe_get_settings()
              .wb_rtl8812au_tx_pwr_idx_override_armed = tx_power_index_override;
        } else {
          m_settings->unsafe_get_settings().wb_rtl8812au_tx_pwr_idx_override =
              tx_power_index_override;
        }
        m_settings->persist();
        m_request_apply_tx_power = true;
      },
      std::chrono::steady_clock::now());
  return try_schedule_work_item(work_item);
}

bool WBLink::request_set_air_mcs_index(int mcs_index) {
  assert(m_profile.is_air);
  m_console->debug("set_air_mcs_index {}", mcs_index);
  if (!openhd::wb::validate_air_mcs_index_change(
          mcs_index, m_broadcast_cards.at(0), m_console)) {
    return false;
  }
  auto tag = fmt::format("SET_AIR_MCS:{}", mcs_index);
  auto work_item = std::make_shared<WorkItem>(
      tag,
      [this, mcs_index]() {
        m_settings->unsafe_get_settings().wb_air_mcs_index = mcs_index;
        m_settings->persist();
        m_request_apply_air_mcs_index = true;
      },
      std::chrono::steady_clock::now());
  return try_schedule_work_item(work_item);
}
bool WBLink::set_air_video_fec_percentage(int fec_percentage) {
  m_console->debug("set_air_video_fec_percentage {}", fec_percentage);
  if (!openhd::is_valid_fec_percentage(fec_percentage)) return false;
  m_settings->unsafe_get_settings().wb_video_fec_percentage = fec_percentage;
  m_settings->persist();
  // The next rate adjustment will adjust the bitrate accordingly
  return true;
}
bool WBLink::set_air_enable_wb_video_variable_bitrate(int value) {
  assert(m_profile.is_air);
  if (!openhd::validate_yes_or_no(value)) return false;
  // value is read in regular intervals.
  m_settings->unsafe_get_settings().enable_wb_video_variable_bitrate = value;
  m_settings->persist();
  return true;
}

bool WBLink::set_air_max_fec_block_size_for_platform(int value) {
  m_settings->unsafe_get_settings().wb_max_fec_block_size = value;
  m_settings->persist();
  return true;
}

bool WBLink::set_air_wb_video_rate_for_mcs_adjustment_percent(int value) {
  if (value <= 5 || value > 1000) return false;
  m_settings->unsafe_get_settings().wb_video_rate_for_mcs_adjustment_percent =
      value;
  m_settings->persist();
  return true;
}
bool WBLink::set_dev_air_set_high_retransmit_count(int value) {
  assert(m_profile.is_air);
  if (!openhd::validate_yes_or_no(value)) return false;
  m_settings->unsafe_get_settings().wb_dev_air_set_high_retransmit_count =
      value;
  m_settings->persist();
  m_tx_header_1->update_set_flag_tx_no_ack(!value);
  return true;
}
bool WBLink::request_start_scan_channels(
    openhd::LinkActionHandler::ScanChannelsParam scan_channels_params) {
  auto work_item = std::make_shared<WorkItem>(
      "SCAN_CHANNELS",
      [this, scan_channels_params]() {
        perform_channel_scan(scan_channels_params);
      },
      std::chrono::steady_clock::now());
  return try_schedule_work_item(work_item);
}

bool WBLink::request_start_analyze_channels(int channels_to_scan) {
  auto work_item = std::make_shared<WorkItem>(
      "ANALYZE_CHANNELS",
      [this, channels_to_scan]() { perform_channel_analyze(channels_to_scan); },
      std::chrono::steady_clock::now());
  return try_schedule_work_item(work_item);
}

bool WBLink::apply_frequency_and_channel_width(int frequency,
                                               int channel_width_rx,
                                               int channel_width_tx) {
  m_console->debug("apply_frequency_and_channel_width {}Mhz RX:{}Mhz TX:{}Mhz",
                   frequency, channel_width_rx, channel_width_tx);
  // Weird bug hunting - I hope this makes the driver less likely too crash
  // Temporarily stop injecting packets
  m_wb_txrx->set_passive_mode(true);
  std::this_thread::sleep_for(std::chrono::milliseconds(
      100));  // Dirty - wait for any tx packets to drain
  const auto res = openhd::wb::set_frequency_and_channel_width_for_all_cards(
      frequency, channel_width_rx, m_broadcast_cards);
  m_tx_header_1->update_channel_width(channel_width_tx);
  m_wb_txrx->tx_reset_stats();
  m_wb_txrx->rx_reset_stats();
  re_enable_injection_unless_user_passive_mode_enabled();
  return res;
}

bool WBLink::apply_frequency_and_channel_width_from_settings() {
  const auto settings = m_settings->get_settings();
  const int center_frequency = settings.wb_frequency;
  uint8_t channel_width_rx = -1;
  uint8_t channel_width_tx = -1;
  if (m_profile.is_air) {
    // Solved: can we send in 40Mhz but listen in 20Mhz ? NO
    // But we can obviously receive 20Mhz packets while in 40Mhz mode
    channel_width_tx = static_cast<int>(settings.wb_air_tx_channel_width);
    channel_width_rx = channel_width_tx;
  } else {
    // GND always uses 20Mhz channel width for uplink, and listens in 40Mhz
    // unless air reports 20Mhz (in which case we can go down to 20Mhz listen,
    // which gives us better sensitivity)
    channel_width_rx = m_gnd_curr_rx_channel_width;
    channel_width_tx = 20;
  }
  const auto res = apply_frequency_and_channel_width(
      center_frequency, channel_width_rx, channel_width_tx);
  return res;
}

void WBLink::apply_txpower() {
  const auto settings = m_settings->get_settings();
  const auto before = std::chrono::steady_clock::now();
  uint32_t pwr_index = (int)settings.wb_rtl8812au_tx_pwr_idx_override;
  uint32_t pwr_mw = (int)settings.wb_tx_power_milli_watt;
  if (m_is_armed && settings.wb_rtl8812au_tx_pwr_idx_override_armed !=
                        openhd::RTL8812AU_TX_POWER_INDEX_ARMED_DISABLED) {
    m_console->debug("Using power index special for armed");
    pwr_index = settings.wb_rtl8812au_tx_pwr_idx_override_armed;
  }
  if (m_is_armed && settings.wb_tx_power_milli_watt_armed !=
                        openhd::WIFI_TX_POWER_MILLI_WATT_ARMED_DISABLED) {
    m_console->debug("Using power mw special for armed");
    pwr_mw = settings.wb_tx_power_milli_watt_armed;
  }
  if (m_profile.is_air) {
    if (m_broadcast_cards.at(0).type == WiFiCardType::OPENHD_RTL_88X2AU &&
        pwr_index > 50 && settings.wb_air_tx_channel_width == 40) {
      m_console->debug("Reducing TX power  to 50 tpi due to 40Mhz");
      pwr_index = 50;
    }
  }
  openhd::wb::set_tx_power_for_all_cards(pwr_mw, pwr_index, m_broadcast_cards);
  m_curr_tx_power_mw = pwr_mw;
  m_curr_tx_power_idx = pwr_index;
  const auto delta = std::chrono::steady_clock::now() - before;
  m_console->debug("Changing tx power took {}", MyTimeHelper::R(delta));
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-unnecessary-value-param"
std::vector<openhd::Setting> WBLink::get_all_settings() {
  using namespace openhd;
  std::vector<openhd::Setting> ret{};
  const auto settings = m_settings->get_settings();
  auto change_freq = openhd::IntSetting{
      (int)settings.wb_frequency,
      [this](std::string, int value) { return request_set_frequency(value); }};
  change_freq.get_callback = [this]() {
    return m_settings->unsafe_get_settings().wb_frequency;
  };
  ret.push_back(Setting{WB_FREQUENCY, change_freq});
  if (m_profile.is_air) {
    // MCS is only changeable on air
    auto change_wb_air_mcs_index = openhd::IntSetting{
        (int)settings.wb_air_mcs_index, [this](std::string, int value) {
          return request_set_air_mcs_index(value);
        }};
    ret.push_back(Setting{WB_MCS_INDEX, change_wb_air_mcs_index});
    // Channel width is only changeable on the air
    auto change_wb_channel_width = openhd::IntSetting{
        (int)settings.wb_air_tx_channel_width, [this](std::string, int value) {
          return request_set_air_tx_channel_width(value);
        }};
    change_wb_channel_width.get_callback = [this]() {
      return m_settings->unsafe_get_settings().wb_air_tx_channel_width;
    };
    ret.push_back(Setting{WB_CHANNEL_WIDTH, change_wb_channel_width});
    auto cb_change_video_fec_percentage = [this](std::string, int value) {
      return set_air_video_fec_percentage(value);
    };
    ret.push_back(
        Setting{WB_VIDEO_FEC_PERCENTAGE,
                openhd::IntSetting{(int)settings.wb_video_fec_percentage,
                                   cb_change_video_fec_percentage}});
    auto cb_enable_wb_video_variable_bitrate = [this](std::string, int value) {
      return set_air_enable_wb_video_variable_bitrate(value);
    };
    ret.push_back(Setting{
        WB_VIDEO_VARIABLE_BITRATE,
        openhd::IntSetting{(int)settings.enable_wb_video_variable_bitrate,
                           cb_enable_wb_video_variable_bitrate}});
    auto cb_wb_max_fec_block_size_for_platform = [this](std::string,
                                                        int value) {
      return set_air_max_fec_block_size_for_platform(value);
    };
    ret.push_back(
        Setting{WB_MAX_FEC_BLOCK_SIZE_FOR_PLATFORM,
                openhd::IntSetting{(int)settings.wb_max_fec_block_size,
                                   cb_wb_max_fec_block_size_for_platform}});
    auto cb_wb_video_rate_for_mcs_adjustment_percent = [this](std::string,
                                                              int value) {
      return set_air_wb_video_rate_for_mcs_adjustment_percent(value);
    };
    ret.push_back(
        Setting{WB_VIDEO_RATE_FOR_MCS_ADJUSTMENT_PERC,
                openhd::IntSetting{
                    (int)settings.wb_video_rate_for_mcs_adjustment_percent,
                    cb_wb_video_rate_for_mcs_adjustment_percent}});
    // changing the mcs index via rc channel only makes sense on air,
    // and is only possible if the card supports it
    if (m_broadcast_cards.at(0).supports_openhd_wifibroadcast()) {
      auto cb_mcs_via_rc_channel = [this](std::string, int value) {
        if (value < 0 || value > 18)
          return false;  // 0 is disabled, valid rc channel number otherwise
        // we check if this is enabled in regular intervals (whenever we get the
        // rc channels message from the FC)
        m_settings->unsafe_get_settings().wb_mcs_index_via_rc_channel = value;
        m_settings->persist();
        return true;
      };
      ret.push_back(
          Setting{openhd::WB_MCS_INDEX_VIA_RC_CHANNEL,
                  openhd::IntSetting{(int)settings.wb_mcs_index_via_rc_channel,
                                     cb_mcs_via_rc_channel}});
      /*auto cb_bw_via_rc_channel = [this](std::string, int value) {
        if (value < 0 || value > 18) {
          return false;
        }
        m_settings->unsafe_get_settings().wb_bw_via_rc_channel = value;
        m_settings->persist();
        return true;
      };
      ret.push_back(
          Setting{openhd::WB_BW_VIA_RC_CHANNEL,
                  openhd::IntSetting{(int)settings.wb_bw_via_rc_channel,
                                     cb_bw_via_rc_channel}});*/
    }
    auto cb_dev_air_set_high_retransmit_count = [this](std::string, int value) {
      return set_dev_air_set_high_retransmit_count(value);
    };
    ret.push_back(Setting{
        WB_DEV_AIR_SET_HIGH_RETRANSMIT_COUNT,
        openhd::IntSetting{(int)settings.wb_dev_air_set_high_retransmit_count,
                           cb_dev_air_set_high_retransmit_count}});
  }
  if (m_profile.is_ground()) {
    // We display the total n of detected RX cards such that users can validate
    // their multi rx setup(s) if there is more than one rx card detected (Note:
    // air always has exactly one monitor mode wi-fi card)
    const int n_rx_cards = static_cast<int>(m_broadcast_cards.size());
    if (n_rx_cards > 1) {
      ret.push_back(openhd::create_read_only_int("WB_N_RX_CARDS", n_rx_cards));
    }
    // feature on the ground station only
    auto cb_passive = [this](std::string, int value) {
      if (!validate_yes_or_no(value)) return false;
      m_settings->unsafe_get_settings().wb_enable_listen_only_mode = value;
      m_settings->persist();
      m_wb_txrx->set_passive_mode(value);
      return true;
    };
    ret.push_back(
        Setting{openhd::WB_PASSIVE_MODE,
                openhd::IntSetting{(int)settings.wb_enable_listen_only_mode,
                                   cb_passive}});
  }
  const bool any_card_supports_stbc_ldpc_sgi =
      openhd::wb::any_card_supports_stbc_ldpc_sgi(m_broadcast_cards);
  // These 3 are only supported / known to work on rtl8812au (yet), therefore
  // only expose them when rtl8812au is used
  if (any_card_supports_stbc_ldpc_sgi) {
    // STBC - definitely for advanced users, but aparently it can have benefits.
    auto cb_wb_enable_stbc = [this](std::string, int stbc) {
      if (stbc < 0 || stbc > 3) return false;
      m_settings->unsafe_get_settings().wb_enable_stbc = stbc;
      m_settings->persist();
      m_tx_header_1->update_stbc(stbc);
      m_tx_header_2->update_stbc(stbc);
      return true;
    };
    ret.push_back(openhd::Setting{
        WB_ENABLE_STBC,
        openhd::IntSetting{settings.wb_enable_stbc, cb_wb_enable_stbc}});
    // These 2 params are exposed by default from OpenHD, but whitelisted in
    // QOpenHD to prevent inexperienced users from changing them
    auto cb_wb_enable_ldpc = [this](std::string, int ldpc) {
      if (!validate_yes_or_no(ldpc)) return false;
      m_settings->unsafe_get_settings().wb_enable_ldpc = ldpc;
      m_settings->persist();
      m_tx_header_1->update_ldpc(ldpc);
      m_tx_header_2->update_ldpc(ldpc);
      return true;
    };
    ret.push_back(openhd::Setting{
        WB_ENABLE_LDPC,
        openhd::IntSetting{settings.wb_enable_ldpc, cb_wb_enable_ldpc}});
    auto cb_wb_enable_sg = [this](std::string, int short_gi) {
      if (!validate_yes_or_no(short_gi)) return false;
      m_settings->unsafe_get_settings().wb_enable_short_guard = short_gi;
      m_settings->persist();
      m_tx_header_1->update_guard_interval(short_gi);
      m_tx_header_2->update_guard_interval(short_gi);
      return true;
    };
    ret.push_back(openhd::Setting{
        WB_ENABLE_SHORT_GUARD,
        openhd::IntSetting{settings.wb_enable_short_guard, cb_wb_enable_sg}});
  }
  // WIFI TX power depends on the used chips
  if (openhd::wb::has_any_rtl8812au(m_broadcast_cards)) {
    auto cb_wb_rtl8812au_tx_pwr_idx_override = [this](std::string, int value) {
      return request_set_tx_power_rtl8812au(value, false);
    };
    ret.push_back(openhd::Setting{
        WB_RTL8812AU_TX_PWR_IDX_OVERRIDE,
        openhd::IntSetting{(int)settings.wb_rtl8812au_tx_pwr_idx_override,
                           cb_wb_rtl8812au_tx_pwr_idx_override}});
    auto cb_wb_rtl8812au_tx_pwr_idx_armed = [this](std::string, int value) {
      return request_set_tx_power_rtl8812au(value, true);
    };
    ret.push_back(openhd::Setting{
        WB_RTL8812AU_TX_PWR_IDX_ARMED,
        openhd::IntSetting{(int)settings.wb_rtl8812au_tx_pwr_idx_override_armed,
                           cb_wb_rtl8812au_tx_pwr_idx_armed}});
  }
  if (openhd::wb::has_any_non_rtl8812au(m_broadcast_cards)) {
    auto cb_wb_tx_power_milli_watt = [this](std::string, int value) {
      return request_set_tx_power_mw(value, false);
    };
    auto change_tx_power = openhd::IntSetting{
        (int)settings.wb_tx_power_milli_watt, cb_wb_tx_power_milli_watt};
    ret.push_back(Setting{WB_TX_POWER_MILLI_WATT, change_tx_power});
    auto cb_wb_tx_power_milli_watt_armed = [this](std::string, int value) {
      return request_set_tx_power_mw(value, true);
    };
    auto change_tx_power_armed =
        openhd::IntSetting{(int)settings.wb_tx_power_milli_watt_armed,
                           cb_wb_tx_power_milli_watt_armed};
    ret.push_back(Setting{WB_TX_POWER_MILLI_WATT_ARMED, change_tx_power_armed});
  }
  openhd::validate_provided_ids(ret);
  return ret;
}
#pragma clang diagnostic pop

void WBLink::loop_do_work() {
  while (m_work_thread_run) {
    // Perform any queued up work if it exists
    {
      m_work_item_queue_mutex.lock();
      if (!m_work_item_queue.empty()) {
        auto front = m_work_item_queue.front();
        if (front->ready_to_be_executed()) {
          m_console->debug("Start execute work item {}", front->TAG);
          front->execute();
          m_console->debug("Done executing work item {}", front->TAG);
          m_work_item_queue.pop();
        }
      }
      m_work_item_queue_mutex.unlock();
    }
    // If needed, apply the proper tx power (depending on armed / disarmed
    // state).
    bool tmp_true = true;
    if (m_request_apply_tx_power.compare_exchange_strong(tmp_true, false)) {
      apply_txpower();
    }
    wt_perform_mcs_via_rc_channel_if_enabled();
    // wt_perform_bw_via_rc_channel_if_enabled();
    wt_gnd_perform_channel_management();
    // air_perform_reset_frequency();
    // Perform thermal protection level calculation before rate adjustment !
    wt_perform_update_thermal_protection();
    wt_perform_rate_adjustment();
    //  After we've applied the rate, we update the tx header mcs index if
    //  necessary
    tmp_true = true;
    if (m_request_apply_air_mcs_index.compare_exchange_strong(tmp_true,
                                                              false)) {
      const int mcs_index = m_settings->unsafe_get_settings().wb_air_mcs_index;
      m_tx_header_1->update_mcs_index(mcs_index);
      m_tx_header_2->update_mcs_index(mcs_index);
    }
    tmp_true = true;
    /*if (m_request_apply_air_bw.compare_exchange_strong(tmp_true,
                                                              false)) {
      apply_frequency_and_channel_width_from_settings();
    }*/
    // update statistics in regular intervals
    wt_update_statistics();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void WBLink::wt_update_statistics() {
  const auto elapsed_since_last =
      std::chrono::steady_clock::now() - m_last_stats_recalculation;
  if (elapsed_since_last < RECALCULATE_STATISTICS_INTERVAL) {
    return;
  }
  m_last_stats_recalculation = std::chrono::steady_clock::now();
  // telemetry is available on both air and ground
  openhd::link_statistics::StatsAirGround stats{};
  if (m_wb_tele_tx) {
    const auto curr_tx_stats = m_wb_tele_tx->get_latest_stats();
    const auto curr_rx_stats = m_wb_tele_rx->get_latest_stats();
    stats.telemetry.curr_tx_bps =
        curr_tx_stats.current_provided_bits_per_second;
    stats.telemetry.curr_tx_pps =
        curr_tx_stats.current_injected_packets_per_second;
    stats.telemetry.curr_rx_bps = curr_rx_stats.curr_in_bits_per_second;
    stats.telemetry.curr_rx_pps = curr_rx_stats.curr_in_packets_per_second;
  }
  if (m_profile.is_air) {
    // video on air
    for (int i = 0; i < m_wb_video_tx_list.size(); i++) {
      auto& wb_tx = *m_wb_video_tx_list.at(i);
      // auto& air_video=i==0 ? stats.air_video0 : stats.air_video1;
      const auto curr_tx_stats = wb_tx.get_latest_stats();
      // optimization - only send for active video links
      if (curr_tx_stats.n_injected_packets == 0) continue;
      openhd::link_statistics::Xmavlink_openhd_stats_wb_video_air_t air_video{};
      openhd::link_statistics::
          Xmavlink_openhd_stats_wb_video_air_fec_performance_t air_fec{};
      air_video.link_index = i;
      int rec_bitrate = 0;
      auto cam_stats = openhd::LinkActionHandler::instance().get_cam_info(i);
      rec_bitrate = cam_stats.encoding_bitrate_kbits;
      air_video.curr_recommended_bitrate = rec_bitrate;
      air_video.curr_measured_encoder_bitrate =
          curr_tx_stats.current_provided_bits_per_second;
      air_video.curr_injected_bitrate =
          curr_tx_stats.current_injected_bits_per_second;
      air_video.curr_injected_pps =
          curr_tx_stats.current_injected_packets_per_second;
      const int tx_dropped_frames =
          i == 0 ? m_primary_total_dropped_frames.load()
                 : m_secondary_total_dropped_frames.load();
      // const int tx_dropped_frames = curr_tx_stats.n_dropped_frames;
      air_video.curr_dropped_frames = tx_dropped_frames;
      air_video.dummy0 =
          (int8_t)m_thermal_protection_level.load(std::memory_order_relaxed);
      const auto curr_tx_fec_stats = wb_tx.get_latest_fec_stats();
      air_fec.curr_fec_encode_time_avg_us =
          openhd::util::get_micros(curr_tx_fec_stats.curr_fec_encode_time.avg);
      air_fec.curr_fec_encode_time_min_us =
          openhd::util::get_micros(curr_tx_fec_stats.curr_fec_encode_time.min);
      air_fec.curr_fec_encode_time_max_us =
          openhd::util::get_micros(curr_tx_fec_stats.curr_fec_encode_time.max);
      air_fec.curr_fec_block_size_min =
          curr_tx_fec_stats.curr_fec_block_length.min;
      air_fec.curr_fec_block_size_max =
          curr_tx_fec_stats.curr_fec_block_length.max;
      air_fec.curr_fec_block_size_avg =
          curr_tx_fec_stats.curr_fec_block_length.avg;
      air_fec.curr_tx_delay_min_us = curr_tx_stats.curr_block_until_tx_min_us;
      air_fec.curr_tx_delay_max_us = curr_tx_stats.curr_block_until_tx_max_us;
      air_fec.curr_tx_delay_avg_us = curr_tx_stats.curr_block_until_tx_avg_us;
      air_video.curr_fec_percentage =
          m_settings->unsafe_get_settings().wb_video_fec_percentage;
      stats.stats_wb_video_air.push_back(air_video);
      if (i == 0) stats.air_fec_performance = air_fec;
    }
  } else {
    // video on ground
    for (int i = 0; i < m_wb_video_rx_list.size(); i++) {
      auto& wb_rx = *m_wb_video_rx_list.at(i);
      const auto wb_rx_stats = wb_rx.get_latest_stats();
      openhd::link_statistics::Xmavlink_openhd_stats_wb_video_ground_t
          ground_video{};
      openhd::link_statistics::
          Xmavlink_openhd_stats_wb_video_ground_fec_performance_t gnd_fec{};
      ground_video.link_index = i;
      // Use outgoing bitrate - otherwise, we get N times the bandwidth with
      // multiple RX-es.
      // ground_video.curr_incoming_bitrate=wb_rx_stats.curr_in_bits_per_second;
      ground_video.curr_incoming_bitrate = wb_rx_stats.curr_out_bits_per_second;
      const auto fec_stats = wb_rx.get_latest_fec_stats();
      ground_video.count_fragments_recovered =
          fec_stats.count_fragments_recovered;
      ground_video.count_blocks_recovered = fec_stats.count_blocks_recovered;
      ground_video.count_blocks_lost = fec_stats.count_blocks_lost;
      ground_video.count_blocks_total = fec_stats.count_blocks_total;
      gnd_fec.curr_fec_decode_time_avg_us =
          openhd::util::get_micros(fec_stats.curr_fec_decode_time.avg);
      gnd_fec.curr_fec_decode_time_min_us =
          openhd::util::get_micros(fec_stats.curr_fec_decode_time.min);
      gnd_fec.curr_fec_decode_time_max_us =
          openhd::util::get_micros(fec_stats.curr_fec_decode_time.max);
      // TODO otimization: Only send stats for an active link
      stats.stats_wb_video_ground.push_back(ground_video);
      if (i == 0) stats.gnd_fec_performance = gnd_fec;
    }
  }
  const auto& curr_settings = m_settings->unsafe_get_settings();
  const auto rxStats = m_wb_txrx->get_rx_stats();
  const auto txStats = m_wb_txrx->get_tx_stats();
  stats.monitor_mode_link.curr_rx_packet_loss_perc =
      rxStats.curr_lowest_packet_loss;
  stats.monitor_mode_link.count_tx_inj_error_hint =
      txStats.count_tx_injections_error_hint;
  stats.monitor_mode_link.count_tx_dropped_packets =
      txStats.count_tx_dropped_packets;
  stats.monitor_mode_link.curr_tx_mcs_index = curr_settings.wb_air_mcs_index;
  // m_console->debug("Big gaps:{}",rxStats.curr_big_gaps_counter);
  stats.monitor_mode_link.curr_tx_channel_mhz = curr_settings.wb_frequency;
  if (m_profile.is_air) {
    stats.monitor_mode_link.curr_tx_channel_w_mhz =
        curr_settings.wb_air_tx_channel_width;
  } else {
    stats.monitor_mode_link.curr_tx_channel_w_mhz = m_gnd_curr_rx_channel_width;
  }
  stats.monitor_mode_link.curr_rate_kbits =
      m_max_total_rate_for_current_wifi_config_kbits;
  stats.monitor_mode_link.curr_n_rate_adjustments = m_curr_n_rate_adjustments;
  stats.monitor_mode_link.curr_tx_pps = txStats.curr_packets_per_second;
  stats.monitor_mode_link.curr_tx_bps =
      txStats.curr_bits_per_second_excluding_overhead;
  stats.monitor_mode_link.curr_rx_pps = rxStats.curr_packets_per_second;
  stats.monitor_mode_link.curr_rx_bps = rxStats.curr_bits_per_second;
  stats.monitor_mode_link.pollution_perc = rxStats.curr_link_pollution_perc;
  stats.monitor_mode_link.dummy1 =
      static_cast<int16_t>(rxStats.curr_n_foreign_packets_pps);
  const int tmp_last_management_packet_ts =
      m_profile.is_air ? m_management_air->get_last_received_packet_ts_ms()
                       : m_management_gnd->get_last_received_packet_ts_ms();
  const int last_received_packet_ts = std::max(
      m_last_received_packet_ts_ms.load(), tmp_last_management_packet_ts);
  const auto elapsed_since_last_rx_packet_ms =
      openhd::util::steady_clock_time_epoch_ms() - last_received_packet_ts;
  const bool curr_rx_last_packet_status_good =
      elapsed_since_last_rx_packet_ms <= 5 * 1000;
  const auto bitfield = openhd::link_statistics::MonitorModeLinkBitfield{
      (bool)curr_settings.wb_enable_stbc, (bool)curr_settings.wb_enable_ldpc,
      (bool)curr_settings.wb_enable_short_guard,
      (bool)curr_rx_last_packet_status_good};
  stats.monitor_mode_link.bitfield =
      openhd::link_statistics::write_monitor_link_bitfield(bitfield);
  {
    // Operating mode
    stats.gnd_operating_mode.operating_mode = 0;
    stats.gnd_operating_mode.tx_passive_mode_is_enabled =
        curr_settings.wb_enable_listen_only_mode ? 1 : 0;
    stats.gnd_operating_mode.progress = 0;
  }
  // m_console->debug("{}",WBTxRx::tx_stats_to_string(txStats));
  // m_console->debug("{}",WBTxRx::rx_stats_to_string(rxStats));
  // m_console->debug("Pollution: {}",rxStats.curr_link_pollution_perc);
  assert(stats.cards.size() >= 4);
  // only populate actually used cards
  assert(m_broadcast_cards.size() <= stats.cards.size());
  const auto curr_active_tx = m_wb_txrx->get_curr_active_tx_card_idx();
  for (int i = 0; i < m_broadcast_cards.size(); i++) {
    const auto& card = m_broadcast_cards.at(i);
    auto& card_stats = stats.cards.at(i);
    card_stats.NON_MAVLINK_CARD_ACTIVE = true;
    auto rxStatsCard = m_wb_txrx->get_rx_stats_for_card(i);
    auto rf_rx_stats = m_wb_txrx->get_rx_rf_stats_for_card(i);
    if (m_broadcast_cards[i].type == WiFiCardType::OPENHD_RTL_88X2AU ||
        m_broadcast_cards[i].type == WiFiCardType::OPENHD_RTL_88X2BU ||
        m_broadcast_cards[i].type == WiFiCardType::OPENHD_RTL_8852BU) {
      // Value per adapter is shit, use the max of antenna(s) instead
      rf_rx_stats.adapter.rssi_dbm = std::max(rf_rx_stats.antenna1.rssi_dbm,
                                              rf_rx_stats.antenna2.rssi_dbm);
    }
    card_stats.tx_active = i == curr_active_tx ? 1 : 0;
    card_stats.rx_rssi = rf_rx_stats.adapter.rssi_dbm;
    card_stats.rx_signal_quality_adapter =
        rf_rx_stats.adapter.card_signal_quality_perc;
    card_stats.rx_noise_adapter = rf_rx_stats.adapter.noise_dbm;
    card_stats.rx_rssi_1 = rf_rx_stats.antenna1.rssi_dbm;
    card_stats.rx_rssi_2 = rf_rx_stats.antenna2.rssi_dbm;
    card_stats.count_p_received = rxStatsCard.count_p_valid;
    card_stats.count_p_injected = 0;
    card_stats.curr_rx_packet_loss_perc = rxStatsCard.curr_packet_loss;
    card_stats.tx_power_current = card.type == WiFiCardType::OPENHD_RTL_88X2AU
                                      ? m_curr_tx_power_idx.load()
                                      : m_curr_tx_power_mw.load();
    card_stats.tx_power_disarmed =
        card.type == WiFiCardType::OPENHD_RTL_88X2AU
            ? curr_settings.wb_rtl8812au_tx_pwr_idx_override
            : curr_settings.wb_tx_power_milli_watt;
    card_stats.tx_power_armed =
        card.type == WiFiCardType::OPENHD_RTL_88X2AU
            ? curr_settings.wb_rtl8812au_tx_pwr_idx_override_armed
            : curr_settings.wb_tx_power_milli_watt_armed;
    card_stats.curr_status = m_wb_txrx->get_card_has_disconnected(i) ? 1 : 0;
    card_stats.card_type = wifi_card_type_to_int(card.type);
    card_stats.card_sub_type = card.sub_type;
    // m_console->debug("Signal quality {}",card_stats.signal_quality);
  }
  stats.is_air = m_profile.is_air;
  stats.ready = true;
  openhd::LinkActionHandler::instance().update_link_stats(stats);
  if (m_profile.is_ground()) {
    if (rxStats.likely_mismatching_encryption_key) {
      const auto elapsed =
          std::chrono::steady_clock::now() - m_last_log_bind_phrase_mismatch;
      if (elapsed > std::chrono::seconds(3)) {
        m_console->warn("Bind phrase mismatch");
        m_last_log_bind_phrase_mismatch = std::chrono::steady_clock::now();
      }
    }
  }
  // m_console->debug("Last received packet mcs:{}
  // chan_width:{}",rxStats.last_received_packet_mcs_index,rxStats.last_received_packet_channel_width);
}

void WBLink::wt_perform_rate_adjustment() {
  using namespace openhd::wb;
  if (!m_profile.is_air) return;  // Only done on air unit
  // Rate adjustment is done on air and only if enabled
  if (!(m_profile.is_air &&
        m_settings->get_settings().enable_wb_video_variable_bitrate)) {
    return;
  }
  const auto& settings = m_settings->get_settings();
  const auto& card = m_broadcast_cards.at(0);
  // First we calculate the theoretical rate for the current "wifi config" aka
  // taking mcs index, channel width, ... into account
  const int max_rate_for_current_wifi_config =
      calculate_bitrate_for_wifi_config_kbits(
          card, settings.wb_frequency, settings.wb_air_tx_channel_width,
          settings.wb_air_mcs_index,
          settings.wb_video_rate_for_mcs_adjustment_percent, false);
  m_max_total_rate_for_current_wifi_config_kbits =
      max_rate_for_current_wifi_config;
  // Subtract the FEC overhead from (video) bitrate
  const int max_video_rate_for_current_wifi_fec_config =
      openhd::wb::deduce_fec_overhead(max_rate_for_current_wifi_config,
                                      settings.wb_video_fec_percentage);
  // const auto stats=m_wb_txrx->get_rx_stats();
  // m_foreign_p_helper.update(stats.count_p_any,stats.count_p_valid);
  // m_console->debug("N foreign packets per second
  // :{}",m_foreign_p_helper.get_foreign_packets_per_second());
  if (m_max_video_rate_for_current_wifi_fec_config !=
          max_video_rate_for_current_wifi_fec_config ||
      m_rate_adjustment_frequency_changed) {
    m_rate_adjustment_frequency_changed = false;
    // Apply the default for this configuration, then return - we will start the
    // auto-adjustment depending on tx error(s) next time the rate adjustment is
    // called
    m_console->debug(
        "MCS:{} ch_width:{} Calculated max_rate:{}, max_video_rate:{}",
        settings.wb_air_mcs_index, settings.wb_air_tx_channel_width,
        openhd::kbits_per_second_to_string(max_rate_for_current_wifi_config),
        openhd::kbits_per_second_to_string(
            max_video_rate_for_current_wifi_fec_config));
    m_max_video_rate_for_current_wifi_fec_config =
        max_video_rate_for_current_wifi_fec_config;
    m_recommended_video_bitrate_kbits =
        m_max_video_rate_for_current_wifi_fec_config;
    m_curr_n_rate_adjustments = 0;
    recommend_bitrate_to_encoder(m_recommended_video_bitrate_kbits);
    // We 'give' the camera up to X seconds to adjust to the newly set rate. If
    // we drop frames during this period, we do not count it as errors that need
    // bitrate reduction.
    m_frame_drop_helper.delay_for(std::chrono::seconds(5));
    m_primary_total_dropped_frames = 0;
    m_secondary_total_dropped_frames = 0;
    return;
  }
  // const bool
  // dropping_many_frames=m_frame_drop_helper.needs_bitrate_reduction();
  const bool dropping_many_frames = false;
  // m_console->debug("Dropped since last check:{}",dropped_since_last_check);
  if (dropping_many_frames) {
    // We are dropping frames / too many tx error hint(s), we need to reduce
    // bitrate. Reduce video bitrate by 1MBit/s
    m_recommended_video_bitrate_kbits -= 1000;
    m_curr_n_rate_adjustments++;
    // Safety, in case we fall below a certain threshold the encoder won't be
    // able to produce an image at some point anyways.
    static constexpr auto MIN_BITRATE_KBITS = 1000 * 2;
    if (m_recommended_video_bitrate_kbits < MIN_BITRATE_KBITS) {
      m_console->warn("Reached minimum bitrate {}",
                      openhd::kbits_per_second_to_string(MIN_BITRATE_KBITS));
      m_recommended_video_bitrate_kbits = MIN_BITRATE_KBITS;
      m_curr_n_rate_adjustments--;
    }
    m_console->warn("TX errors, reducing video bitrate to {}",
                    m_recommended_video_bitrate_kbits);
  }
  // Extra x20 - thermal protection
  if (OHDPlatform::instance().is_x20()) {
    const int x20_rate = m_thermal_protection_level > 0
                             ? m_recommended_video_bitrate_kbits * 30 / 100
                             : m_recommended_video_bitrate_kbits;
    recommend_bitrate_to_encoder(x20_rate);
    return;
  }
  recommend_bitrate_to_encoder(m_recommended_video_bitrate_kbits);
}

void WBLink::recommend_bitrate_to_encoder(int recommended_video_bitrate_kbits) {
  // Since settings might change dynamically at run time, we constantly
  // recommend a bitrate to the encoder / camera - The camera is responsible for
  // "not doing anything" when we recommend the same bitrate to it multiple
  // times
  /*if(!m_opt_action_handler){
      m_console->debug("No action handler,cannot recommend bitrate to camera");
      return;
  }*/
  openhd::LinkActionHandler::LinkBitrateInformation lb{};
  lb.recommended_encoder_bitrate_kbits = recommended_video_bitrate_kbits;
  openhd::LinkActionHandler::instance().action_request_bitrate_change_handle(
      lb);
}

bool WBLink::try_schedule_work_item(
    const std::shared_ptr<WorkItem>& work_item) {
  std::unique_lock<std::mutex> lock(m_work_item_queue_mutex, std::try_to_lock);
  if (lock.owns_lock()) {
    if (m_work_item_queue.empty()) {
      m_console->debug("Adding work item {} to queue", work_item->TAG);
      m_work_item_queue.push(work_item);
      return true;
    }
    m_console->debug("Work queue full,cannot add {}", work_item->TAG);
    m_console->warn("Please try again later");
    return false;
  }
  // Most likely, the lock is hold by the wb_link thread currently performing a
  // previous work item - this is not an error, the user has to try changing
  // param X later.
  m_console->debug("Cannot get lock,cannot add {}", work_item->TAG);
  m_console->warn("Please try again later");
  return false;
}

void WBLink::transmit_telemetry_data(TelemetryTxPacket packet) {
  assert(packet.n_injections >= 1);
  // m_console->debug("N injections:{}",packet.n_injections);
  const auto n_dropped =
      m_wb_tele_tx->enqueue_packet_dropping(packet.data, packet.n_injections);
  if (n_dropped > 0) {
    m_console->debug("Telemetry queue jam, dropped {}", n_dropped);
  }
}

void WBLink::transmit_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  assert(m_profile.is_air);
  if (stream_index < 0 || stream_index > m_wb_video_tx_list.size()) {
    m_console->debug("Invalid camera stream_index {}", stream_index);
    return;
  }
  if (m_air_close_video_in.load(std::memory_order_relaxed)) {
    m_console->debug("Video TX temporarily disabled");
    return;
  }
  if (m_thermal_protection_level.load(std::memory_order_relaxed) >=
      THERMAL_PROTECTION_VIDEO_DISABLED) {
    // Thermal protection disable video active, don't transmit video
    return;
  }
  // m_console->debug("Got {}",fragmented_video_frame.rtp_fragments.size());
  auto& tx = *m_wb_video_tx_list[stream_index];
  tx.set_encryption(fragmented_video_frame.enable_ultra_secure_encryption);
  const int max_fec_block_size = get_max_fec_block_size();
  const int fec_perc = m_settings->get_settings().wb_video_fec_percentage;
  int n_dropped_frames = 0;
  if (fragmented_video_frame.dirty_frame != nullptr) {
    // non rtp
    const auto res = tx.try_enqueue_frame(fragmented_video_frame.dirty_frame,
                                          max_fec_block_size, fec_perc,
                                          fragmented_video_frame.creation_time);
    if (!res) {
      // We dropped this frame
      n_dropped_frames = 1;
    }
  } else {
    // Pushes out previous enqueued frames if there is not enough space in the
    // queue
    const bool use_dropping_enqueue = fragmented_video_frame.is_intra_stream ||
                                      fragmented_video_frame.is_idr_frame;

    if (use_dropping_enqueue) {
      const auto count_removed = tx.enqueue_block_dropping(
          fragmented_video_frame.rtp_fragments, max_fec_block_size, fec_perc,
          fragmented_video_frame.creation_time);
      if (count_removed != 0) {
        openhd::log::get_default()->debug(
            "Cleared {} frames to make space for frame {}", count_removed,
            fragmented_video_frame.to_string());
        n_dropped_frames = count_removed;
      }
    } else {
      const auto res = tx.try_enqueue_block(
          fragmented_video_frame.rtp_fragments, max_fec_block_size, fec_perc,
          fragmented_video_frame.creation_time);
      if (!res) {
        n_dropped_frames = 1;
        m_console->debug("TX enqueue video frame failed, queue size:{}",
                         tx.get_tx_queue_available_size_approximate());
      }
    }
  }
  if (n_dropped_frames != 0) {
    m_frame_drop_helper.notify_dropped_frame(n_dropped_frames);
    if (stream_index == 0) {
      m_primary_total_dropped_frames += n_dropped_frames;
    } else {
      m_secondary_total_dropped_frames += n_dropped_frames;
    }
  }
}

void WBLink::transmit_audio_data(const openhd::AudioPacket& audio_packet) {
  if (m_wb_audio_tx) {
    m_wb_audio_tx->try_enqueue_packet(audio_packet.data);
  }
}

void WBLink::reset_all_rx_stats() {
  m_wb_txrx->rx_reset_stats();
  for (auto& rx : m_wb_video_rx_list) {
    rx->reset_stream_stats();
  }
  m_wb_tele_rx->reset_stream_stats();
}

openhd::WifiSpace WBLink::get_current_frequency_channel_space() const {
  return openhd::get_space_from_frequency(
      m_settings->get_settings().wb_frequency);
}

void WBLink::perform_channel_scan(
    const openhd::LinkActionHandler::ScanChannelsParam& scan_channels_params) {
  const WiFiCard& card = m_broadcast_cards.at(0);
  const auto channels_to_scan = openhd::wb::get_scan_channels_frequencies(
      card, scan_channels_params.channels_to_scan);
  if (channels_to_scan.empty()) {
    m_console->warn("No channels to scan, return early");
    return;
  }
  // const auto channel_widths_to_scan=
  //         openhd::wb::get_scan_channels_bandwidths(scan_channels_params.check_20Mhz_channel_width_if_card_supports,
  //                                                  scan_channels_params.check_40Mhz_channel_width_if_card_supports);
  // if(channel_widths_to_scan.empty()){
  //   m_console->warn("No channel_widths to scan, return early");
  //   return;
  // }
  //  We only scan 40Mhz, this way we get both 20Mhz and 40Mhz air unit(s)
  const std::vector<uint16_t> channel_widths_to_scan = {40};

  auto stats_current = openhd::LinkActionHandler::instance().get_link_stats();
  stats_current.gnd_operating_mode.operating_mode = 1;
  openhd::LinkActionHandler::instance().update_link_stats(stats_current);

  struct ScanResult {
    bool success = false;
    int frequency = 0;
    int channel_width = 0;
  };
  ScanResult result{false, 0, 0};
  // Note: We intentionally do not modify the persistent settings here
  m_console->debug(
      "Channel scan N channels to scan:{} N channel widths to scan:{}",
      channels_to_scan.size(), channel_widths_to_scan.size());
  bool done_early = false;
  // We need to loop through all possible channels
  for (int i = 0; i < channels_to_scan.size(); i++) {
    const auto& channel = channels_to_scan[i];
    if (done_early) break;
    // and all possible channel widths (20 or 40Mhz only right now)
    for (const auto& channel_width : channel_widths_to_scan) {
      // Return early in some cases (e.g. when we have a low loss and are quite
      // certain about a frequency)
      if (done_early) break;
      // Skip channels / frequencies the card doesn't support anyways
      if (!openhd::wb::any_card_support_frequency(
              channel.frequency, m_broadcast_cards, m_console)) {
        continue;
      }
      // set new frequency, reset the packet count, sleep, then check if any
      // openhd packets have been received
      const bool freq_success = apply_frequency_and_channel_width(
          channel.frequency, channel_width, 20);
      if (!freq_success) {
        m_console->warn("Cannot scan [{}] {}Mhz@{}Mhz", channel.channel,
                        channel.frequency, channel_width);
        continue;
      }
      openhd::LinkActionHandler::ScanChannelsProgress tmp{};
      tmp.channel_mhz = (int)channel.frequency;
      tmp.channel_width_mhz = channel_width;
      tmp.success = false;
      tmp.progress =
          OHDUtil::calculate_progress_perc(i, (int)channels_to_scan.size());
      openhd::LinkActionHandler::instance().add_scan_channels_progress(tmp);
      // Disable injection during scan
      m_wb_txrx->set_passive_mode(true);
      // sleeep a bit - some cards /drivers might need time switching
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      m_console->debug("Scanning [{}] {}Mhz@{}Mhz", channel.channel,
                       channel.frequency, channel_width);
      reset_all_rx_stats();
      m_management_gnd->m_air_reported_curr_frequency = -1;
      m_management_gnd->m_air_reported_curr_channel_width = -1;
      std::this_thread::sleep_for(std::chrono::seconds(2));
      const auto n_likely_openhd_packets =
          m_wb_txrx->get_rx_stats().curr_n_likely_openhd_packets;
      // If we got what looks to be openhd packets, sleep a bit more such that
      // we can reliably get a management frame
      if (n_likely_openhd_packets > 0) {
        m_console->debug("Got {} likely openhd packets, sleep a bit more",
                         n_likely_openhd_packets);
        const auto begin_long_listen = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - begin_long_listen <
               std::chrono::seconds(5)) {
          const int air_center_frequency =
              m_management_gnd->m_air_reported_curr_frequency;
          const int air_tx_channel_width =
              m_management_gnd->m_air_reported_curr_channel_width;
          const bool has_received_management =
              air_center_frequency > 0 && air_tx_channel_width > 0;
          if (has_received_management) {
            break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
      }
      const auto packet_loss =
          m_wb_txrx->get_rx_stats().curr_lowest_packet_loss;
      const auto n_valid_packets = m_wb_txrx->get_rx_stats().count_p_valid;
      const int air_center_frequency =
          m_management_gnd->m_air_reported_curr_frequency;
      const int air_tx_channel_width =
          m_management_gnd->m_air_reported_curr_channel_width;
      m_console->debug(
          "Got {} packets on {}@{} air_reports:[{}@{}] with loss {}%",
          n_valid_packets, channel.frequency, channel_width,
          air_center_frequency, air_tx_channel_width, packet_loss);
      if (n_valid_packets > 0 && air_center_frequency > 0 &&
          (air_tx_channel_width == 20 || air_tx_channel_width == 40) &&
          channel.frequency == air_center_frequency) {
        m_console->debug("Found air unit");
        result.frequency = channel.frequency;
        result.channel_width = air_tx_channel_width;
        result.success = true;
        done_early = true;
      }
    }
  }
  re_enable_injection_unless_user_passive_mode_enabled();
  if (!result.success) {
    m_console->warn("Channel scan failure, restore local settings");
    apply_frequency_and_channel_width_from_settings();
    result.success = false;
    result.frequency = 0;
  } else {
    m_console->debug("Channel scan success, {}@{}Mhz", result.frequency,
                     result.channel_width);
    m_settings->unsafe_get_settings().wb_frequency = result.frequency;
    m_settings->persist();
    m_gnd_curr_rx_channel_width = result.channel_width;
    apply_frequency_and_channel_width_from_settings();
  }
  openhd::LinkActionHandler::ScanChannelsProgress tmp{};
  tmp.channel_mhz = (int)result.frequency;
  tmp.channel_width_mhz = result.channel_width;
  tmp.success = result.success;
  tmp.progress = 100;
  openhd::LinkActionHandler::instance().add_scan_channels_progress(tmp);
}

void WBLink::perform_channel_analyze(int channels_to_scan) {
  const auto analyze_begin = std::chrono::steady_clock::now();
  struct AnalyzeResult {
    int frequency;
    int n_foreign_packets;
  };
  const WiFiCard& card = m_broadcast_cards.at(0);
  const auto channels_to_analyze =
      openhd::wb::get_analyze_channels_frequencies(card, channels_to_scan);
  auto stats_current = openhd::LinkActionHandler::instance().get_link_stats();
  stats_current.gnd_operating_mode.operating_mode = 2;
  openhd::LinkActionHandler::instance().update_link_stats(stats_current);
  std::vector<AnalyzeResult> results{};
  for (int i = 0; i < channels_to_analyze.size(); i++) {
    const auto channel = channels_to_analyze[i];
    // We use fixed 40Mhz during analyze.
    const int channel_width = 40;
    // set new frequency, reset the packet count, sleep, then check if any
    // openhd packets have been received
    apply_frequency_and_channel_width(channel.frequency, channel_width, 20);
    // Disable injection during analyze
    m_wb_txrx->set_passive_mode(true);
    // Sleep a bit to give the card time to switch
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    m_console->debug("Analyzing [{}] {}Mhz@{}Mhz", channel.channel,
                     channel.frequency, channel_width);
    reset_all_rx_stats();
    std::this_thread::sleep_for(std::chrono::seconds(4));
    const auto stats = m_wb_txrx->get_rx_stats();
    const auto n_foreign_packets = stats.count_p_any - stats.count_p_valid;
    m_console->debug("Got {} foreign packets {}:{}", n_foreign_packets,
                     stats.count_p_any, stats.count_p_valid);
    results.push_back(
        AnalyzeResult{(int)channel.frequency, (int)n_foreign_packets});

    openhd::LinkActionHandler::AnalyzeChannelsResult tmp{};
    for (int j = 0; j < 30; j++) {
      if (j < results.size()) {
        tmp.channels_mhz[j] = results[j].frequency;
        tmp.foreign_packets[j] = results[j].n_foreign_packets;
      } else {
        tmp.channels_mhz[j] = 0;
        tmp.foreign_packets[j] = 0;
      }
    }
    tmp.progress = OHDUtil::calculate_progress_perc(
        i + 1, (int)channels_to_analyze.size());
    openhd::LinkActionHandler::instance().add_analyze_result(tmp);
  }
  /*std::stringstream ss;
  for(int i=0;i<results.size();i++){
      ss<<results[i].frequency<<"@"<<results[i].n_foreign_packets<<"\n";
  }
  m_console->debug("{}",ss.str().c_str());*/
  re_enable_injection_unless_user_passive_mode_enabled();
  m_console->debug(
      "Done analyzing, took:{}",
      MyTimeHelper::R(std::chrono::steady_clock::now() - analyze_begin));
  // Go back to the previous frequency
  apply_frequency_and_channel_width_from_settings();
}

void WBLink::wt_perform_mcs_via_rc_channel_if_enabled() {
  if (!m_profile.is_air) {
    return;
  }
  const auto& settings = m_settings->get_settings();
  if (settings.wb_mcs_index_via_rc_channel <=
      openhd::WB_MCS_INDEX_VIA_RC_CHANNEL_OFF) {
    // disabled
    return;
  }
  // 1= channel number 1 aka array index 0
  const int channel_index = (int)settings.wb_mcs_index_via_rc_channel - 1;
  const auto mcs_from_rc_opt =
      m_rc_channel_helper.get_mcs_from_rc_channel(channel_index, m_console);
  if (!mcs_from_rc_opt.has_value()) {
    return;
  }
  const auto& mcs_from_rc = mcs_from_rc_opt.value();
  if (settings.wb_air_mcs_index != mcs_from_rc) {
    m_console->debug("RC CHANNEL - changing MCS from {} to {} ",
                     settings.wb_air_mcs_index, mcs_from_rc);
    m_settings->unsafe_get_settings().wb_air_mcs_index = mcs_from_rc;
    m_settings->persist();
    m_request_apply_air_mcs_index = true;
  }
}

void WBLink::wt_perform_bw_via_rc_channel_if_enabled() {
  if (!m_profile.is_air) {
    return;
  }
  const auto& settings = m_settings->get_settings();
  const auto opt_rc_bw =
      m_rc_channel_helper.get_bw_from_rc_channel(settings.wb_bw_via_rc_channel);
  if (!opt_rc_bw.has_value()) return;
  const auto rc_bw = opt_rc_bw.value();
  if (settings.wb_air_tx_channel_width != rc_bw) {
    m_console->debug("RC CHANNEL - changing BW from {} to {} ",
                     settings.wb_air_tx_channel_width, rc_bw);
    m_settings->unsafe_get_settings().wb_air_tx_channel_width = rc_bw;
    m_settings->persist();
    m_request_apply_air_bw = true;
  }
}

void WBLink::update_arming_state(bool armed) {
  m_console->debug("update arming state, armed: {}", armed);
  // We just update the internal armed / disarmed state and then call
  // apply_tx_power - it will set the right tx power if the user enabled it
  m_is_armed = armed;
  m_request_apply_tx_power = true;
}

void WBLink::wt_gnd_perform_channel_management() {
  if (m_profile.is_ground()) {
    // Ground: Listen on the channel width the air reports (always works due to
    // management always on 20Mhz) And switch "up" to 40Mhz if needed
    // AND react to (announced) frequency changes (right now without any
    // recovery protocol)
    const int air_reported_channel_width =
        m_management_gnd->m_air_reported_curr_channel_width;
    const int air_reported_frequency =
        m_management_gnd->m_air_reported_curr_frequency;
    if ((air_reported_channel_width == 20 ||
         air_reported_channel_width == 40) &&
        air_reported_frequency > 100) {
      if (m_gnd_curr_rx_channel_width != air_reported_channel_width ||
          m_gnd_curr_rx_frequency != air_reported_frequency) {
        m_console->debug("GND changing from {}:{} to {}:{}",
                         m_gnd_curr_rx_frequency, m_gnd_curr_rx_channel_width,
                         air_reported_frequency, air_reported_channel_width);
        m_gnd_curr_rx_frequency = air_reported_frequency;
        m_gnd_curr_rx_channel_width = air_reported_channel_width;
        m_settings->unsafe_get_settings().wb_frequency = air_reported_frequency;
        m_settings->persist(false);
        apply_frequency_and_channel_width(air_reported_frequency,
                                          air_reported_channel_width, 20);
      }
    }
  }
}

void WBLink::re_enable_injection_unless_user_passive_mode_enabled() {
  bool enable_passive_mode = false;
  if (m_profile.is_ground() &&
      m_settings->get_settings().wb_enable_listen_only_mode) {
    enable_passive_mode = true;
  }
  m_wb_txrx->set_passive_mode(enable_passive_mode);
}

void WBLink::wt_perform_air_hotspot_after_timeout() {
  if (!m_profile.is_air) return;
  // If we are armed, we never go into hs mode
  if (openhd::ArmingStateHelper::instance().is_currently_armed()) {
    // Reset timeout
    m_hs_timeout = std::chrono::steady_clock::now();
  }
}

int WBLink::get_max_fec_block_size() {
  const int tmp = m_settings->get_settings().wb_max_fec_block_size;
  if (tmp < 0) {
    return m_recommended_max_fec_blk_size_for_this_platform;
  }
  if (tmp > 128) {
    m_console->warn("Invalid blk size");
    return 20;
  }
  return tmp;
}

void WBLink::on_wifi_card_fatal_error() {
  if (m_wifi_card_error_has_been_handled) return;
  m_console->error("on_wifi_card_fatal_error");
  // Terminate if we are air or if we are ground and have only one wifibroadcast
  // card connected. If we are ground and have more than one wifibroadcast card,
  // don't terminate, since the other card can still be used
  if (m_profile.is_air ||
      (m_profile.is_ground() && m_broadcast_cards.size() < 2)) {
    m_console->error("Terminating - disconnected card");
    openhd::TerminateHelper::instance().terminate_after(
        "CARD DISCONNECT", std::chrono::milliseconds(1));
  }
  m_wifi_card_error_has_been_handled = true;
}

void WBLink::wt_perform_update_thermal_protection() {
  if (!OHDPlatform::instance().is_x20()) {
    // Only works on x20
    return;
  }
  if (OHDFilesystemUtil::exists("/boot/openhd/disable_thermal_limits.txt")) {
    m_thermal_protection_level = THERMAL_PROTECTION_NONE;
    return;
  }
  auto temp = openhd::x20_read_rtl8812au_thermal_sensor_degree();
  if (temp <= 0) {
    m_thermal_protection_level = THERMAL_PROTECTION_NONE;
    return;
  }
  static auto THERMAL_LIMIT_VIDEO_REDUCED = 75;
  static auto THERMAL_LIMIT_VIDEO_OFF = 79;
  uint8_t new_thermal_protection_level;
  if (temp >= THERMAL_LIMIT_VIDEO_OFF) {
    // >=X degree, disable video
    new_thermal_protection_level = THERMAL_PROTECTION_VIDEO_DISABLED;
  } else if (temp >= THERMAL_LIMIT_VIDEO_REDUCED) {
    //  >=X degree, throttle video
    new_thermal_protection_level = THERMAL_PROTECTION_RATE_REDUCED;
  } else {  // no thermal protection
    new_thermal_protection_level = THERMAL_PROTECTION_NONE;
  }
  if (new_thermal_protection_level > m_thermal_protection_level) {
    // apply immediately
    m_thermal_protection_level = new_thermal_protection_level;
    m_thermal_protection_enable_tp = std::chrono::steady_clock::now();
  } else if (new_thermal_protection_level < m_thermal_protection_level) {
    const auto elapsed_since_thermal_protection_enabled =
        std::chrono::steady_clock::now() - m_thermal_protection_enable_tp;
    // Every time some type of thermal protection is activated, it stays active
    // for at least X seconds to avoid oscillating
    if (elapsed_since_thermal_protection_enabled < std::chrono::seconds(10)) {
      return;
    }
    if (m_thermal_protection_level == THERMAL_PROTECTION_VIDEO_DISABLED) {
      // When video streaming is disabled, we only re-enable it once we have
      // cooled down significantly
      if (temp <= 70) {
        m_thermal_protection_level = new_thermal_protection_level;
      }
    } else {
      m_thermal_protection_level = new_thermal_protection_level;
    }
  }
}
