#include "wb_link.h"
#include "wifi_command_helper.h"
//#include "wifi_command_helper2.h"

#include <utility>

#include "openhd_global_constants.hpp"
#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util_filesystem.h"
#include "openhd_reboot_util.h"
#include "openhd_bitrate_conversions.hpp"
#include "openhd_config.h"
#include "wb_link_helper.h"
#include "wifi_card.h"
#include "wb_link_rate_helper.hpp"

WBLink::WBLink(OHDProfile profile,OHDPlatform platform,std::vector<WiFiCard> broadcast_cards,std::shared_ptr<openhd::ActionHandler> opt_action_handler)
    : m_profile(std::move(profile)),
      m_platform(platform),
      m_broadcast_cards(std::move(broadcast_cards)),
      m_disable_all_frequency_checks(openhd::wb::disable_all_frequency_checks()),
      m_opt_action_handler(std::move(opt_action_handler))
{
  m_console = openhd::log::create_or_get("wb_streams");
  assert(m_console);
  m_any_card_supports_injection= false;
  for(const auto& card:m_broadcast_cards){
      if(card.supports_injection){
          m_any_card_supports_injection= true;
      }
  }
  m_console->info("Broadcast cards:{} any suports injection:{}",debug_cards(m_broadcast_cards),m_any_card_supports_injection);
  m_console->debug("m_disable_all_frequency_checks:{}",OHDUtil::yes_or_no(m_disable_all_frequency_checks));
  // sanity checks
  if(m_broadcast_cards.empty() || (m_profile.is_air && m_broadcast_cards.size()>1)) {
    // NOTE: Here we crash, since it would be a programmer(s) error
    // Air needs exactly one wifi card
    // ground supports rx diversity, therefore can have more than one card
    m_console->error("Without at least one wifi card, the stream(s) cannot be started");
    exit(1);
  }
  // this fetches the last settings, otherwise creates default ones
  m_settings =std::make_unique<openhd::WBStreamsSettingsHolder>(m_platform,m_profile,m_broadcast_cards);
  // fixup any settings coming from a previous use with a different wifi card (e.g. if user swaps around cards)
  openhd::wb::fixup_unsupported_frequency(*m_settings, m_broadcast_cards,
                                          m_console);
  takeover_cards_monitor_mode();
  WBTxRx::Options txrx_options{};
  txrx_options.session_key_packet_interval=SESSION_KEY_PACKETS_INTERVAL;
  txrx_options.use_gnd_identifier=m_profile.is_ground();
  txrx_options.debug_rssi= 0;
  txrx_options.debug_multi_rx_packets_variance= false;
  txrx_options.tx_without_pcap= true;
  txrx_options.enable_auto_switch_tx_card= false; //TODO remove me
  //txrx_options.debug_decrypt_time= true;
  //txrx_options.debug_encrypt_time= true;
  //txrx_options.debug_packet_gaps= true;
  const auto keypair_file= "/boot/openhd/txrx.key";
  if(OHDFilesystemUtil::exists(keypair_file)){
    txrx_options.secure_keypair=wb::read_keypair_from_file(keypair_file);
    m_console->debug("Using key from file {}",keypair_file);
  }else{
      txrx_options.secure_keypair = std::nullopt;
      m_console->debug("Using key from default bind phrase");
  }
  //txrx_options.log_all_received_packets= true;
  //txrx_options.log_all_received_validated_packets= true;
  //txrx_options.advanced_latency_debugging_rx=true;
  //const auto card_names = openhd::wb::get_card_names(m_broadcast_cards);
  //assert(!card_names.empty());
  std::vector<WBTxRx::WifiCard> tmp_wifi_cards;
  for(const auto& card: m_broadcast_cards){
      int wb_type=card.type==WiFiCardType::Realtek8812au ? 1 : 0;
      tmp_wifi_cards.push_back(WBTxRx::WifiCard{card.device_name,wb_type});
  }
  m_wb_txrx=std::make_shared<WBTxRx>(tmp_wifi_cards,txrx_options);
  const auto tx_radiotap_params=create_radiotap_params();
  m_console->debug("{}",RadiotapHeader::user_params_to_string(tx_radiotap_params));
  m_wb_txrx->tx_threadsafe_update_radiotap_header(tx_radiotap_params);
  {
      // Setup the tx & rx instances for telemetry. Telemetry is bidirectional,aka
      // tx radio port on air is the same as rx on ground and verse visa
      const auto radio_port_rx = m_profile.is_air ? openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT : openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT;
      const auto radio_port_tx = m_profile.is_air ? openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT : openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT;
      auto cb=[this](const uint8_t* data, int data_len){
        auto shared=std::make_shared<std::vector<uint8_t>>(data,data+data_len);
        on_receive_telemetry_data(shared);
      };
      m_wb_tele_rx = create_wb_rx(radio_port_rx, false, cb);
      m_wb_tele_tx = create_wb_tx(radio_port_tx, false);
      m_wb_tele_tx->set_encryption(true);
  }
  {
      // Video is unidirectional, aka always goes from air pi to ground pi
      if (m_profile.is_air) {
          // we transmit video
          auto primary = create_wb_tx(openhd::VIDEO_PRIMARY_RADIO_PORT, true);
          auto secondary = create_wb_tx(openhd::VIDEO_SECONDARY_RADIO_PORT, true);
          primary->set_encryption(m_settings->get_settings().wb_air_enable_video_encryption);
          secondary->set_encryption(m_settings->get_settings().wb_air_enable_video_encryption);
          m_wb_video_tx_list.push_back(std::move(primary));
          m_wb_video_tx_list.push_back(std::move(secondary));
      } else {
          // we receive video
          auto cb1=[this](const uint8_t* data,int data_len){
            on_receive_video_data(0,data,data_len);
          };
          auto cb2=[this](const uint8_t* data,int data_len){
            on_receive_video_data(1,data,data_len);
          };
          auto primary = create_wb_rx(openhd::VIDEO_PRIMARY_RADIO_PORT, true,cb1);
          auto secondary = create_wb_rx(openhd::VIDEO_SECONDARY_RADIO_PORT, true,cb2);
          m_wb_video_rx_list.push_back(std::move(primary));
          m_wb_video_rx_list.push_back(std::move(secondary));
      }
  }
  apply_frequency_and_channel_width_from_settings();
  apply_txpower();
  m_wb_txrx->start_receiving();
  m_work_thread_run = true;
  m_work_thread =std::make_unique<std::thread>(&WBLink::loop_do_work, this);
  if(m_opt_action_handler){
      std::function<bool(openhd::ActionHandler::ScanChannelsParam)> cb_scan=[this](openhd::ActionHandler::ScanChannelsParam param){
        return async_scan_channels(param);
      };
      m_opt_action_handler->wb_cmd_scan_channels=cb_scan;
      std::function<bool()> cb_analyze=[this](){
          return async_analyze_channels();
      };
      m_opt_action_handler->wb_cmd_analyze_channels=cb_analyze;
      if(m_profile.is_air){
          // MCS is only changed on air
          auto cb_mcs=[this](const std::array<int,18>& rc_channels){
            set_mcs_index_from_rc_channel(rc_channels);
          };
          m_opt_action_handler->action_on_any_rc_channel_register(cb_mcs);
      }
      auto cb_arm=[this](bool armed){
        update_arming_state(armed);
      };
      m_opt_action_handler->m_action_tx_power_when_armed=std::make_shared<openhd::ActionHandler::ACTION_TX_POWER_WHEN_ARMED>(cb_arm);
      std::function<std::vector<uint16_t>(void)> wb_get_supported_channels=[this](){
          std::vector<uint16_t> ret;
          const auto frequencies=m_broadcast_cards.at(0).get_supported_frequencies_2G_5G();
          for(const auto freq:frequencies){
              ret.push_back(static_cast<uint16_t>(freq));
          }
          return ret;
      };
      m_opt_action_handler->wb_get_supported_channels= wb_get_supported_channels;
  }
}

