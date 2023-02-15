#include "wb_link.h"
#include "wifi_command_helper.h"
//#include "wifi_command_helper2.h"

#include <iostream>
#include <utility>

#include "openhd_dirty_fatal_error.hpp"
#include "openhd_global_constants.hpp"
#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util_filesystem.h"
#include "wb_link_helper.h"
#include "wifi_card.hpp"

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
  m_settings =std::make_unique<openhd::WBStreamsSettingsHolder>(platform,m_broadcast_cards);
  // fixup any settings coming from a previous use with a different wifi card (e.g. if user swaps around cards)
  openhd::wb::fixup_unsupported_settings(*m_settings,m_broadcast_cards,m_console);
  takeover_cards_monitor_mode();
  configure_cards();
  configure_telemetry();
  configure_video();
  m_work_thread_run = true;
  m_work_thread =std::make_unique<std::thread>(&WBLink::loop_do_work, this);
  auto cb2=[this](openhd::ActionHandler::ScanChannelsParam param){
    async_scan_channels(param);
  };
  if(m_opt_action_handler){
    m_opt_action_handler->action_wb_link_scan_channels_register(cb2);
  }
  // exp
  /*const auto t_radio_port_rx = m_profile.is_air ? openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT : openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT;
  auto excluded=std::vector<int>{t_radio_port_rx};
  if(m_profile.is_ground()){
    excluded.push_back(openhd::VIDEO_PRIMARY_RADIO_PORT);
    excluded.push_back(openhd::VIDEO_SECONDARY_RADIO_PORT);
  }
  auto excluded2=std::vector<int>{openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT,openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT,
                                    openhd::VIDEO_PRIMARY_RADIO_PORT,openhd::VIDEO_SECONDARY_RADIO_PORT};
  m_foreign_packets_receiver=std::make_unique<ForeignPacketsReceiver>(get_rx_card_names(),excluded2);*/
}

WBLink::~WBLink() {
  if(m_opt_action_handler){
    m_opt_action_handler->action_wb_link_scan_channels_register(nullptr);
  }
  if(m_work_thread){
    m_work_thread_run =false;
    m_work_thread->join();
  }
  // stop all the receiver/transmitter instances, after that, give card back to network manager
  m_wb_tele_rx.reset();
  m_wb_tele_tx.reset();
  m_wb_video_tx_list.resize(0);
  m_wb_video_rx_list.resize(0);
  // give the monitor mode cards back to network manager
  for(const auto& card: m_broadcast_cards){
    wifi::commandhelper::nmcli_set_device_managed_status(card.device_name, true);
  }
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

void WBLink::configure_cards() {
  m_console->debug("configure_cards() begin");
  apply_frequency_and_channel_width_from_settings();
  apply_txpower();
  m_console->debug("configure_cards() end");
}

void WBLink::configure_telemetry() {
  // Setup the tx & rx instances for telemetry. Telemetry is bidirectional,aka
  // tx radio port on air is the same as rx on ground and verse visa
  const auto radio_port_rx = m_profile.is_air ? openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT : openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT;
  const auto radio_port_tx = m_profile.is_air ? openhd::TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT : openhd::TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT;
  auto cb=[this](const uint8_t* data, int data_len){
    auto shared=std::make_shared<std::vector<uint8_t>>(data,data+data_len);
    on_receive_telemetry_data(shared);
  };
  m_wb_tele_rx = create_wb_rx(radio_port_rx, cb);
  m_wb_tele_rx->start_async();
  m_wb_tele_tx = create_wb_tx(radio_port_tx, false);
}

void WBLink::configure_video() {
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
    auto primary = create_wb_rx(openhd::VIDEO_PRIMARY_RADIO_PORT,cb1);
    primary->start_async();
    auto secondary = create_wb_rx(openhd::VIDEO_SECONDARY_RADIO_PORT,cb2);
    secondary->start_async();
    m_wb_video_rx_list.push_back(std::move(primary));
    m_wb_video_rx_list.push_back(std::move(secondary));
  }
}

RadiotapHeader::UserSelectableParams WBLink::create_radiotap_params()const {
  const auto settings=m_settings->get_settings();
  const auto mcs_index=static_cast<int>(settings.wb_mcs_index);
  const auto channel_width=static_cast<int>(settings.wb_channel_width);
  return RadiotapHeader::UserSelectableParams{
      channel_width, settings.wb_enable_short_guard,settings.wb_enable_short_guard,
      settings.wb_enable_ldpc, mcs_index};
}

