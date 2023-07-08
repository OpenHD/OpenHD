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
#include "wb_link_helper.h"
#include "wifi_card.h"

// optionally, if no file exists we just use a default, hard coded seed
static std::string get_opt_keypair_filename(bool is_air){
  std::string filename=openhd::get_interface_settings_directory();
  filename+=is_air ? "drone.key" : "gs.key";
  return filename;
}

WBLink::WBLink(OHDProfile profile,OHDPlatform platform,std::vector<WiFiCard> broadcast_cards,std::shared_ptr<openhd::ActionHandler> opt_action_handler)
    : m_profile(std::move(profile)),
      m_platform(platform),
      m_broadcast_cards(std::move(broadcast_cards)),
      m_disable_all_frequency_checks(openhd::wb::disable_all_frequency_checks()),
      m_opt_action_handler(std::move(opt_action_handler))
{
  m_console = openhd::log::create_or_get("wb_streams");
  assert(m_console);
  m_console->info("Broadcast cards:{}",debug_cards(m_broadcast_cards));
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
  openhd::wb::fixup_unsupported_settings(*m_settings,m_broadcast_cards,m_console);
  // We default to the right setting (clean install) but only print a warning, don't actively fix it.
  if(m_profile.is_ground()){
	if(all_cards_support_setting_mcs_index(m_broadcast_cards) &&
	m_settings->unsafe_get_settings().wb_mcs_index!=openhd::DEFAULT_GND_UPLINK_MCS_INDEX){
	  // We always use a MCS index of X for the uplink, since (compared to the down / video link) it requires a negligible amount of bandwidth
	  // and for those using RC over OpenHD, we have the benefit that the range of RC is "more" than the range for video
	  m_console->warn("GND recommended MCS{}",openhd::DEFAULT_GND_UPLINK_MCS_INDEX);
	  //m_settings->unsafe_get_settings().wb_mcs_index=openhd::DEFAULT_GND_UPLINK_MCS_INDEX;
	  //m_settings->persist();
	}
  }
  takeover_cards_monitor_mode();
  WBTxRx::Options txrx_options{};
  txrx_options.rtl8812au_rssi_fixup= true;
  const auto keypair_file= get_opt_keypair_filename(m_profile.is_air);
  if(OHDFilesystemUtil::exists(keypair_file)){
      txrx_options.encryption_key = std::string(keypair_file);
      m_console->debug("Using key from file {}",txrx_options.encryption_key->c_str());
  }else{
      txrx_options.encryption_key = std::nullopt;
  }
  //txrx_options.log_all_received_packets= true;
  //txrx_options.log_all_received_validated_packets= true;
  //txrx_options.advanced_latency_debugging_rx=true;
  const auto card_names = openhd::wb::get_card_names(m_broadcast_cards);
  assert(!card_names.empty());
  m_wb_txrx=std::make_shared<WBTxRx>(card_names,txrx_options);
  m_wb_txrx->tx_threadsafe_update_radiotap_header(create_radiotap_params());
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
  }
  {
      // Video is unidirectional, aka always goes from air pi to ground pi
      if (m_profile.is_air) {
          // we transmit video
          auto primary = create_wb_tx(openhd::VIDEO_PRIMARY_RADIO_PORT, true);
          auto secondary = create_wb_tx(openhd::VIDEO_SECONDARY_RADIO_PORT, true);
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
  m_wb_txrx->start_receiving();
  set_freq_width_power();
  m_work_thread_run = true;
  m_work_thread =std::make_unique<std::thread>(&WBLink::loop_do_work, this);
  if(m_opt_action_handler){
        auto cb_scan=[this](openhd::ActionHandler::ScanChannelsParam param){
          async_scan_channels(param);
        };
        m_opt_action_handler->action_wb_link_scan_channels_register(cb_scan);
        auto cb_mcs=[this](const std::array<int,18>& rc_channels){
          set_mcs_index_from_rc_channel(rc_channels);
        };
        m_opt_action_handler->action_on_ony_rc_channel_register(cb_mcs);
        auto cb_arm=[this](bool armed){
          update_arming_state(armed);
        };
        m_opt_action_handler->m_action_tx_power_when_armed=std::make_shared<openhd::ActionHandler::ACTION_TX_POWER_WHEN_ARMED>(cb_arm);
  }
}

WBLink::~WBLink() {
  m_console->debug("WBLink::~WBLink() begin");
  if(m_opt_action_handler){
    m_opt_action_handler->action_wb_link_scan_channels_register(nullptr);
    m_opt_action_handler->action_on_ony_rc_channel_register(nullptr);
    m_opt_action_handler->m_action_tx_power_when_armed= nullptr;
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
    // TODO
    //wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, true);
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

void WBLink::set_freq_width_power() {
  m_console->debug("set_freq_width_power() begin");
  apply_frequency_and_channel_width_from_settings();
  apply_txpower();
  m_console->debug("set_freq_width_power() end");
}

RadiotapHeader::UserSelectableParams WBLink::create_radiotap_params()const {
  const auto settings=m_settings->get_settings();
  const auto mcs_index=static_cast<int>(settings.wb_mcs_index);
  const auto channel_width=static_cast<int>(settings.wb_channel_width);
  return RadiotapHeader::UserSelectableParams{
      channel_width, settings.wb_enable_short_guard,settings.wb_enable_stbc,
      settings.wb_enable_ldpc, mcs_index};
}


std::unique_ptr<WBStreamTx> WBLink::create_wb_tx(uint8_t radio_port,bool is_video) {
  WBStreamTx::Options options{};
  options.enable_fec=is_video;
  options.radio_port=radio_port;
  if(is_video){
    options.block_data_queue_size=2;
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

std::string WBLink::createDebug()const{
  std::stringstream ss;
  /*if (m_wb_tele_rx) {
    ss<<"TeleRx: "<< m_wb_tele_rx->createDebugState();
  }
  if (m_wb_tele_tx) {
    ss<<"TeleTx: "<< m_wb_tele_tx->createDebugState();
  }
  for (const auto &txvid: m_wb_video_tx_list) {
    ss<<"VidTx: "<<txvid->createDebugState();
  }
  for (const auto &rxvid: m_wb_video_rx_list) {
    ss<<"VidRx :"<<rxvid->createDebugState();
  }*/
  return ss.str();
}

bool WBLink::request_set_frequency(int frequency) {
  m_console->debug("request_set_frequency {}",frequency);
  if(m_disable_all_frequency_checks){
    m_console->warn("Not sanity checking frequency");
  }else{
    if(!openhd::wb::cards_support_frequency(frequency,m_broadcast_cards,m_platform,m_console)){
      return false;
    }
  }
  if(!check_in_state_support_changing_settings())return false;
  m_settings->unsafe_get_settings().wb_frequency=frequency;
  m_settings->persist();
  // We need to delay the change to make sure the mavlink ack has enough time to make it to the ground
  auto work_item=std::make_shared<WorkItem>([this](){
    apply_frequency_and_channel_width_from_settings();
  },std::chrono::steady_clock::now()+ DELAY_FOR_TRANSMIT_ACK);
  schedule_work_item(work_item);
  // And add a work item that runs after 5 seconds and resets the frequency to the previous one if no data is being received
  // after X seconds
  const auto rx_count_p_decryption_ok=m_wb_txrx->get_rx_stats().count_p_valid;
  if(rx_count_p_decryption_ok>100){
    m_console->debug("Adding reset to previous known frequency work item {}",rx_count_p_decryption_ok);
    auto backup_work_item=std::make_shared<WorkItem>([this](){
      m_console->debug("check if data is being received {}",m_wb_txrx->get_rx_stats().count_p_valid);
    },std::chrono::steady_clock::now()+ DELAY_FOR_TRANSMIT_ACK+std::chrono::seconds(2));
    schedule_work_item(backup_work_item);
  }
  return true;
}

bool WBLink::apply_frequency_and_channel_width(uint32_t frequency, uint32_t channel_width) {
  const auto res=openhd::wb::set_frequency_and_channel_width_for_all_cards(frequency,channel_width,m_broadcast_cards);
  m_wb_txrx->tx_update_channel_width(channel_width);
  return res;
}

bool WBLink::apply_frequency_and_channel_width_from_settings() {
  const auto settings=m_settings->get_settings();
  return apply_frequency_and_channel_width(settings.wb_frequency,settings.wb_channel_width);
}

void WBLink::apply_txpower() {
  const auto settings=m_settings->get_settings();
  const auto before=std::chrono::steady_clock::now();
  for(const auto& card: m_broadcast_cards){
    if(card.type==WiFiCardType::Realtek8812au){
      // requires corresponding driver workaround for dynamic tx power
      uint32_t pwr_index=(int)settings.wb_rtl8812au_tx_pwr_idx_override;
      if(m_is_armed && settings.wb_rtl8812au_tx_pwr_idx_armed!=openhd::RTL8812AU_TX_POWER_INDEX_ARMED_DISABLED){
        m_console->debug("Using power index special for armed");
        pwr_index=settings.wb_rtl8812au_tx_pwr_idx_armed;
      }
      m_console->debug("RTL8812AU tx_pwr_idx_override: {}",pwr_index);
      wifi::commandhelper::iw_set_tx_power(card.device_name,pwr_index);
    }else{
      const auto tmp=openhd::milli_watt_to_mBm(settings.wb_tx_power_milli_watt);
      wifi::commandhelper::iw_set_tx_power(card.device_name,tmp);
    }
  }
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

bool WBLink::request_set_channel_width(int channel_width) {
  m_console->debug("request_set_channel_width {}",channel_width);
  if(!openhd::is_valid_channel_width(channel_width)){
    m_console->warn("Invalid channel width {}",channel_width);
    return false;
  }
  if(!openhd::wb::cards_support_setting_channel_width(m_broadcast_cards)){
    m_console->warn("Cannot change channel width, at least one card doesn't support it");
    return false;
  }
  if(!check_in_state_support_changing_settings())return false;
  m_settings->unsafe_get_settings().wb_channel_width=channel_width;
  m_settings->persist();
  // We need to delay the change to make sure the mavlink ack has enough time to make it to the ground
  auto work_item=std::make_shared<WorkItem>([this](){
    apply_frequency_and_channel_width_from_settings();
  },std::chrono::steady_clock::now()+ DELAY_FOR_TRANSMIT_ACK);
  schedule_work_item(work_item);
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
  auto change_wb_channel_width=openhd::IntSetting{(int)settings.wb_channel_width,[this](std::string,int value){
                                                      return request_set_channel_width(value);
                                                    }};
  auto change_wb_mcs_index=openhd::IntSetting{(int)settings.wb_mcs_index,[this](std::string,int value){
                                                  return set_mcs_index(value);
                                                }};
  ret.push_back(Setting{WB_FREQUENCY,change_freq});
  ret.push_back(Setting{WB_CHANNEL_WIDTH,change_wb_channel_width});
  ret.push_back(Setting{WB_MCS_INDEX,change_wb_mcs_index});
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
  }
  // These 3 are only supported / known to work on rtl8812au (yet), therefore only expose them when rtl8812au is used
  if(openhd::wb::has_any_rtl8812au(m_broadcast_cards)){
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
      m_settings->unsafe_get_settings().wb_rtl8812au_tx_pwr_idx_armed=value;
      m_settings->persist();
      return true;
    };
    ret.push_back(openhd::Setting{WB_RTL8812AU_TX_PWR_IDX_ARMED,openhd::IntSetting{(int)settings.wb_rtl8812au_tx_pwr_idx_armed,cb_wb_rtl8812au_tx_pwr_idx_armed}});
  }else{
    auto cb_wb_tx_power_milli_watt=[this](std::string,int value){
      return set_tx_power_mw(value);
    };
    auto change_tx_power=openhd::IntSetting{(int)settings.wb_tx_power_milli_watt,cb_wb_tx_power_milli_watt};
    ret.push_back(Setting{WB_TX_POWER_MILLI_WATT,change_tx_power});
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
          front->execute();
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
    stats.telemetry.curr_tx_bps=curr_tx_stats.current_provided_bits_per_second;
    stats.telemetry.curr_tx_pps=curr_tx_stats.current_injected_packets_per_second;
  }
  if(m_wb_tele_rx){
    const auto curr_rx_stats= m_wb_tele_rx->get_latest_stats();
    stats.telemetry.curr_rx_bps=curr_rx_stats.curr_in_bits_per_second;
  }
  if(m_profile.is_air){
    // video on air
    for(int i=0;i< m_wb_video_tx_list.size();i++){
      auto& wb_tx= *m_wb_video_tx_list.at(i);
      //auto& air_video=i==0 ? stats.air_video0 : stats.air_video1;
      const auto curr_tx_stats=wb_tx.get_latest_stats();
      openhd::link_statistics::StatsWBVideoAir air_video{};
      if(m_opt_action_handler){
        const int tmp= m_opt_action_handler->dirty_get_bitrate_of_camera(i);
        air_video.curr_recommended_bitrate=tmp>0 ? tmp : 0;
      }
      air_video.link_index=i;
      air_video.curr_measured_encoder_bitrate=curr_tx_stats.current_provided_bits_per_second;
      air_video.curr_injected_bitrate=curr_tx_stats.current_injected_bits_per_second;
      air_video.curr_injected_pps=curr_tx_stats.current_injected_packets_per_second;
      air_video.curr_dropped_packets=curr_tx_stats.n_dropped_packets;
      const auto curr_tx_fec_stats=wb_tx.get_latest_fec_stats();
      air_video.curr_fec_encode_time_avg_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.avg);
      air_video.curr_fec_encode_time_min_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.min);
      air_video.curr_fec_encode_time_max_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.max);
      air_video.curr_fec_block_size_min=curr_tx_fec_stats.curr_fec_block_length.min;
      air_video.curr_fec_block_size_max=curr_tx_fec_stats.curr_fec_block_length.max;
      air_video.curr_fec_block_size_avg=curr_tx_fec_stats.curr_fec_block_length.avg;
      air_video.curr_fec_percentage=m_settings->unsafe_get_settings().wb_video_fec_percentage;
      air_video.curr_keyframe_interval=m_opt_action_handler ? m_opt_action_handler->curr_cam1_cam2_keyframe_interval.load() : -1;
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
  auto rxStats=m_wb_txrx->get_rx_stats();
  auto txStats=m_wb_txrx->get_tx_stats();
  stats.monitor_mode_link.curr_rx_packet_loss_perc=rxStats.curr_packet_loss;
  stats.monitor_mode_link.count_tx_inj_error_hint=txStats.count_tx_injections_error_hint;
  stats.monitor_mode_link.count_tx_dropped_packets=get_total_dropped_packets();
  stats.monitor_mode_link.curr_tx_card_idx=m_wb_txrx->get_curr_active_tx_card_idx();
  stats.monitor_mode_link.curr_tx_mcs_index=m_settings->unsafe_get_settings().wb_mcs_index;

  // dBm is per card, not per stream
  assert(stats.cards.size()>=4);
  // only populate actually used cards
  assert(m_broadcast_cards.size()<=stats.cards.size());
  for(int i=0;i< m_broadcast_cards.size();i++){
    auto& card = stats.cards.at(i);
    auto rxStatsCard=m_wb_txrx->get_rx_stats_for_card(i);
    card.rx_rssi=rxStatsCard.rssi_for_wifi_card.last_rssi;
    card.count_p_received=rxStatsCard.count_p_valid;
    card.count_p_injected=rxStatsCard.curr_packet_loss;
    card.exists_in_openhd= true;
  }
  stats.is_air=m_profile.is_air;
  if(m_opt_action_handler){
    m_opt_action_handler->action_wb_link_statistcs_handle(stats);
  }
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
  m_last_rate_adjustment=std::chrono::steady_clock::now();
  // First we calculate the theoretical rate for the current "wifi config" aka taking mcs index, channel width, ... into account
  const auto settings = m_settings->get_settings();
  const auto wifi_space=openhd::get_space_from_frequency(settings.wb_frequency);
  const auto max_rate_for_current_wifi_config_without_adjust =
      openhd::wb::get_max_rate_possible(m_broadcast_cards.at(0),wifi_space,
                                           settings.wb_mcs_index,
                                           settings.wb_channel_width == 40);
  const auto max_rate_for_current_wifi_config=max_rate_for_current_wifi_config_without_adjust * m_settings->get_settings().wb_video_rate_for_mcs_adjustment_percent/100;
  m_console->debug("Max rate {} with {} perc {}",kbits_per_second_to_string(max_rate_for_current_wifi_config_without_adjust),
                   m_settings->get_settings().wb_video_rate_for_mcs_adjustment_percent,max_rate_for_current_wifi_config);
  const auto max_video_rate_for_current_wifi_config =
      openhd::wb::deduce_fec_overhead(max_rate_for_current_wifi_config,settings.wb_video_fec_percentage);
  if(m_max_video_rate_for_current_wifi_config !=max_video_rate_for_current_wifi_config){
    // Apply the default for this configuration, then return - we will start the auto-adjustment
    // depending on tx error(s) next time the rate adjustment is called
    m_console->debug("MCS:{} ch_width:{} Calculated max_rate:{}, max_video_rate:{}",
                     settings.wb_mcs_index,settings.wb_channel_width,
                     kbits_per_second_to_string(max_rate_for_current_wifi_config),
                     kbits_per_second_to_string(max_video_rate_for_current_wifi_config));
    m_max_video_rate_for_current_wifi_config =
        max_video_rate_for_current_wifi_config;
    m_recommended_video_bitrate= m_max_video_rate_for_current_wifi_config;
    m_n_detected_and_reset_tx_errors=0;
    m_last_total_tx_error_count=0;
    if (m_opt_action_handler) {
      openhd::ActionHandler::LinkBitrateInformation lb{};
      lb.recommended_encoder_bitrate_kbits = m_recommended_video_bitrate;
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
        m_max_video_rate_for_current_wifi_config,m_recommended_video_bitrate);
    m_n_detected_and_reset_tx_errors=0;
    // Reduce video bitrate by 1MBit/s
    m_recommended_video_bitrate-=1000;
    // Safety, in case we fall below a certain threshold the encoder won't be able to produce an image at some point anyways.
    static constexpr auto MIN_BITRATE=1000*2;
    if(m_recommended_video_bitrate<MIN_BITRATE){
      m_recommended_video_bitrate=MIN_BITRATE;
    }
    m_console->warn("TX errors, reducing video bitrate to {}",m_recommended_video_bitrate);
  }
  // Since settings might change dynamically at run time, we constantly recommend a bitrate to the encoder / camera -
  // The camera is responsible for "not doing anything" when we recommend the same bitrate to it multiple times
  if (m_opt_action_handler) {
    openhd::ActionHandler::LinkBitrateInformation lb{};
    lb.recommended_encoder_bitrate_kbits = m_recommended_video_bitrate;
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
  if(value<=5 || value>=500)return false;
  m_settings->unsafe_get_settings().wb_video_rate_for_mcs_adjustment_percent=value;
  m_settings->persist();
  return true;
}
void WBLink::schedule_work_item(const std::shared_ptr<WorkItem>& work_item) {
  std::lock_guard<std::mutex> guard(m_work_item_queue_mutex);
  m_work_item_queue.push(work_item);
}

bool WBLink::check_work_queue_empty() {
  std::lock_guard<std::mutex> guard(m_work_item_queue_mutex);
  if(!m_work_item_queue.empty()){
    m_console->info("Rejecting param, another change is still queued up");
    return false;
  }
  return true;
}


void WBLink::transmit_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data) {
  const auto res=m_wb_tele_tx->try_enqueue_packet(data);
  if(!res)m_console->debug("Enqueing tele packet failed");
  if(!m_broadcast_cards.at(0).supports_injection){
    const auto now=std::chrono::steady_clock::now();
    const auto elapsed=now-m_last_log_card_does_might_not_inject;
    if(elapsed>WARN_CARD_DOES_NOT_INJECT_INTERVAL){
      m_console->warn("Card {} (might) not support injection/TX",m_broadcast_cards.at(0).device_name);
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
    tx.try_enqueue_block(fragmented_video_frame.frame_fragments, max_block_size_for_platform,fec_perc);
  }else{
    m_console->debug("Invalid camera stream_index {}",stream_index);
  }
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
  is_scanning=true;
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
  m_console->debug("Channel scan, time per channel:{}ms N channels to scan:{} N channel widths to scan:{}",
                   std::chrono::duration_cast<std::chrono::milliseconds>(DEFAULT_SCAN_TIME_PER_CHANNEL).count(),
                   channels_to_scan.size(),channel_widths_to_scan.size());
  bool done_early=false;
  // We need to loop through all possible channels
  for(const auto& channel:channels_to_scan){
    if(done_early)break;
    // and all possible channel widths (20 or 40Mhz only right now)
    for(const auto& channel_width:channel_widths_to_scan){
      // Return early in some cases (e.g. when we have a low loss and are quite certain about a frequency)
      if(done_early)break;
      // Skip channels / frequencies the card doesn't support anyways
      if(!openhd::wb::cards_support_frequency(channel.frequency,m_broadcast_cards,m_platform, m_console)){
        continue;
      }
      // set new frequency, reset the packet count, sleep, then check if any openhd packets have been received
      apply_frequency_and_channel_width(channel.frequency,channel_width);
      m_console->warn("Scanning [{}] {}Mhz@{}Mhz",channel.channel,channel.frequency,channel_width);
      reset_all_rx_stats();
      std::this_thread::sleep_for(DEFAULT_SCAN_TIME_PER_CHANNEL);
      const int n_packets= m_wb_txrx->get_rx_stats().count_p_valid;
      // We might receive 20Mhz channel width packets from a air unit sending on 20Mhz channel width while
      // receiving on 40Mhz channel width - if we were to then to set the gnd to 40Mhz, we will be able to receive data,
      // but not be able to send any data up to the air unit.
      const int rx_chann_width=get_last_rx_packet_chan_width();
      m_console->debug("Got {} packets on {}@{}",n_packets,channel.frequency,rx_chann_width);
      if(n_packets>0){
        // We got packets on this frequency, but it is not guaranteed those packets are from an openhd air unit.
        // sleep a bit more, then check if we actually got any decrypted packets
        std::this_thread::sleep_for(std::chrono::seconds(2));
        const int n_packets_decrypted= m_wb_txrx->get_rx_stats().count_p_valid;
        const int packet_loss=m_wb_txrx->get_rx_stats().curr_packet_loss;
        m_console->debug("Got {} decrypted packets on frequency {} with {} packet loss",n_packets_decrypted,channel.frequency,packet_loss);
        if(n_packets_decrypted>0 && rx_chann_width==channel_width){
          TmpResult tmp_result{channel,channel_width,packet_loss};
          possible_frequencies.push_back(tmp_result);
          if(packet_loss<10){
            // if the packet loss is low, we can safely return early
            m_console->debug("Got <10% packet loss, return early");
            done_early= true;
          }else{
            m_console->warn("Got >10% packet loss,continue and select most likely at the end");
          }
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
    openhd::reboot::dirty_terminate_openhd_and_let_service_restart();
    apply_frequency_and_channel_width_from_settings();
  }
  is_scanning=false;
  return result;
}

void WBLink::async_scan_channels(openhd::ActionHandler::ScanChannelsParam scan_channels_params) {
  if(!check_work_queue_empty()){
    m_console->warn("Rejecting async_scan_channels, work queue busy");
    return;
  }
  auto work_item=std::make_shared<WorkItem>([this,scan_channels_params](){
    scan_channels(scan_channels_params);
  },std::chrono::steady_clock::now());
  schedule_work_item(work_item);
}

void WBLink::reset_all_rx_stats() {
  m_wb_txrx->rx_reset_stats();
  for(auto& rx:m_wb_video_rx_list){
    rx->reset_stream_stats();
  }
  m_wb_tele_rx->reset_stream_stats();
}

int WBLink::get_last_rx_packet_chan_width() {
  return m_wb_txrx->get_rx_stats().last_received_packet_channel_width;
}

bool WBLink::check_in_state_support_changing_settings(){
  return !is_scanning && check_work_queue_empty();
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