WBLink::~WBLink() {
  m_console->debug("WBLink::~WBLink() begin");
  if(m_opt_action_handler){
      m_opt_action_handler->action_on_any_rc_channel_register(nullptr);
      m_opt_action_handler->m_action_tx_power_when_armed= nullptr;
      m_opt_action_handler->wb_get_supported_channels= nullptr;
      m_opt_action_handler->wb_cmd_scan_channels= nullptr;
      m_opt_action_handler->wb_cmd_analyze_channels=nullptr;
  }
  if(m_work_thread){
    m_work_thread_run =false;
    m_work_thread->join();
  }
  m_wb_txrx->stop_receiving();
  // stop all the receiver/transmitter instances, after that, give card back to network manager
  m_wb_tele_rx.reset();
  m_wb_tele_tx.reset();
  m_wb_video_tx_list.resize(0);
  m_wb_video_rx_list.resize(0);
  // give the monitor mode cards back to network manager
  for(const auto& card: m_broadcast_cards){
    wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, true);
  }
  m_console->debug("WBLink::~WBLink() end");
}

void WBLink::takeover_cards_monitor_mode() {
  m_console->debug( "takeover_cards_monitor_mode() begin");
  // We need to take "ownership" from the system over the cards used for monitor mode / wifibroadcast.
  // This can be different depending on the OS we are running on - in general, we try to go for the following with openhd:
  // Have network manager running on the host OS - the nice thing about network manager is that we can just tell it
  // to ignore the cards we are doing wifibroadcast with, instead of killing all processes that might interfere with
  // wifibroadcast and therefore making other networking incredibly hard.
  // Tell network manager to ignore the cards we want to do wifibroadcast on
  for(const auto& card: m_broadcast_cards){
    wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, false);
  }
  wifi::commandhelper::rfkill_unblock_all();
  // TODO: sometimes this happens:
  // 1) Running openhd fist time: pcap_compile doesn't work (fatal error)
  // 2) Running openhd second time: works
  // I cannot find what's causing the issue - a sleep here is the worst solution, but r.n the only one I can come up with
  // perhaps we'd need to wait for network manager to finish switching to ignoring the monitor mode cards ?!
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // now we can enable monitor mode on the given cards.
  for(const auto& card: m_broadcast_cards) {
    wifi::commandhelper::ip_link_set_card_state(card.device_name, false);
    wifi::commandhelper::iw_enable_monitor_mode(card.device_name);
    wifi::commandhelper::ip_link_set_card_state(card.device_name, true);
    //wifi::commandhelper2::set_wifi_monitor_mode(card->_wifi_card.interface_name);
  }
  m_console->debug("takeover_cards_monitor_mode() end");
}

RadiotapHeader::UserSelectableParams WBLink::create_radiotap_params()const {
  const auto settings=m_settings->get_settings();
  auto mcs_index=static_cast<int>(settings.wb_mcs_index);
  if(m_profile.is_ground()){
      // Always use mcs 0 on ground
      mcs_index =openhd::WB_GND_UPLINK_MCS_INDEX;
  }
  const auto channel_width=static_cast<int>(settings.wb_channel_width);
  return RadiotapHeader::UserSelectableParams{
      channel_width, settings.wb_enable_short_guard,settings.wb_enable_stbc,
      settings.wb_enable_ldpc, mcs_index,!settings.wb_tx_use_ack};
}


std::unique_ptr<WBStreamTx> WBLink::create_wb_tx(uint8_t radio_port,bool is_video) {
  WBStreamTx::Options options{};
  options.enable_fec=is_video;
  options.radio_port=radio_port;
  if(is_video){
    options.block_data_queue_size=2;
    options.log_time_blocks_until_tx= true;
  }
  auto ret=std::make_unique<WBStreamTx>(m_wb_txrx, options);
  return ret;
}

std::unique_ptr<WBStreamRx> WBLink::create_wb_rx(uint8_t radio_port,bool is_video,WBStreamRx::OUTPUT_DATA_CALLBACK cb){
  WBStreamRx::Options options{};
  options.enable_fec=is_video;
  options.radio_port=radio_port;
  options.enable_fec_debug_log=false;
  if(!is_video){
    // We use threading for telemetry, since the callback where we process telemetry data
    // can hang for a while in some cases
    options.enable_threading= true;
    options.packet_queue_size=20;
  }
  auto ret=std::make_unique<WBStreamRx>(m_wb_txrx, options);
  ret->set_callback(cb);
  return ret;
}

bool WBLink::request_set_frequency(int frequency) {
  m_console->debug("request_set_frequency {}",frequency);
  if(m_disable_all_frequency_checks){
    m_console->warn("Not sanity checking frequency");
  }else{
    if(!openhd::wb::all_cards_support_frequency(frequency,m_broadcast_cards,m_console)){
        m_console->warn("Cannot change frequency, at least one card doesn't support");
      return false;
    }
    if(!openhd::wb::all_cards_support_frequency_and_channel_width(frequency,m_settings->get_settings().wb_channel_width,m_broadcast_cards,m_console)){
        m_console->warn("Cannot change frequency, 40Mhz not allowed (on at least one card)");
        return false;
    }
  }
  if(!check_in_state_support_changing_settings())return false;
  // We need to delay the change to make sure the mavlink ack has enough time to make it to the ground
  auto work_item=std::make_shared<WorkItem>(fmt::format("SET_FREQ:{}",frequency),[this,frequency](){
      m_settings->unsafe_get_settings().wb_frequency=frequency;
      m_settings->persist();
      if(m_profile.is_air){
          // Wait a bit for the ack
          std::this_thread::sleep_for(DELAY_FOR_TRANSMIT_ACK);
      }
     apply_frequency_and_channel_width_from_settings();
  },std::chrono::steady_clock::now());
  return try_schedule_work_item(work_item);
}

bool WBLink::request_set_channel_width(int channel_width) {
    m_console->debug("request_set_channel_width {}",channel_width);
    if(!openhd::is_valid_channel_width(channel_width)){
        m_console->warn("Invalid channel width {}",channel_width);
        return false;
    }
    // On the ground, it doesn't really matter if the card actually supports injecting 40Mhz, only if it receives 40Mhz
    if(m_profile.is_air){
        // We only have one tx card, check if it supports injecting with 40Mhz channel width:
        if(channel_width==40 && !wifi_card_supports_40Mhz_channel_width_injection(m_broadcast_cards.at(0))){
            m_console->warn("Cannot change channel width, not supported by card");
            return false;
        }
    }
    if(!openhd::wb::all_cards_support_frequency_and_channel_width(m_settings->unsafe_get_settings().wb_frequency,channel_width,m_broadcast_cards,m_console)){
        m_console->warn("Cannot change channel width, 40Mhz not possible on this channel");
        return false;
    }
    if(!check_in_state_support_changing_settings())return false;
    // We need to delay the change to make sure the mavlink ack has enough time to make it to the ground
    auto work_item=std::make_shared<WorkItem>(fmt::format("SET_CHWIDTH:{}",channel_width),[this,channel_width](){
        m_settings->unsafe_get_settings().wb_channel_width=channel_width;
        m_settings->persist();
        if(m_profile.is_air){
            // Wait a bit for the ack
            std::this_thread::sleep_for(DELAY_FOR_TRANSMIT_ACK);
        }
        apply_frequency_and_channel_width_from_settings();
    },std::chrono::steady_clock::now());
    return try_schedule_work_item(work_item);
}

bool WBLink::apply_frequency_and_channel_width(uint32_t frequency, uint32_t channel_width) {
  const auto res=openhd::wb::set_frequency_and_channel_width_for_all_cards(frequency,channel_width,m_broadcast_cards);
  m_wb_txrx->tx_update_channel_width(channel_width);
  m_wb_txrx->tx_reset_stats();
  m_wb_txrx->rx_reset_stats();
  return res;
}

bool WBLink::apply_frequency_and_channel_width_from_settings() {
  const auto settings=m_settings->get_settings();
  const auto res=apply_frequency_and_channel_width(settings.wb_frequency,settings.wb_channel_width);
  m_wb_txrx->tx_reset_stats();
  m_wb_txrx->rx_reset_stats();
  m_max_video_rate_for_current_wifi_config_freq_changed= true;
  return res;
}