TOptions WBLink::create_tx_options(uint8_t radio_port,bool is_video)const {
  const auto settings=m_settings->get_settings();
  TOptions options{};
  options.radio_port = radio_port;
  const auto keypair_file= get_opt_keypair_filename(m_profile.is_air);
  if(OHDFilesystemUtil::exists(keypair_file)){
    options.keypair = std::string(keypair_file);
    m_console->debug("Using key from file {}",options.keypair->c_str());
  }else{
    options.keypair = std::nullopt;
  }
  //options.log_time_spent_in_atomic_queue= true;
  if(is_video){
    options.enable_fec= true;
    options.tx_fec_options.fixed_k=static_cast<int>(settings.wb_video_fec_block_length);
    options.tx_fec_options.overhead_percentage=static_cast<int>(settings.wb_video_fec_percentage);
    options.use_block_queue= true;
    // We allow up to 2 queued up frames - note that this doesn't add any latency as long as the bitrate(s) are configured correctly.
    options.block_data_queue_size=2;
  }else{
    options.enable_fec= false;
    options.tx_fec_options.fixed_k=0;
    options.tx_fec_options.overhead_percentage=0;
    options.use_block_queue= false;
    // we do not need a big queue for telemetry data packets
    options.packet_data_queue_size=32;
  }
  options.wlan = m_broadcast_cards.at(0).device_name;
  return options;
}

ROptions WBLink::create_rx_options(uint8_t radio_port)const {
  ROptions options{};
  options.radio_port = radio_port;
  const auto keypair_file= get_opt_keypair_filename(m_profile.is_air);
  if(OHDFilesystemUtil::exists(keypair_file)){
    options.keypair = std::string(keypair_file);
    m_console->debug("Using key from file {}",options.keypair->c_str());
  }else{
    options.keypair = std::nullopt;
  }
  const auto cards = get_rx_card_names();
  assert(!cards.empty());
  options.rxInterfaces = cards;
  // For multi rx-es we need the rx queue - but using it really has negative effects
  // for a single rx card, we can just use a depth of 1 (essentially disabling the rx queue) and eliminate those negative effects
  options.rx_queue_depth = m_broadcast_cards.size() > 1 ? 2 : 1;
  const auto wifi_card_type=m_broadcast_cards.at(0).type;
  options.rtl8812au_rssi_fixup=wifi_card_type==WiFiCardType::Realtek8812au;
  return options;
}

std::unique_ptr<WBTransmitter> WBLink::create_wb_tx(uint8_t radio_port,bool is_video) {
  TOptions options= create_tx_options(radio_port,is_video);
  RadiotapHeader::UserSelectableParams wifiParams= create_radiotap_params();
  return std::make_unique<WBTransmitter>(wifiParams, options);
}

std::unique_ptr<AsyncWBReceiver> WBLink::create_wb_rx(uint8_t radio_port,WBReceiver::OUTPUT_DATA_CALLBACK cb){
  ROptions options= create_rx_options(radio_port);
  return std::make_unique<AsyncWBReceiver>(options,cb);
}

std::string WBLink::createDebug()const{
  std::stringstream ss;
  // we use telemetry data only here
  bool any_data_received=false;
  //if(m_wb_tele_rx && m_wb_tele_rx->anyDataReceived()){
  //  any_data_received=true;
  //}
  ss<<"Any data received: "<<(any_data_received ? "Y":"N")<<"\n";
  if (m_wb_tele_rx) {
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
  }
  return ss.str();
}

std::vector<std::string> WBLink::get_rx_card_names() const {
  std::vector<std::string> ret{};
  for(const auto& card: m_broadcast_cards){
    ret.push_back(card.device_name);
  }
  return ret;
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
  const auto rx_count_p_decryption_ok=get_rx_count_p_decryption_ok();
  if(rx_count_p_decryption_ok>100){
    m_console->debug("Addding reset to previous known frequency work item {}",rx_count_p_decryption_ok);
    auto backup_work_item=std::make_shared<WorkItem>([this](){
      m_console->debug("check if data is being received {}",get_rx_count_p_decryption_ok());
    },std::chrono::steady_clock::now()+ DELAY_FOR_TRANSMIT_ACK+std::chrono::seconds(2));
    schedule_work_item(backup_work_item);
  }
  return true;
}

bool WBLink::apply_frequency_and_channel_width(uint32_t frequency, uint32_t channel_width) {
  const auto res=openhd::wb::set_frequency_and_channel_width_for_all_cards(frequency,channel_width,m_broadcast_cards);
  // TODO: R.n I am not sure if and how you need / even can set it either via radiotap or "iw"
  auto transmitters=get_tx_list();
  for(auto& tx: transmitters){
    tx->update_channel_width(channel_width);
  }
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
      const auto tmp=settings.wb_rtl8812au_tx_pwr_idx_override;
      m_console->debug("RTL8812AU tx_pwr_idx_override: {}",tmp);
      wifi::commandhelper::iw_set_tx_power(card.device_name,tmp);
    }else{
      const auto tmp=openhd::milli_watt_to_mBm(settings.wb_tx_power_milli_watt);
      wifi::commandhelper::iw_set_tx_power(card.device_name,tmp);
      //WifiCardCommandHelper::iwconfig_set_txpower(card,settings.wb_tx_power_milli_watt);
    }
  }
  const auto delta=std::chrono::steady_clock::now()-before;
  m_console->debug("Changing tx power took {}",MyTimeHelper::R(delta));
}

bool WBLink::try_set_mcs_index(int mcs_index) {
  m_console->debug("try_set_mcs_index {}",mcs_index);
  if(!openhd::is_valid_mcs_index(mcs_index)){
    m_console->warn("Invalid mcs index{}",mcs_index);
    return false;
  }
  if(!validate_cards_support_setting_mcs_index()){
    m_console->warn("Cannot change mcs index, it is fixed for at least one of the used cards");
    return false;
  }
  if(!check_in_state_support_changing_settings())return false;
  m_settings->unsafe_get_settings().wb_mcs_index=mcs_index;
  m_settings->persist();
  // R.n the only card known to properly allow setting the MCS index is rtl8812au,
  // and there it is done by modifying the radiotap header
  //for(const auto& wlan:m_broadcast_cards){
  //  wifi::commandhelper::iw_set_rate_mcs(wlan.device_name,settings.wb_mcs_index, false);
  //}
  auto transmitters=get_tx_list();
  for(auto& tx: transmitters){
    tx->update_mcs_index(mcs_index);
  }
  return true;
}