void WBLink::apply_txpower() {
  const auto settings=m_settings->get_settings();
  const auto before=std::chrono::steady_clock::now();
  uint32_t pwr_index=(int)settings.wb_rtl8812au_tx_pwr_idx_override;
  uint32_t pwr_mw=(int)settings.wb_tx_power_milli_watt;
  if(m_is_armed && settings.wb_rtl8812au_tx_pwr_idx_override_armed != openhd::RTL8812AU_TX_POWER_INDEX_ARMED_DISABLED){
      m_console->debug("Using power index special for armed");
      pwr_index=settings.wb_rtl8812au_tx_pwr_idx_override_armed;
  }
  if(m_is_armed && settings.wb_tx_power_milli_watt_armed != openhd::WIFI_TX_POWER_MILLI_WATT_ARMED_DISABLED){
    m_console->debug("Using power mw special for armed");
    pwr_mw=settings.wb_tx_power_milli_watt_armed;
  }
  openhd::wb::set_tx_power_for_all_cards(pwr_mw,pwr_index,m_broadcast_cards);
  m_curr_tx_power_mw=pwr_mw;
  m_curr_tx_power_idx=pwr_index;
  const auto delta=std::chrono::steady_clock::now()-before;
  m_console->debug("Changing tx power took {}",MyTimeHelper::R(delta));
}

bool WBLink::set_mcs_index(int mcs_index) {
  m_console->debug("set_mcs_index {}",mcs_index);
  if(!openhd::is_valid_mcs_index(mcs_index)){
    m_console->warn("Invalid mcs index{}",mcs_index);
    return false;
  }
  if(!all_cards_support_setting_mcs_index(m_broadcast_cards)){
    m_console->warn("Cannot change mcs index, it is fixed for at least one of the used cards");
    return false;
  }
  m_settings->unsafe_get_settings().wb_mcs_index=mcs_index;
  m_settings->persist();
  // R.n the only card known to properly allow setting the MCS index is rtl8812au,
  // and there it is done by modifying the radiotap header
  //for(const auto& wlan:m_broadcast_cards){
  //  wifi::commandhelper::iw_set_rate_mcs(wlan.device_name,settings.wb_mcs_index, false);
  //}
  m_wb_txrx->tx_update_mcs_index(mcs_index);
  return true;
}

bool WBLink::set_video_fec_percentage(int fec_percentage) {
  m_console->debug("set_video_fec_percentage {}",fec_percentage);
  if(!openhd::is_valid_fec_percentage(fec_percentage)){
    m_console->warn("Invalid fec percentage:{}",fec_percentage);
    return false;
  }
  m_settings->unsafe_get_settings().wb_video_fec_percentage=fec_percentage;
  m_settings->persist();
  return true;
}


std::vector<openhd::Setting> WBLink::get_all_settings(){
  using namespace openhd;
  std::vector<openhd::Setting> ret{};
  const auto settings=m_settings->get_settings();
  auto change_freq=openhd::IntSetting{(int)settings.wb_frequency,[this](std::string,int value){
                                          return request_set_frequency(value);
                                        }};
  change_freq.get_callback=[this](){
      return m_settings->unsafe_get_settings().wb_frequency;
  };
  auto change_wb_channel_width=openhd::IntSetting{(int)settings.wb_channel_width,[this](std::string,int value){
                                                      return request_set_channel_width(value);
                                                    }};
  change_wb_channel_width.get_callback=[this](){
      return m_settings->unsafe_get_settings().wb_channel_width;
  };
  if(m_profile.is_air){
      // Changing the MCS index only serves a purpose on the ground
      auto change_wb_mcs_index=openhd::IntSetting{(int)settings.wb_mcs_index,[this](std::string,int value){
          return set_mcs_index(value);
      }};
      ret.push_back(Setting{WB_MCS_INDEX,change_wb_mcs_index});
  }
  ret.push_back(Setting{WB_FREQUENCY,change_freq});
  ret.push_back(Setting{WB_CHANNEL_WIDTH,change_wb_channel_width});
  if(m_profile.is_air){
    auto change_video_fec_percentage=openhd::IntSetting{(int)settings.wb_video_fec_percentage,[this](std::string,int value){
                                                            return set_video_fec_percentage(value);
                                                          }};
    ret.push_back(Setting{WB_VIDEO_FEC_PERCENTAGE,change_video_fec_percentage});
    auto cb_enable_wb_video_variable_bitrate=[this](std::string,int value){
      return set_enable_wb_video_variable_bitrate(value);
    };
    ret.push_back(Setting{WB_VIDEO_VARIABLE_BITRATE,openhd::IntSetting{(int)settings.enable_wb_video_variable_bitrate, cb_enable_wb_video_variable_bitrate}});
    auto cb_wb_max_fec_block_size_for_platform=[this](std::string,int value){
      return set_max_fec_block_size_for_platform(value);
    };
    ret.push_back(Setting{WB_MAX_FEC_BLOCK_SIZE_FOR_PLATFORM,openhd::IntSetting{(int)settings.wb_max_fec_block_size_for_platform, cb_wb_max_fec_block_size_for_platform}});
    auto cb_wb_video_rate_for_mcs_adjustment_percent=[this](std::string,int value){
      return set_wb_video_rate_for_mcs_adjustment_percent(value);
    };
    ret.push_back(Setting{WB_VIDEO_RATE_FOR_MCS_ADJUSTMENT_PERC,openhd::IntSetting{(int)settings.wb_video_rate_for_mcs_adjustment_percent, cb_wb_video_rate_for_mcs_adjustment_percent}});
    // changing the mcs index via rc channel only makes sense on air,
    // and is only possible if the card supports it
    if(openhd::wb::has_any_rtl8812au(m_broadcast_cards)){
      auto cb_mcs_via_rc_channel=[this](std::string,int value){
        if(value<0 || value>18)return false; // 0 is disabled, valid rc channel number otherwise
        // we check if this is enabled in regular intervals (whenever we get the rc channels message from the FC)
        m_settings->unsafe_get_settings().wb_mcs_index_via_rc_channel=value;
        m_settings->persist();
        return true;
      };
      ret.push_back(Setting{openhd::WB_MCS_INDEX_VIA_RC_CHANNEL,openhd::IntSetting{(int)settings.wb_mcs_index_via_rc_channel, cb_mcs_via_rc_channel}});
    }
  }
  if(m_profile.is_ground()){
    // We display the total n of detected RX cards such that users can validate their multi rx setup(s) if there is more than one rx card detected
    // (Note: air always has exactly one monitor mode wi-fi card)
    const int n_rx_cards=static_cast<int>(m_broadcast_cards.size());
    if(n_rx_cards>1){
      ret.push_back(openhd::create_read_only_int("WB_N_RX_CARDS",n_rx_cards));
    }
    // feature on the ground station only
    auto cb_passive=[this](std::string,int value){
      if(!validate_yes_or_no(value))return false;
      m_settings->unsafe_get_settings().wb_enable_listen_only_mode=value;
      m_settings->persist();
      m_wb_txrx->set_passive_mode(value);
      return true;
    };
    ret.push_back(Setting{openhd::WB_PASSIVE_MODE,openhd::IntSetting{(int)settings.wb_enable_listen_only_mode, cb_passive}});
  }
  const bool any_card_supports_stbc_ldpc_sgi=openhd::wb::any_card_supports_stbc_ldpc_sgi(m_broadcast_cards);
  // These 3 are only supported / known to work on rtl8812au (yet), therefore only expose them when rtl8812au is used
  if(any_card_supports_stbc_ldpc_sgi){
	// STBC - definitely for advanced users, but aparently it can have benefits.
	auto cb_wb_enable_stbc=[this](std::string,int stbc){
	  if(stbc<0 || stbc>3)return false;
	  m_settings->unsafe_get_settings().wb_enable_stbc=stbc;
	  m_settings->persist();
          m_wb_txrx->tx_update_stbc(stbc);
	  return true;
	};
	ret.push_back(openhd::Setting{WB_ENABLE_STBC,openhd::IntSetting{settings.wb_enable_stbc,cb_wb_enable_stbc}});
	// These 2 params are exposed by default from OpenHD, but whitelisted in QOpenHD to prevent inexperienced users from changing them
	auto cb_wb_enable_ldpc=[this](std::string,int ldpc){
	  if(!validate_yes_or_no(ldpc))return false;
	  m_settings->unsafe_get_settings().wb_enable_ldpc=ldpc;
	  m_settings->persist();
          m_wb_txrx->tx_update_ldpc(ldpc);
	  return true;
	};
	ret.push_back(openhd::Setting{WB_ENABLE_LDPC,openhd::IntSetting{settings.wb_enable_stbc,cb_wb_enable_ldpc}});
	auto cb_wb_enable_sg=[this](std::string,int short_gi){
	  if(!validate_yes_or_no(short_gi))return false;
	  m_settings->unsafe_get_settings().wb_enable_short_guard=short_gi;
	  m_settings->persist();
          m_wb_txrx->tx_update_guard_interval(short_gi);
	  return true;
	};
	ret.push_back(openhd::Setting{WB_ENABLE_SHORT_GUARD,openhd::IntSetting{settings.wb_enable_short_guard,cb_wb_enable_sg}});
  }
  // WIFI TX power depends on the used chips
  if(openhd::wb::has_any_rtl8812au(m_broadcast_cards)){
    auto cb_wb_rtl8812au_tx_pwr_idx_override=[this](std::string,int value){
      return set_tx_power_rtl8812au(value);
    };
    ret.push_back(openhd::Setting{WB_RTL8812AU_TX_PWR_IDX_OVERRIDE,openhd::IntSetting{(int)settings.wb_rtl8812au_tx_pwr_idx_override,cb_wb_rtl8812au_tx_pwr_idx_override}});
    auto cb_wb_rtl8812au_tx_pwr_idx_armed=[this](std::string,int value){
      if(!openhd::validate_wb_rtl8812au_tx_pwr_idx_override(value))return false;
      m_settings->unsafe_get_settings().wb_rtl8812au_tx_pwr_idx_override_armed=value;
      m_settings->persist();
      return true;
    };
    ret.push_back(openhd::Setting{WB_RTL8812AU_TX_PWR_IDX_ARMED,openhd::IntSetting{(int)settings.wb_rtl8812au_tx_pwr_idx_override_armed, cb_wb_rtl8812au_tx_pwr_idx_armed}});
  }
  if(openhd::wb::has_any_non_rtl8812au(m_broadcast_cards)){
      auto cb_wb_tx_power_milli_watt=[this](std::string,int value){
          return set_tx_power_mw(value);
      };
      auto change_tx_power=openhd::IntSetting{(int)settings.wb_tx_power_milli_watt,cb_wb_tx_power_milli_watt};
      ret.push_back(Setting{WB_TX_POWER_MILLI_WATT,change_tx_power});
      auto cb_wb_tx_power_milli_watt_armed=[this](std::string,int value){
          if(value<0 || value > 10000)return false;
          m_settings->unsafe_get_settings().wb_tx_power_milli_watt_armed=value;
          m_settings->persist();
          return true;
      };
      auto change_tx_power_armed=openhd::IntSetting{(int)settings.wb_tx_power_milli_watt_armed,cb_wb_tx_power_milli_watt_armed};
      ret.push_back(Setting{WB_TX_POWER_MILLI_WATT_ARMED,change_tx_power_armed});
  }
  if(m_profile.is_air){
      auto cb_video_encrypt=[this](std::string,int value){
          if(!openhd::validate_yes_or_no(value))return false;
          set_wb_air_video_encryption_enabled(value);
          return true;
      };
      auto change_video_encryption=openhd::IntSetting{(int)settings.wb_air_enable_video_encryption,cb_video_encrypt};
      ret.push_back(Setting{WB_VIDEO_ENCRYPTION_ENABLE,change_video_encryption});
  }
  openhd::validate_provided_ids(ret);
  return ret;
}

static uint32_t get_micros(std::chrono::nanoseconds ns){
  return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(ns).count());
}

void WBLink::loop_do_work() {
  while (m_work_thread_run){
    // Perform any queued up work if it exists
    {
      m_work_item_queue_mutex.lock();
      if(!m_work_item_queue.empty()){
        auto front=m_work_item_queue.front();
        if(front->ready_to_be_executed()){
          m_console->debug("Start execute work item {}",front->TAG);
          front->execute();
          m_console->debug("Done executing work item {}",front->TAG);
          m_work_item_queue.pop();
        }
      }
      m_work_item_queue_mutex.unlock();
    }
    // update statistics in regular intervals
    update_statistics();
    //const auto delta_calc_stats=std::chrono::steady_clock::now()-begin_calculate_stats;
    //m_console->debug("Calculating stats took:{} ms",std::chrono::duration_cast<std::chrono::microseconds>(delta_calc_stats).count()/1000.0f);
    // update recommended rate if enabled in regular intervals
    perform_rate_adjustment();
    // Dirty - deliberately crash openhd and let the service restart it
    // if we think a wi-fi card disconnected
    /*bool any_rx_wifi_disconnected_errors=false;
    if(m_wb_tele_rx->get_latest_stats().wb_rx_stats.n_receiver_likely_disconnect_errors>100){
      any_rx_wifi_disconnected_errors= true;
    }
    for(auto& rx: m_wb_video_rx_list){
      if(rx->get_latest_stats().wb_rx_stats.n_receiver_likely_disconnect_errors>100){
        any_rx_wifi_disconnected_errors= true;
      }
    }*/
    //if(any_rx_wifi_disconnected_errors){
    //  openhd::fatalerror::handle_needs_openhd_restart("wifi disconnected");
    //}
    std::this_thread::sleep_for(std::chrono::milliseconds (100));
  }
}