bool WBLink::request_set_channel_width(int channel_width) {
  m_console->debug("request_set_channel_width {}",channel_width);
  if(!openhd::is_valid_channel_width(channel_width)){
    m_console->warn("Invalid channel width {}",channel_width);
    return false;
  }
  if(!validate_cards_support_setting_channel_width()){
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

bool WBLink::set_video_fec_block_length(const int block_length) {
  m_console->debug("set_video_fec_block_length {}",block_length);
  if(!openhd::is_valid_fec_block_length(block_length)){
    m_console->warn("Invalid fec block length:{}",block_length);
    return false;
  }
  m_settings->unsafe_get_settings().wb_video_fec_block_length=block_length;
  m_settings->persist();
  // we only use the fec blk length for video tx-es, and changing it is fast
  for(auto& tx: m_wb_video_tx_list){
    tx->update_fec_k(block_length);
  }
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
  // we only use the fec percentage for video txes, and changing it is fast
  for(auto& tx: m_wb_video_tx_list){
    tx->update_fec_percentage(fec_percentage);
  }
  return true;
}


std::vector<openhd::Setting> WBLink::get_all_settings(){
  using namespace openhd;
  std::vector<openhd::Setting> ret{};
  const auto settings=m_settings->get_settings();
  auto change_freq=openhd::IntSetting{(int)m_settings->get_settings().wb_frequency,[this](std::string,int value){
                                          return request_set_frequency(value);
                                        }};
  auto change_wb_channel_width=openhd::IntSetting{(int)m_settings->get_settings().wb_channel_width,[this](std::string,int value){
                                                      return request_set_channel_width(value);
                                                    }};
  auto change_wb_mcs_index=openhd::IntSetting{(int)m_settings->get_settings().wb_mcs_index,[this](std::string,int value){
                                                  return try_set_mcs_index(value);
                                                }};
  ret.push_back(Setting{WB_FREQUENCY,change_freq});
  ret.push_back(Setting{WB_CHANNEL_WIDTH,change_wb_channel_width});
  ret.push_back(Setting{WB_MCS_INDEX,change_wb_mcs_index});
  if(m_profile.is_air){
    auto change_video_fec_block_length=openhd::IntSetting{(int)m_settings->get_settings().wb_video_fec_block_length,[this](std::string,int value){
                                                              return set_video_fec_block_length(value);
                                                            }};
    ret.push_back(Setting{WB_VIDEO_FEC_BLOCK_LENGTH,change_video_fec_block_length});
    auto change_video_fec_percentage=openhd::IntSetting{(int)m_settings->get_settings().wb_video_fec_percentage,[this](std::string,int value){
                                                            return set_video_fec_percentage(value);
                                                          }};
    ret.push_back(Setting{WB_VIDEO_FEC_PERCENTAGE,change_video_fec_percentage});
    auto cb_enable_wb_video_variable_bitrate=[this](std::string,int value){
      return set_enable_wb_video_variable_bitrate(value);
    };
    ret.push_back(Setting{WB_VIDEO_VARIABLE_BITRATE,openhd::IntSetting{(int)m_settings->get_settings().enable_wb_video_variable_bitrate, cb_enable_wb_video_variable_bitrate}});
    auto cb_wb_max_fec_block_size_for_platform=[this](std::string,int value){
      return set_max_fec_block_size_for_platform(value);
    };
    ret.push_back(Setting{WB_MAX_FEC_BLOCK_SIZE_FOR_PLATFORM,openhd::IntSetting{(int)m_settings->get_settings().wb_max_fec_block_size_for_platform, cb_wb_max_fec_block_size_for_platform}});
  }
  if(m_profile.is_ground()){
    // We display the total n of detected RX cards such that users can validate their multi rx setup(s) if there is more than one rx card detected
    // (Note: air always has exactly one monitor mode wi-fi card)
    const int n_rx_cards=static_cast<int>(m_broadcast_cards.size());
    if(n_rx_cards>1){
      ret.push_back(openhd::create_read_only_int("WB_N_RX_CARDS",n_rx_cards));
    }
  }
  // disabled for now, they are too complicated that a normal user can do something with them anyways
  if(false){
    auto cb_wb_enable_stbc=[this](std::string,int value){
      if(!validate_yes_or_no(value))return false;
      m_settings->unsafe_get_settings().wb_enable_stbc=value;
      m_settings->persist();
      return true;
    };
    ret.push_back(openhd::Setting{WB_ENABLE_STBC,openhd::IntSetting{settings.wb_enable_stbc,cb_wb_enable_stbc}});
    auto cb_wb_enable_ldpc=[this](std::string,int value){
      if(!validate_yes_or_no(value))return false;
      m_settings->unsafe_get_settings().wb_enable_ldpc=value;
      m_settings->persist();
      return true;
    };
    ret.push_back(openhd::Setting{WB_ENABLE_LDPC,openhd::IntSetting{settings.wb_enable_stbc,cb_wb_enable_ldpc}});
    auto cb_wb_enable_sg=[this](std::string,int value){
      if(!validate_yes_or_no(value))return false;
      m_settings->unsafe_get_settings().wb_enable_short_guard=value;
      m_settings->persist();
      return true;
    };
    ret.push_back(openhd::Setting{WB_ENABLE_SHORT_GUARD,openhd::IntSetting{settings.wb_enable_short_guard,cb_wb_enable_sg}});
  }
  // WIFI TX power depends on the used chips
  if(has_rtl8812au()){
    auto cb_wb_rtl8812au_tx_pwr_idx_override=[this](std::string,int value){
      return try_set_tx_power_rtl8812au(value);
    };
    ret.push_back(openhd::Setting{WB_RTL8812AU_TX_PWR_IDX_OVERRIDE,openhd::IntSetting{(int)settings.wb_rtl8812au_tx_pwr_idx_override,cb_wb_rtl8812au_tx_pwr_idx_override}});
  }else{
    auto cb_wb_tx_power_milli_watt=[this](std::string,int value){
      return try_set_tx_power_mw(value);
    };
    auto change_tx_power=openhd::IntSetting{(int)m_settings->get_settings().wb_tx_power_milli_watt,cb_wb_tx_power_milli_watt};
    ret.push_back(Setting{WB_TX_POWER_MILLI_WATT,change_tx_power});
  }
  openhd::validate_provided_ids(ret);
  return ret;
}

bool WBLink::validate_cards_support_setting_mcs_index() {
  return openhd::wb::cards_support_setting_mcs_index(m_broadcast_cards);
}

bool WBLink::validate_cards_support_setting_channel_width() {
  return openhd::wb::cards_support_setting_channel_width(m_broadcast_cards);
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
    bool any_rx_wifi_disconnected_errors=false;
    if(m_wb_tele_rx->get_latest_stats().wb_rx_stats.n_receiver_likely_disconnect_errors>100){
      any_rx_wifi_disconnected_errors= true;
    }
    for(auto& rx: m_wb_video_rx_list){
      if(rx->get_latest_stats().wb_rx_stats.n_receiver_likely_disconnect_errors>100){
        any_rx_wifi_disconnected_errors= true;
      }
    }
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
  if(m_foreign_packets_receiver){
    const auto stats=m_foreign_packets_receiver->get_current_stats();
    m_console->debug("Foreign packets stats:{}",stats.to_string());
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
    stats.telemetry.curr_rx_bps=curr_rx_stats.wb_rx_stats.curr_incoming_bits_per_second;
    //stats.telemetry.curr_rx_pps=curr_rx_stats.wb_rx_stats;
  }
  if(m_profile.is_air){
    // video on air
    for(int i=0;i< m_wb_video_tx_list.size();i++){
      auto& wb_tx= *m_wb_video_tx_list.at(i);
      //auto& air_video=i==0 ? stats.air_video0 : stats.air_video1;
      const auto curr_tx_stats=wb_tx.get_latest_stats();
      openhd::link_statistics::StatsWBVideoAir air_video{};
      //
      air_video.link_index=i;
      air_video.curr_measured_encoder_bitrate=curr_tx_stats.current_provided_bits_per_second;
      air_video.curr_injected_bitrate=curr_tx_stats.current_injected_bits_per_second;
      air_video.curr_injected_pps=curr_tx_stats.current_injected_packets_per_second;
      air_video.curr_dropped_packets=curr_tx_stats.n_dropped_packets;
      //
      const auto curr_tx_fec_stats=wb_tx.get_latest_fec_stats();
      air_video.curr_fec_encode_time_avg_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.avg);
      air_video.curr_fec_encode_time_min_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.min);
      air_video.curr_fec_encode_time_max_us= get_micros(curr_tx_fec_stats.curr_fec_encode_time.max);
      air_video.curr_fec_block_size_min=curr_tx_fec_stats.curr_fec_block_length.min;
      air_video.curr_fec_block_size_max=curr_tx_fec_stats.curr_fec_block_length.max;
      air_video.curr_fec_block_size_avg=curr_tx_fec_stats.curr_fec_block_length.avg;
      // TODO otimization: Only send stats for an active link
      stats.stats_wb_video_air.push_back(air_video);
    }
  }else{
    // video on ground
    for(int i=0;i< m_wb_video_rx_list.size();i++){
      auto& wb_rx= *m_wb_video_rx_list.at(i);
      const auto wb_rx_stats=wb_rx.get_latest_stats();
      //if(wb_rx_stats.wb_rx_stats.last_received_packet_mcs_index>=0){
      //  m_console->debug("MCS {}",wb_rx_stats.wb_rx_stats.last_received_packet_mcs_index);
      //}
      openhd::link_statistics::StatsWBVideoGround ground_video{};
      //
      ground_video.link_index=i;
      ground_video.curr_incoming_bitrate=wb_rx_stats.wb_rx_stats.curr_incoming_bits_per_second;
      if(wb_rx_stats.fec_rx_stats.has_value()){
        const auto fec_stats=wb_rx_stats.fec_rx_stats.value();
        ground_video.count_fragments_recovered=fec_stats.count_fragments_recovered;
        ground_video.count_blocks_recovered=fec_stats.count_blocks_recovered;
        ground_video.count_blocks_lost=fec_stats.count_blocks_lost;
        ground_video.count_blocks_total=fec_stats.count_blocks_total;
        ground_video.curr_fec_decode_time_avg_us =get_micros(fec_stats.curr_fec_decode_time.avg);
        ground_video.curr_fec_decode_time_min_us =get_micros(fec_stats.curr_fec_decode_time.min);
        ground_video.curr_fec_decode_time_max_us =get_micros(fec_stats.curr_fec_decode_time.max);
      }
      // TODO otimization: Only send stats for an active link
      stats.stats_wb_video_ground.push_back(ground_video);
    }
  }
  // DIRTY: On air, we use the telemetry lost packets percentage, on ground,
  // we use the video lost packets' percentage. Once we have one global rx, we can change that
  if(m_profile.is_air){
    if(m_wb_tele_rx){
      stats.monitor_mode_link.curr_rx_packet_loss_perc=
          m_wb_tele_rx->get_latest_stats().wb_rx_stats.curr_packet_loss_percentage;
    }
  }else{
    if(!m_wb_video_rx_list.empty()){
      stats.monitor_mode_link.curr_rx_packet_loss_perc=
          m_wb_video_rx_list.at(0)->get_latest_stats().wb_rx_stats.curr_packet_loss_percentage;
    }
  }
  // temporary, accumulate tx error(s) and dropped packets
  uint64_t acc_tx_injections_error_hint=0;
  uint64_t acc_tx_n_dropped_packets=0;
  acc_tx_injections_error_hint+= m_wb_tele_tx->get_latest_stats().count_tx_injections_error_hint;
  acc_tx_n_dropped_packets+= m_wb_tele_tx->get_latest_stats().n_dropped_packets;
  for(const auto& videoTx: m_wb_video_tx_list){
    acc_tx_injections_error_hint+=videoTx->get_latest_stats().count_tx_injections_error_hint;
    acc_tx_n_dropped_packets+=videoTx->get_latest_stats().n_dropped_packets;
  }
  stats.monitor_mode_link.count_tx_inj_error_hint=acc_tx_injections_error_hint;
  stats.monitor_mode_link.count_tx_dropped_packets=acc_tx_n_dropped_packets;

  // dBm is per card, not per stream
  assert(stats.cards.size()>=4);
  // only populate actually used cards
  assert(m_broadcast_cards.size()<=stats.cards.size());
  for(int i=0;i< m_broadcast_cards.size();i++){
    auto& card = stats.cards.at(i);
    if(m_profile.is_air){
      // on air, we use the dbm reported by the telemetry stream
      card.rx_rssi= m_wb_tele_rx->get_latest_stats().stats_per_card.at(i).rssi_for_wifi_card.last_rssi;
    }else{
      // on ground, we use the dBm reported by the video stream (if available), otherwise
      // we use the dBm reported by the telemetry rx instance.
      const int8_t rssi_telemetry= m_wb_tele_rx->get_latest_stats().stats_per_card.at(i).rssi_for_wifi_card.last_rssi;
      const int8_t rssi_video0= m_wb_video_rx_list.at(0)->get_latest_stats().stats_per_card.at(i).rssi_for_wifi_card.last_rssi;
      if(rssi_video0<=-127){
        // use telemetry, most likely no video data (yet)
        card.rx_rssi=rssi_telemetry;
      }else{
        card.rx_rssi=rssi_video0;
      }
    }
    card.exists_in_openhd= true;
    // accumulate all rx-es
    {
      uint64_t count_p_received_this_card=0;
      if(m_wb_tele_rx)count_p_received_this_card+=m_wb_tele_rx->get_latest_stats().stats_per_card.at(i).count_received_packets;
      for(auto& rx:m_wb_video_rx_list){
        count_p_received_this_card+=rx->get_latest_stats().stats_per_card.at(i).count_received_packets;
      }
      card.count_p_received=count_p_received_this_card;
    }
    // not yet supported
    card.count_p_injected=0;
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
  // stupid encoder rate control
  // TODO improve me !
  // First, calculate the theoretical values
  const auto settings = m_settings->get_settings();
  const uint32_t max_rate_possible_kbits =
      openhd::get_max_rate_kbits(settings.wb_mcs_index);
  // m_console->debug("mcs index:{}",settings.wb_mcs_index);
  //  we assume X% of the theoretical link bandwidth is available for the primary video stream 2.4G are almost always completely full of noise, which is why we go with a more conservative perc. value for them. NOTE: It is stupid to reason about the RF environment of the user, but feedback from the beta channel shows that this is kinda needed.
  const bool is_2g_channel =
      openhd::is_valid_frequency_2G(settings.wb_frequency);
  const uint32_t kFactorAvailablePerc = is_2g_channel ? 70 : 80;
  const uint32_t max_video_allocated_kbits =
      max_rate_possible_kbits * kFactorAvailablePerc / 100;
  // and deduce the FEC overhead
  const uint32_t max_video_after_fec_kbits =
      max_video_allocated_kbits * 100 /
      (100 + settings.wb_video_fec_percentage);
  m_console->debug(
      "max_rate_possible_kbits:{} kFactorAvailablePerc:{} max_video_after_fec_kbits:{}",
      max_rate_possible_kbits, kFactorAvailablePerc,
      max_video_after_fec_kbits);

  // then check if there are tx errors since the last time we checked (1 second intervals)
  bool bitrate_is_still_too_high = false;
  auto& primary_video_tx = *m_wb_video_tx_list.at(0).get();
  const auto primary_video_tx_stats = primary_video_tx.get_latest_stats();
  if (last_tx_error_count < 0) {
    last_tx_error_count = static_cast<int64_t>(
        primary_video_tx_stats.count_tx_injections_error_hint);
  } else {
    const auto delta = primary_video_tx_stats.count_tx_injections_error_hint -
                       last_tx_error_count;
    last_tx_error_count = static_cast<int64_t>(
        primary_video_tx_stats.count_tx_injections_error_hint);
    if (delta >= 1) {
      bitrate_is_still_too_high = true;
    }
  }
  // initialize with the theoretical default, since we do not know what the camera is doing, even though it probably is "too high".
  if (last_recommended_bitrate <= 0) {
    last_recommended_bitrate = max_video_after_fec_kbits;
  }
  if (bitrate_is_still_too_high) {
    m_console->warn("Bitrate probably too high");
    // reduce bitrate slightly
    last_recommended_bitrate = last_recommended_bitrate * 80 / 100;
  } else {
    if (last_recommended_bitrate < max_video_after_fec_kbits) {
      // otherwise, slowly increase bitrate
      last_recommended_bitrate = last_recommended_bitrate * 120 / 100;
    }
  }
  // 1Mbit/s as lower limit
  if (last_recommended_bitrate < 1000) {
    last_recommended_bitrate = 1000;
  }
  // theoretical max as upper limit
  if (last_recommended_bitrate > max_video_after_fec_kbits) {
    last_recommended_bitrate = max_video_after_fec_kbits;
  }
  if (m_opt_action_handler) {
    openhd::ActionHandler::LinkBitrateInformation lb{};
    lb.recommended_encoder_bitrate_kbits = last_recommended_bitrate;
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


bool WBLink::has_rtl8812au() {
  for(const auto& card: m_broadcast_cards){
    if(card.type==WiFiCardType::Realtek8812au){
      return true;
    }
  }
  return false;
}

void WBLink::transmit_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data) {
  const auto res=m_wb_tele_tx->try_enqueue_packet(data);
  if(!res)m_console->debug("Enqueing tele packet failed");
}

void WBLink::transmit_video_data(int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame){
  assert(m_profile.is_air);
  if(stream_index>=0 && stream_index< m_wb_video_tx_list.size()){
    auto& tx= *m_wb_video_tx_list[stream_index];
    const bool use_fixed_fec_instead=!m_settings->get_settings().is_video_variable_block_length_enabled();
    //tx.tmp_feed_frame_fragments(fragmented_video_frame.frame_fragments,use_fixed_fec_instead);
    uint32_t max_block_size_for_platform=m_settings->get_settings().wb_max_fec_block_size_for_platform;
    //openhd::log::get_default()->debug("max_block_size_for_platform:{}",max_block_size_for_platform);
    if(!openhd::valid_wb_max_fec_block_size_for_platform(max_block_size_for_platform)){
      openhd::log::get_default()->warn("Invalid max_block_size_for_platform:{}",max_block_size_for_platform);
      max_block_size_for_platform=openhd::DEFAULT_MAX_FEC_BLK_SIZE_FOR_PLATFORM;
    }
    if(m_settings->get_settings().is_video_variable_block_length_enabled()){
      tx.try_enqueue_block(fragmented_video_frame.frame_fragments,max_block_size_for_platform);
    }else{
      tx.try_enqueue_block(fragmented_video_frame.frame_fragments, 100);
    }
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
      const int n_packets= get_rx_count_p_all();
      // We might receive 20Mhz channel width packets from a air unit sending on 20Mhz channel width while
      // receiving on 40Mhz channel width - if we were to then to set the gnd to 40Mhz, we will be able to receive data,
      // but not be able to send any data up to the air unit.
      const int rx_chann_width=get_last_rx_packet_chan_width();
      m_console->debug("Got {} packets on {}@{}",n_packets,channel.frequency,rx_chann_width);
      if(n_packets>0){
        // We got packets on this frequency, but it is not guaranteed those packets are from an openhd air unit.
        // sleep a bit more, then check if we actually got any decrypted packets
        std::this_thread::sleep_for(std::chrono::seconds(2));
        const int n_packets_decrypted= get_rx_count_p_decryption_ok();
        const int packet_loss=m_wb_video_rx_list.at(0)->get_latest_stats().wb_rx_stats.curr_packet_loss_percentage;
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
  auto receivers=get_rx_list();
  for(auto& rx:receivers){
    rx->reset_all_rx_stats();
  }
}

int WBLink::get_rx_count_p_all() {
  auto receivers=get_rx_list();
  int total=0;
  for(auto& rx:receivers){
    total += static_cast<int>(rx->get_latest_stats().wb_rx_stats.count_p_all);
  }
  return total;
}

int WBLink::get_rx_count_p_decryption_ok() {
  auto receivers=get_rx_list();
  int total=0;
  for(auto& rx:receivers){
    total += static_cast<int>(rx->get_latest_stats().wb_rx_stats.count_p_decryption_ok);
  }
  return total;
}

int WBLink::get_last_rx_packet_chan_width() {
  for(auto& rx:m_wb_video_rx_list){
    auto rx_stats=rx->get_latest_stats().wb_rx_stats;
    if(rx_stats.last_received_packet_channel_width!=-1)return rx_stats.last_received_packet_channel_width;
  }
  return -1;
}

bool WBLink::check_in_state_support_changing_settings(){
  return !is_scanning && check_work_queue_empty();
}

openhd::Space WBLink::get_current_frequency_channel_space()const {
  return openhd::get_space_from_frequency(m_settings->get_settings().wb_frequency);
}

std::vector<WBTransmitter*> WBLink::get_tx_list() {
  std::vector<WBTransmitter*> ret;
  for(auto& vid_tx:m_wb_video_tx_list)ret.push_back(vid_tx.get());
  if(m_wb_tele_tx)ret.push_back(m_wb_tele_tx.get());
  return ret;
}

std::vector<AsyncWBReceiver*> WBLink::get_rx_list() {
  std::vector<AsyncWBReceiver*> ret;
  for(auto& vid_rx:m_wb_video_rx_list)ret.push_back(vid_rx.get());
  if(m_wb_tele_rx)ret.push_back(m_wb_tele_rx.get());
  return ret;
}

bool WBLink::try_set_tx_power_mw(int tx_power_mw) {
  m_console->debug("try_set_tx_power_mw {}mW",tx_power_mw);
  if(!openhd::is_valid_tx_power_milli_watt(tx_power_mw)){
    m_console->warn("Invalid tx power:{}mW",tx_power_mw);
    return false;
  }
  if(!check_in_state_support_changing_settings())return false;
  m_settings->unsafe_get_settings().wb_tx_power_milli_watt=tx_power_mw;
  m_settings->persist();
  apply_txpower();
  return true;
}

bool WBLink::try_set_tx_power_rtl8812au(int tx_power_index_override){
  m_console->debug("try_set_tx_power_rtl8812au {}index",tx_power_index_override);
  if(!openhd::validate_wb_rtl8812au_tx_pwr_idx_override(tx_power_index_override))return false;
  if(!check_in_state_support_changing_settings())return false;
  m_settings->unsafe_get_settings().wb_rtl8812au_tx_pwr_idx_override=tx_power_index_override;
  m_settings->persist();
  apply_txpower();
  return true;
}