void WBLink::update_statistics() {
  const auto elapsed_since_last=std::chrono::steady_clock::now()-m_last_stats_recalculation;
  if(elapsed_since_last<RECALCULATE_STATISTICS_INTERVAL){
    return;
  }
  m_last_stats_recalculation=std::chrono::steady_clock::now();
  // telemetry is available on both air and ground
  openhd::link_statistics::StatsAirGround stats{};
  if(m_wb_tele_tx){
    const auto curr_tx_stats= m_wb_tele_tx->get_latest_stats();
    const auto curr_rx_stats=m_wb_tele_rx->get_latest_stats();
    stats.telemetry.curr_tx_bps=curr_tx_stats.current_provided_bits_per_second;
    stats.telemetry.curr_tx_pps=curr_tx_stats.current_injected_packets_per_second;
    stats.telemetry.curr_rx_bps=curr_rx_stats.curr_in_bits_per_second;
    stats.telemetry.curr_rx_pps=curr_rx_stats.curr_in_packets_per_second;
  }
  if(m_profile.is_air){
    // video on air
    for(int i=0;i< m_wb_video_tx_list.size();i++){
      auto& wb_tx= *m_wb_video_tx_list.at(i);
      //auto& air_video=i==0 ? stats.air_video0 : stats.air_video1;
      const auto curr_tx_stats=wb_tx.get_latest_stats();
      openhd::link_statistics::StatsWBVideoAir air_video{};
      air_video.link_index=i;
      int rec_bitrate=0;
      if(m_opt_action_handler){
        auto cam_stats=m_opt_action_handler->get_cam_info(i);
        rec_bitrate=cam_stats.encoding_bitrate_kbits;
      }
      air_video.curr_recommended_bitrate=rec_bitrate;
      air_video.curr_measured_encoder_bitrate=curr_tx_stats.current_provided_bits_per_second;
      air_video.curr_injected_bitrate=curr_tx_stats.current_injected_bits_per_second;
      air_video.curr_injected_pps=curr_tx_stats.current_injected_packets_per_second;
      air_video.curr_dropped_frames=curr_tx_stats.n_dropped_frames;
      const auto curr_tx_fec_stats=wb_tx.get_latest_fec_stats();
      air_video.curr_fec_encode_time_avg_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.avg);
      air_video.curr_fec_encode_time_min_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.min);
      air_video.curr_fec_encode_time_max_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.max);
      air_video.curr_fec_block_size_min=curr_tx_fec_stats.curr_fec_block_length.min;
      air_video.curr_fec_block_size_max=curr_tx_fec_stats.curr_fec_block_length.max;
      air_video.curr_fec_block_size_avg=curr_tx_fec_stats.curr_fec_block_length.avg;
      air_video.curr_fec_percentage=m_settings->unsafe_get_settings().wb_video_fec_percentage;
       // TODO otimization: Only send stats for an active link
      stats.stats_wb_video_air.push_back(air_video);
    }
  }else{
    // video on ground
    for(int i=0;i< m_wb_video_rx_list.size();i++){
      auto& wb_rx= *m_wb_video_rx_list.at(i);
      const auto wb_rx_stats=wb_rx.get_latest_stats();
      openhd::link_statistics::StatsWBVideoGround ground_video{};
      ground_video.link_index=i;
      // Use outgoing bitrate - otherwise, we get N times the bandwidth with multiple RX-es.
      //ground_video.curr_incoming_bitrate=wb_rx_stats.curr_in_bits_per_second;
      ground_video.curr_incoming_bitrate=wb_rx_stats.curr_out_bits_per_second;
      const auto fec_stats=wb_rx.get_latest_fec_stats();
      ground_video.count_fragments_recovered=fec_stats.count_fragments_recovered;
      ground_video.count_blocks_recovered=fec_stats.count_blocks_recovered;
      ground_video.count_blocks_lost=fec_stats.count_blocks_lost;
      ground_video.count_blocks_total=fec_stats.count_blocks_total;
      ground_video.curr_fec_decode_time_avg_us =get_micros(fec_stats.curr_fec_decode_time.avg);
      ground_video.curr_fec_decode_time_min_us =get_micros(fec_stats.curr_fec_decode_time.min);
      ground_video.curr_fec_decode_time_max_us =get_micros(fec_stats.curr_fec_decode_time.max);
      // TODO otimization: Only send stats for an active link
      stats.stats_wb_video_ground.push_back(ground_video);
    }
  }
  const auto& curr_settings=m_settings->unsafe_get_settings();
  const auto rxStats=m_wb_txrx->get_rx_stats();
  const auto txStats=m_wb_txrx->get_tx_stats();
  stats.monitor_mode_link.curr_rx_packet_loss_perc=rxStats.curr_lowest_packet_loss;
  stats.monitor_mode_link.count_tx_inj_error_hint=txStats.count_tx_injections_error_hint;
  stats.monitor_mode_link.count_tx_dropped_packets=get_total_dropped_packets();
  stats.monitor_mode_link.curr_tx_card_idx=m_wb_txrx->get_curr_active_tx_card_idx();
  stats.monitor_mode_link.curr_tx_mcs_index=curr_settings.wb_mcs_index;
  //m_console->debug("Big gaps:{}",rxStats.curr_big_gaps_counter);
  stats.monitor_mode_link.curr_tx_channel_mhz=curr_settings.wb_frequency;
  stats.monitor_mode_link.curr_tx_channel_w_mhz=curr_settings.wb_channel_width;
  stats.monitor_mode_link.tx_operating_mode =0;
  if(!m_any_card_supports_injection){
      stats.monitor_mode_link.tx_operating_mode=1;
  }
  if(curr_settings.wb_enable_listen_only_mode){
      stats.monitor_mode_link.tx_operating_mode=2;
  }
  stats.monitor_mode_link.curr_rate_kbits= m_max_total_rate_for_current_wifi_config_kbits;
  stats.monitor_mode_link.curr_n_rate_adjustments=m_curr_n_rate_adjustments;
  stats.monitor_mode_link.curr_tx_pps=txStats.curr_packets_per_second;
  stats.monitor_mode_link.curr_tx_bps=txStats.curr_bits_per_second_excluding_overhead;
  stats.monitor_mode_link.curr_rx_pps=rxStats.curr_packets_per_second;
  stats.monitor_mode_link.curr_rx_bps=rxStats.curr_bits_per_second;
  const auto bitfield_stbc_lpdc_sg= openhd::link_statistics::write_stbc_lpdc_shortguard_bitfield(curr_settings.wb_enable_stbc,curr_settings.wb_enable_ldpc,curr_settings.wb_enable_short_guard);
  stats.monitor_mode_link.curr_tx_stbc_lpdc_shortguard_bitfield=bitfield_stbc_lpdc_sg;
  stats.monitor_mode_link.curr_pollution_perc=rxStats.curr_link_pollution_perc;
  //m_console->debug("{}",WBTxRx::tx_stats_to_string(txStats));
  //m_console->debug("{}",WBTxRx::rx_stats_to_string(rxStats));
  //m_console->debug("Pollution: {}",rxStats.curr_link_pollution_perc);

  // dBm is per card, not per stream
  assert(stats.cards.size()>=4);
  // only populate actually used cards
  assert(m_broadcast_cards.size()<=stats.cards.size());
  for(int i=0;i< m_broadcast_cards.size();i++){
    const auto& card=m_broadcast_cards.at(i);
    auto& card_stats = stats.cards.at(i);
    auto rxStatsCard=m_wb_txrx->get_rx_stats_for_card(i);
    card_stats.rx_rssi_card=rxStatsCard.card_dbm;
    card_stats.rx_rssi_1=rxStatsCard.antenna1_dbm;
    card_stats.rx_rssi_2=rxStatsCard.antenna2_dbm;
    card_stats.count_p_received=rxStatsCard.count_p_valid;
    card_stats.count_p_injected=0; //TODO
    card_stats.curr_rx_packet_loss_perc=rxStatsCard.curr_packet_loss;
    card_stats.tx_power_current=card.type==WiFiCardType::Realtek8812au ?
                                m_curr_tx_power_idx.load() : m_curr_tx_power_mw.load();
    card_stats.tx_power_disarmed=card.type==WiFiCardType::Realtek8812au ?
            curr_settings.wb_rtl8812au_tx_pwr_idx_override : curr_settings.wb_tx_power_milli_watt;
    card_stats.tx_power_armed=card.type==WiFiCardType::Realtek8812au ?
                              curr_settings.wb_rtl8812au_tx_pwr_idx_override_armed : curr_settings.wb_tx_power_milli_watt_armed;
    card_stats.exists_in_openhd= true;
    card_stats.curr_status= m_wb_txrx->get_card_has_disconnected(i) ? 1 : 0;
    card_stats.signal_quality=rxStatsCard.signal_quality;
    card_stats.card_type= 0;
    if(card.type==WiFiCardType::Realtek8812au){
        card_stats.card_type=1;
    }
    if(card.type==WiFiCardType::Realtek88x2bu){
        card_stats.card_type=2;
    }
    //m_console->debug("Signal quality {}",card_stats.signal_quality);
  }
  stats.is_air=m_profile.is_air;
  stats.ready=true;
  if(m_opt_action_handler){
    m_opt_action_handler->update_link_stats(stats);
  }
  if(m_profile.is_ground()){
    if(rxStats.likely_mismatching_encryption_key){
        const auto elapsed=std::chrono::steady_clock::now()-m_last_log_bind_phrase_mismatch;
        if(elapsed>std::chrono::seconds(3)){
            m_console->warn("Bind phrase mismatch");
            m_last_log_bind_phrase_mismatch=std::chrono::steady_clock::now();
        }
    }
  }
  //m_console->debug("Last received packet mcs:{} chan_width:{}",rxStats.last_received_packet_mcs_index,rxStats.last_received_packet_channel_width);
}

void WBLink::perform_rate_adjustment() {
  // Rate adjustment is done on air and only if enabled
  if(!(m_profile.is_air && m_settings->get_settings().enable_wb_video_variable_bitrate)){
    return;
  }
  // We do it at a fixed interval
  const auto elapsed_since_last=std::chrono::steady_clock::now()-m_last_rate_adjustment;
  if(elapsed_since_last<RATE_ADJUSTMENT_INTERVAL){
    return;
  }
  // Since we only do it on the air, we only have one wifi card
  const auto card=m_broadcast_cards.at(0);
  m_last_rate_adjustment=std::chrono::steady_clock::now();
  // First we calculate the theoretical rate for the current "wifi config" aka taking mcs index, channel width, ... into account
  const auto settings = m_settings->get_settings();
  const auto wifi_space=openhd::get_space_from_frequency(settings.wb_frequency);
  const auto max_rate_for_current_wifi_config_without_adjust =
      openhd::wb::get_max_rate_possible(card,wifi_space,
                                           settings.wb_mcs_index,
                                           settings.wb_channel_width == 40);
  using namespace openhd::wb;
  const auto max_rate_for_current_wifi_config= multiply_by_perc(max_rate_for_current_wifi_config_without_adjust,m_settings->get_settings().wb_video_rate_for_mcs_adjustment_percent);
  m_max_total_rate_for_current_wifi_config_kbits=max_rate_for_current_wifi_config;
  /*m_console->debug("Max rate {} with {} perc {}",kbits_per_second_to_string(max_rate_for_current_wifi_config_without_adjust),
                   m_settings->get_settings().wb_video_rate_for_mcs_adjustment_percent,
                   kbits_per_second_to_string(max_rate_for_current_wifi_config));*/
  const auto max_video_rate_for_current_wifi_config =
      openhd::wb::deduce_fec_overhead(max_rate_for_current_wifi_config,settings.wb_video_fec_percentage);
  if(m_max_video_rate_for_current_wifi_config !=max_video_rate_for_current_wifi_config ||
        m_max_video_rate_for_current_wifi_config_freq_changed){
      m_max_video_rate_for_current_wifi_config_freq_changed= false;
    // Apply the default for this configuration, then return - we will start the auto-adjustment
    // depending on tx error(s) next time the rate adjustment is called
    m_console->debug("MCS:{} ch_width:{} Calculated max_rate:{}, max_video_rate:{}",
                     settings.wb_mcs_index,settings.wb_channel_width,
                     kbits_per_second_to_string(max_rate_for_current_wifi_config),
                     kbits_per_second_to_string(max_video_rate_for_current_wifi_config));
    m_max_video_rate_for_current_wifi_config =
        max_video_rate_for_current_wifi_config;
    m_recommended_video_bitrate_kbits = m_max_video_rate_for_current_wifi_config;
    m_n_detected_and_reset_tx_errors=0;
    m_last_total_tx_error_count=0;
    m_curr_n_rate_adjustments=0;
    if (m_opt_action_handler) {
      openhd::ActionHandler::LinkBitrateInformation lb{};
      lb.recommended_encoder_bitrate_kbits = m_recommended_video_bitrate_kbits;
      m_opt_action_handler->action_request_bitrate_change_handle(lb);
    }
    return;
  }
  // Check if we had any tx errors since last time we checked, resetting them every time
  const auto curr_total_tx_errors=get_total_dropped_packets();
  const auto delta_total_tx_errors=curr_total_tx_errors-m_last_total_tx_error_count;
  m_last_total_tx_error_count=curr_total_tx_errors;
  const bool has_tx_errors=delta_total_tx_errors>0;
  if(has_tx_errors){
    m_n_detected_and_reset_tx_errors++;
    m_console->warn("Got {} tx errors {} times",delta_total_tx_errors,m_n_detected_and_reset_tx_errors);
    const auto tmp_tx_stats=m_wb_txrx->get_tx_stats();
    m_console->debug("{}",WBTxRx::tx_stats_to_string(tmp_tx_stats));
  }else{
    if(m_n_detected_and_reset_tx_errors>0){
      m_console->warn("No tx errors after {}",m_n_detected_and_reset_tx_errors);
    }
    m_n_detected_and_reset_tx_errors=0;
  }
  if(m_n_detected_and_reset_tx_errors>=3){
    // We got tx errors N consecutive times, (resetting if there are no tx errors) - we need to reduce bitrate
    m_console->debug("Got m_n_detected_and_reset_tx_errors{} with max:{} recommended:{}",
                     m_n_detected_and_reset_tx_errors,
        m_max_video_rate_for_current_wifi_config,
        m_recommended_video_bitrate_kbits);
    m_n_detected_and_reset_tx_errors=0;
    // Reduce video bitrate by 1MBit/s
    m_recommended_video_bitrate_kbits -=1000;
    m_curr_n_rate_adjustments++;
    // Safety, in case we fall below a certain threshold the encoder won't be able to produce an image at some point anyways.
    static constexpr auto MIN_BITRATE_KBITS=1000*2;
    if(m_recommended_video_bitrate_kbits <MIN_BITRATE_KBITS){
      m_console->warn("Reached minimum bitrate {}", kbits_per_second_to_string(MIN_BITRATE_KBITS));
      m_recommended_video_bitrate_kbits =MIN_BITRATE_KBITS;
      m_curr_n_rate_adjustments--;
    }
    m_console->warn("TX errors, reducing video bitrate to {}",m_recommended_video_bitrate_kbits);
  }
  // Since settings might change dynamically at run time, we constantly recommend a bitrate to the encoder / camera -
  // The camera is responsible for "not doing anything" when we recommend the same bitrate to it multiple times
  if (m_opt_action_handler) {
    openhd::ActionHandler::LinkBitrateInformation lb{};
    lb.recommended_encoder_bitrate_kbits = m_recommended_video_bitrate_kbits;
    m_opt_action_handler->action_request_bitrate_change_handle(lb);
  }
}

bool WBLink::set_enable_wb_video_variable_bitrate(int value) {
  if(openhd::validate_yes_or_no(value)){
    // value is read in regular intervals.
    m_settings->unsafe_get_settings().enable_wb_video_variable_bitrate=value;
    m_settings->persist();
    return true;
  }
  return false;
}

bool WBLink::set_max_fec_block_size_for_platform(int value) {
  if(!openhd::valid_wb_max_fec_block_size_for_platform(value))return false;
  m_settings->unsafe_get_settings().wb_max_fec_block_size_for_platform=value;
  m_settings->persist();
  return true;
}

bool WBLink::set_wb_video_rate_for_mcs_adjustment_percent(int value) {
  if(value<=5 || value>1000)return false;
  m_settings->unsafe_get_settings().wb_video_rate_for_mcs_adjustment_percent=value;
  m_settings->persist();
  return true;
}

void WBLink::set_wb_air_video_encryption_enabled(bool enable) {
    if(enable){
      m_console->debug("Video encryption enabled:{}",enable);
    }
    m_settings->unsafe_get_settings().wb_air_enable_video_encryption=enable;
    m_settings->persist();
    for(auto& tx:m_wb_video_tx_list){
        tx->set_encryption(m_settings->get_settings().wb_air_enable_video_encryption);
    }
}

bool WBLink::try_schedule_work_item(const std::shared_ptr<WorkItem> &work_item) {
    std::unique_lock<std::mutex> lock(m_work_item_queue_mutex, std::try_to_lock);
    if(lock.owns_lock()){
        if(m_work_item_queue.empty()){
            m_console->debug("Adding work item {} to queue",work_item->TAG);
            m_work_item_queue.push(work_item);
            return true;
        }
        m_console->debug("Work queue full,cannot add {}",work_item->TAG);
        m_console->warn("Please try again later");
        return false;
    }
    m_console->debug("Cannot get lock,cannot add {}",work_item->TAG);
    m_console->warn("Please try again later");
    return false;
}

bool WBLink::check_work_queue_empty() {
    std::unique_lock<std::mutex> lock(m_work_item_queue_mutex, std::try_to_lock);
    if(lock.owns_lock()){
        if(!m_work_item_queue.empty()){
            m_console->info("Rejecting param, another change is still queued up");
            return false;
        }
        return true;
    }
    m_console->info("Work queue busy");
    return false;
}


void WBLink::transmit_telemetry_data(TelemetryTxPacket packet) {
  assert(packet.n_injections>=1);
  //m_console->debug("N injections:{}",packet.n_injections);
  const auto res=m_wb_tele_tx->try_enqueue_packet(packet.data,packet.n_injections);
  if(!res)m_console->debug("Enqueing tele packet failed");
  if(!m_any_card_supports_injection){
    const auto now=std::chrono::steady_clock::now();
    const auto elapsed=now-m_last_log_card_does_might_not_inject;
    if(elapsed>WARN_CARD_DOES_NOT_INJECT_INTERVAL){
      m_console->warn("TX (likely) not supported by card(s)",m_broadcast_cards.at(0).device_name);
      m_last_log_card_does_might_not_inject=now;
    }
  }
}

void WBLink::transmit_video_data(int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame){
  assert(m_profile.is_air);
  if(stream_index>=0 && stream_index< m_wb_video_tx_list.size()){
    auto& tx= *m_wb_video_tx_list[stream_index];
    //tx.tmp_feed_frame_fragments(fragmented_video_frame.frame_fragments,use_fixed_fec_instead);
    uint32_t max_block_size_for_platform=m_settings->get_settings().wb_max_fec_block_size_for_platform;
    //openhd::log::get_default()->debug("max_block_size_for_platform:{}",max_block_size_for_platform);
    if(!openhd::valid_wb_max_fec_block_size_for_platform(max_block_size_for_platform)){
      openhd::log::get_default()->warn("Invalid max_block_size_for_platform:{}",max_block_size_for_platform);
      max_block_size_for_platform=openhd::DEFAULT_MAX_FEC_BLK_SIZE_FOR_PLATFORM;
    }
    const int fec_perc=m_settings->get_settings().wb_video_fec_percentage;
    const auto res=tx.try_enqueue_block(fragmented_video_frame.frame_fragments, max_block_size_for_platform,fec_perc);
    if(!res){
        m_console->warn("TX enqueue video frame failed, {}",tx.get_tx_queue_available_size_approximate());
    }
  }else{
    m_console->debug("Invalid camera stream_index {}",stream_index);
  }
}

void WBLink::reset_all_rx_stats() {
  m_wb_txrx->rx_reset_stats();
  for(auto& rx:m_wb_video_rx_list){
    rx->reset_stream_stats();
  }
  m_wb_tele_rx->reset_stream_stats();
}

bool WBLink::check_in_state_support_changing_settings(){
  return check_work_queue_empty();
}

openhd::WifiSpace WBLink::get_current_frequency_channel_space()const {
  return openhd::get_space_from_frequency(m_settings->get_settings().wb_frequency);
}

bool WBLink::set_tx_power_mw(int tx_power_mw) {
  m_console->debug("set_tx_power_mw {}mW", tx_power_mw);
  if(!openhd::is_valid_tx_power_milli_watt(tx_power_mw)){
    m_console->warn("Invalid tx power:{}mW", tx_power_mw);
    return false;
  }
  if(!check_in_state_support_changing_settings())return false;
  m_settings->unsafe_get_settings().wb_tx_power_milli_watt= tx_power_mw;
  m_settings->persist();
  apply_txpower();
  return true;
}

bool WBLink::set_tx_power_rtl8812au(int tx_power_index_override){
  m_console->debug("set_tx_power_rtl8812au {}index",tx_power_index_override);
  if(!openhd::validate_wb_rtl8812au_tx_pwr_idx_override(tx_power_index_override))return false;
  if(!check_in_state_support_changing_settings())return false;
  m_settings->unsafe_get_settings().wb_rtl8812au_tx_pwr_idx_override=tx_power_index_override;
  m_settings->persist();
  apply_txpower();
  return true;
}

int64_t WBLink::get_total_dropped_packets() {
  int64_t total=0;
  for(const auto& tx:m_wb_video_tx_list){
    auto stats=tx->get_latest_stats();
    total+=stats.n_dropped_packets;
  }
  total+=m_wb_tele_tx->get_latest_stats().n_dropped_packets;
  return total;
}

WBLink::ScanResult WBLink::scan_channels(const openhd::ActionHandler::ScanChannelsParam& params){
  const WiFiCard& card=m_broadcast_cards.at(0);
  std::vector<openhd::WifiChannel> channels_to_scan;
  if(params.check_2g_channels_if_card_support && card.supports_2GHz()){
    auto tmp=openhd::get_channels_2G();
    OHDUtil::vec_append(channels_to_scan,tmp);
  }
  if(params.check_5g_channels_if_card_supports && card.supports_5GHz()){
    auto tmp=openhd::get_channels_5G();
    OHDUtil::vec_append(channels_to_scan,tmp);
  }
  if(channels_to_scan.empty()){
    m_console->warn("No channels to scan, return early");
    return {};
  }
  std::vector<uint32_t> channel_widths_to_scan;
  if(params.check_20Mhz_channel_width_if_card_supports){
    channel_widths_to_scan.push_back(20);
  }
  if(params.check_40Mhz_channel_width_if_card_supports){
    channel_widths_to_scan.push_back(40);
  }
  if(channel_widths_to_scan.empty()){
    m_console->warn("No channel_widths to scan, return early");
    return {};
  }
  // Issue / bug with RTL8812AU: Apparently the adapter sometimes receives data from a frequency that is not correct
  // (e.g. when the air is set to 5700 and the rx listens on frequency  5540 ) but with an incredibly high packet loss.
  // therefore, instead of returning early, we hop through all frequencies and on frequencies where we get data, store
  // the packet loss. In the end, we then decide what frequency is most likely the one the air is sending on.
  struct TmpResult{
    openhd::WifiChannel channel;
    uint32_t channel_width;
    int packet_loss_perc=0;
  };
  std::vector<TmpResult> possible_frequencies{};
  // Note: We intentionally do not modify the persistent settings here
  m_console->debug("Channel scan N channels to scan:{} N channel widths to scan:{}",
                   channels_to_scan.size(),channel_widths_to_scan.size());
  bool done_early=false;
  // We need to loop through all possible channels
  for(int i=0;i<channels_to_scan.size();i++){
    const auto& channel=channels_to_scan[i];
    if(done_early)break;
    // and all possible channel widths (20 or 40Mhz only right now)
    for(const auto& channel_width:channel_widths_to_scan){
      // Return early in some cases (e.g. when we have a low loss and are quite certain about a frequency)
      if(done_early)break;
      // Skip channels / frequencies the card doesn't support anyways
      if(!openhd::wb::any_card_support_frequency(channel.frequency,m_broadcast_cards,m_platform, m_console)){
        continue;
      }
      // set new frequency, reset the packet count, sleep, then check if any openhd packets have been received
      const bool freq_success=apply_frequency_and_channel_width(channel.frequency,channel_width);
      if(!freq_success){
          m_console->warn("Cannot scan [{}] {}Mhz@{}Mhz",channel.channel,channel.frequency,channel_width);
          continue;
      }
      if(m_opt_action_handler){
            openhd::ActionHandler::ScanChannelsProgress tmp{};
            tmp.channel_mhz=(int)channel.frequency;
            tmp.channel_width_mhz=channel_width;
            tmp.success= false;
            tmp.progress=OHDUtil::calculate_progress_perc(i+1,channels_to_scan.size());
            m_opt_action_handler->add_scan_channels_progress(tmp);
      }
      // sleeep a bit - some cards /drivers might need time switching
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      m_console->debug("Scanning [{}] {}Mhz@{}Mhz",channel.channel,channel.frequency,channel_width);
      reset_all_rx_stats();
      std::this_thread::sleep_for(std::chrono::seconds(2));
      const auto n_likely_openhd_packets=m_wb_txrx->get_rx_stats().curr_n_likely_openhd_packets;
      // If we got what looks to be openhd packets, sleep a bit more such that we can reliably get a session and packet loss
      if(n_likely_openhd_packets>0){
        m_console->debug("Got {} likely openhd packets, sleep a bit more",n_likely_openhd_packets);
        std::this_thread::sleep_for(std::chrono::seconds(2));
      }
      const auto packet_loss=m_wb_txrx->get_rx_stats().curr_lowest_packet_loss;
      const auto n_valid_packets=m_wb_txrx->get_rx_stats().count_p_valid;
      // We might receive 20Mhz channel width packets from a air unit sending on 20Mhz channel width while
      // receiving on 40Mhz channel width - if we were to then to set the gnd to 40Mhz, we will be able to receive data,
      // but not be able to send any data up to the air unit.
      const int rx_chann_width=m_wb_txrx->get_rx_stats().last_received_packet_channel_width;
      m_console->debug("Got {} packets on {}@{} rx_chan_width:{} with loss {}",n_valid_packets,channel.frequency,channel_width,
                       rx_chann_width,packet_loss);
      if(n_valid_packets>0 && (rx_chann_width==channel_width || rx_chann_width==-1)){
        TmpResult tmp_result{channel,channel_width,packet_loss};
        possible_frequencies.push_back(tmp_result);
        if(packet_loss>=0 && packet_loss<20){
          // if the packet loss is low, we can safely return early
          m_console->debug("Got <10% packet loss, return early");
          done_early= true;
        }else{
          m_console->warn("Got >10% packet loss,continue and select most likely at the end");
        }
      }
    }
  }
  ScanResult result{};
  if(possible_frequencies.empty()){
    m_console->warn("Channel scan failure, restore local settings");
    apply_frequency_and_channel_width_from_settings();
    result.success= false;
    result.frequency=0;
  }else{
    m_console->debug("Channel scan success, possible frequencies {}",possible_frequencies.size());
    auto best=possible_frequencies.at(0);
    for(int i=1;i<possible_frequencies.size();i++){
      const auto other=possible_frequencies.at(i);
      if(other.packet_loss_perc<best.packet_loss_perc){
        best=other;
      }
    }
    m_console->warn("Selected {}@{} with loss:{}% as most likely",best.channel.frequency,best.channel_width,best.packet_loss_perc);
    result.success= true;
    result.frequency=best.channel.frequency;
    result.channel_width=best.channel_width;
    m_settings->unsafe_get_settings().wb_frequency=result.frequency;
    m_settings->unsafe_get_settings().wb_channel_width=result.channel_width;
    m_settings->persist();
    // Not needed anymore
    //openhd::reboot::dirty_terminate_openhd_and_let_service_restart();
    apply_frequency_and_channel_width_from_settings();
  }
  if(m_opt_action_handler){
      openhd::ActionHandler::ScanChannelsProgress tmp{};
      tmp.channel_mhz=(int)result.frequency;
      tmp.channel_width_mhz=result.channel_width;
      tmp.success= result.success;
      tmp.progress=100;
      m_opt_action_handler->add_scan_channels_progress(tmp);
  }
  return result;
}

bool WBLink::async_scan_channels(openhd::ActionHandler::ScanChannelsParam scan_channels_params) {
    if(!check_work_queue_empty()){
        m_console->warn("Rejecting async_scan_channels, work queue busy");
        return false;
    }
    auto work_item=std::make_shared<WorkItem>("SCAN_CHANNELS",[this,scan_channels_params](){
        scan_channels(scan_channels_params);
    },std::chrono::steady_clock::now());
    return try_schedule_work_item(work_item);
}

struct AnalyzeResult{
    int frequency;
    int n_foreign_packets;
};

void WBLink::analyze_channels() {
    const WiFiCard& card=m_broadcast_cards.at(0);
    std::vector<openhd::WifiChannel> channels_to_analyze;
    const auto supported_freq_5G=card.supported_frequencies_5G;
    const auto supported_freq_2G=card.supported_frequencies_2G;
    for(const auto& freq:supported_freq_2G){
        auto tmp=openhd::channel_from_frequency(freq);
        if(tmp.has_value()){
            channels_to_analyze.push_back(tmp.value());
        }
    }
    for(const auto freq:supported_freq_5G){
        auto tmp=openhd::channel_from_frequency(freq);
        if(tmp.has_value()){
            channels_to_analyze.push_back(tmp.value());
        }
    }
    std::vector<AnalyzeResult> results{};
    for(int i=0; i < channels_to_analyze.size(); i++){
        const auto channel=channels_to_analyze[i];
        // We use 40Mhz when possible, 20Mhz otherwise
        int channel_width=40;
        if(!channel.is_legal_any_country_40Mhz){
            channel_width=20;
        }
        // set new frequency, reset the packet count, sleep, then check if any openhd packets have been received
        apply_frequency_and_channel_width(channel.frequency,channel_width);
        m_console->debug("Analyzing [{}] {}Mhz@{}Mhz",channel.channel,channel.frequency,channel_width);
        reset_all_rx_stats();
        std::this_thread::sleep_for(std::chrono::seconds(4));
        const auto stats=m_wb_txrx->get_rx_stats();
        const auto n_foreign_packets=stats.count_p_any-stats.count_p_valid;
        m_console->debug("Got {} foreign packets {}:{}",n_foreign_packets,stats.count_p_any,stats.count_p_valid);
        results.push_back(AnalyzeResult{(int)channel.frequency,(int)n_foreign_packets});
        if(m_opt_action_handler){
            openhd::ActionHandler::AnalyzeChannelsResult tmp{};
            tmp.channel_mhz=(int)channel.frequency;
            tmp.channel_width_mhz=channel_width;
            tmp.n_foreign_packets=(int)n_foreign_packets;
            tmp.progress=OHDUtil::calculate_progress_perc(i+1, channels_to_analyze.size());
            m_opt_action_handler->add_analyze_result(tmp);
        }
    }
    std::stringstream ss;
    for(int i=0;i<results.size();i++){
        ss<<results[i].frequency<<"@"<<results[i].n_foreign_packets<<"\n";
    }
    m_console->debug("{}",ss.str().c_str());
    // Go back to the previous frequency
    apply_frequency_and_channel_width_from_settings();
}

bool WBLink::async_analyze_channels() {
    if(!check_work_queue_empty()){
        m_console->warn("Rejecting async_scan_channels, work queue busy");
        return false;
    }
    auto work_item=std::make_shared<WorkItem>("ANALYZE_CHANNELS",[this](){
        analyze_channels();
    },std::chrono::steady_clock::now());
    return try_schedule_work_item(work_item);
}

void WBLink::set_mcs_index_from_rc_channel(const std::array<int, 18>& rc_channels) {
  const auto& settings=m_settings->get_settings();
  if(settings.wb_mcs_index_via_rc_channel==openhd::WB_MCS_INDEX_VIA_RC_CHANNEL_OFF){
    // disabled
    return ;
  }
  // 1= channel number 1 = array index 0
  const int channel_index=(int)settings.wb_mcs_index_via_rc_channel-1;
  // check if we are in bounds of array (better be safe than sorry, in case user manually messes up a number)
  if(!(channel_index>=0 && channel_index<rc_channels.size())){
    m_console->debug("Invalid channel index {}",channel_index);
    return ;
  }
  const auto mcs_channel_value_pwm=rc_channels[channel_index];
  // UINT16_MAX means ignore channel
  if(mcs_channel_value_pwm==UINT16_MAX){
    m_console->debug("Disabled channel {}: {}",channel_index,mcs_channel_value_pwm);
    return ;
  }
  // mavlink says pwm in [1000, 2000] range - but from my testing with frsky for example, it is quite common for a
  // switch (for example) to be at for example [988 - 2012] us
  // which is why we accept a [900 ... 2100] range here
  if(mcs_channel_value_pwm<900 || mcs_channel_value_pwm>2100){
    m_console->debug("Invalid channel data on channel {}: {}",channel_index,mcs_channel_value_pwm);
    // most likely invalid data, discard
    return ;
  }
  // We simply pre-define a range (pwm: [900,...,2100]
  // [900 ... 1200] : MCS0
  // [1200 ... 1400] : MCS1
  // [1400 ... 1600] : MCS2
  // [1600 ... 1800] : MCS3
  // [1800 ... 2100] : MCS 4
  int mcs_index=0;
  if(mcs_channel_value_pwm>1800){
    mcs_index=4;
  }else if(mcs_channel_value_pwm>1600){
    mcs_index=3;
  }else if(mcs_channel_value_pwm>1400){
    mcs_index=2;
  }else if(mcs_channel_value_pwm>1200){
    mcs_index=1;
  }
  // check if we are already using the wanted mcs index
  if(settings.wb_mcs_index==mcs_index){
    return ;
  }
  // apply the wanted mcs index
  set_mcs_index(mcs_index);
}

void WBLink::update_arming_state(bool armed) {
  m_console->debug("update arming state, armed: {}",armed);
  // We just update the internal armed / disarmed state and then call apply_tx_power -
  // it will set the right tx power if the user enabled it
  m_is_armed=armed;
  apply_txpower();
}
