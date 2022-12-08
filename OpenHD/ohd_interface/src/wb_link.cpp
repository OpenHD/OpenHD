#include "wb_link.h"
#include "wifi_command_helper.hpp"
//#include "wifi_command_helper2.h"

#include <iostream>
#include <utility>

#include "openhd-dirty-fatal-error.hpp"
#include "openhd-global-constants.hpp"
#include "openhd-platform.hpp"
#include "openhd-spdlog.hpp"
#include "openhd-util-filesystem.hpp"
#include "wifi_card.hpp"

static const char *KEYPAIR_FILE_DRONE = "/usr/local/share/openhd/drone.key";
static const char *KEYPAIR_FILE_GROUND = "/usr/local/share/openhd/gs.key";

WBLink::WBLink(OHDProfile profile,OHDPlatform platform,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards1,
                     std::shared_ptr<openhd::ActionHandler> opt_action_handler) : m_profile(std::move(profile)),
      m_platform(platform),
      m_broadcast_cards(std::move(broadcast_cards1)),
   m_disable_all_frequency_checks(OHDFilesystemUtil::exists(FIlE_DISABLE_ALL_FREQUENCY_CHECKS)),
   m_opt_action_handler(std::move(opt_action_handler))
{
  m_console = openhd::log::create_or_get("wb_streams");
  assert(m_console);
  m_console->info("Broadcast cards:{}",debug_cards(m_broadcast_cards));
  m_console->debug("m_disable_all_frequency_checks:"+OHDUtil::yes_or_no(m_disable_all_frequency_checks));
  // sanity checks
  if(m_broadcast_cards.empty()) {
    // NOTE: Here we crash, since it would be a programmer(s) error to create a WBStreams instance without at least 1 wifi card
    // (at least 1 card supporting monitor mode)
    // In OHDInterface, we handle it more gracefully with an error code.
    m_console->error("Without at least one wifi card, the stream(s) cannot be started");
    exit(1);
  }
  // more than 4 cards would be completely insane, most likely a programming error
  assert(m_broadcast_cards.size()<=4);
  // sanity checking
  for(const auto& card: m_broadcast_cards){
    assert(card->get_settings().use_for==WifiUseFor::MonitorMode);
    assert(card->_wifi_card.supports_2ghz || card->_wifi_card.supports_5ghz);
  }
  if (m_profile.is_air && m_broadcast_cards.size() > 1) {
    // We cannot use more than 1 wifi card for injection
    m_console->warn("dangerous, the air unit should not have more than 1 wifi card for wifibroadcast");
    // We just use the first one, this points to a upper level programming error or something weird.
    m_broadcast_cards.resize(1);
  }
  // this fetches the last settings, otherwise creates default ones
  m_settings =std::make_unique<openhd::WBStreamsSettingsHolder>(platform,openhd::tmp_convert(m_broadcast_cards));
  // check if the cards connected match the previous settings.
  // For now, we check if the first wb card can do 2 / 4 ghz, and assume the rest can do the same
  const auto first_card= m_broadcast_cards.at(0)->_wifi_card;
  if(m_settings->get_settings().configured_for_2G()){
    if(! first_card.supports_2ghz){
      // we need to switch to 5ghz, since the connected card cannot do 2ghz
      m_console->warn("WB configured for 2G but card can only do 5G - overwriting old settings");
      m_settings->set_default_5G();
    }
  }else{
    if(!first_card.supports_5ghz){
      // similar, we need to switch to 2G
      m_console->warn("WB configured for 5G but card can only do 2G - overwriting old settings");
      m_settings->set_default_2G();
    }
  }
  if(!validate_cards_support_setting_mcs_index()){
    // cards that do not support changing the mcs index are always fixed to mcs3 on openhd drivers
    m_settings->unsafe_get_settings().wb_mcs_index=3;
    m_settings->persist();
  }
  takeover_cards_monitor_mode();
  configure_cards();
  configure_streams();
  m_work_thread_run = true;
  m_work_thread =std::make_unique<std::thread>(&WBLink::loop_do_work, this);
}

WBLink::~WBLink() {
  if(m_work_thread){
    m_work_thread_run =false;
    m_work_thread->join();
  }
}

void WBLink::takeover_cards_monitor_mode() {
  m_console->debug( "WBStreams::takeover_cards_monitor_mode() begin");
  // We need to take "ownership" from the system over the cards used for monitor mode / wifibroadcast.
  // This can be different depending on the OS we are running on - in general, we try to go for the following with openhd:
  // Have network manager running on the host OS - the nice thing about network manager is that we can just tell it
  // to ignore the cards we are doing wifibroadcast with, instead of killing all processes that might interfere with
  // wifibroadcast and therefore making other networking increadibly hard.
  // Tell network manager to ignore the cards we want to do wifibroadcast on
  for(const auto& card: m_broadcast_cards){
    WifiCardCommandHelper::network_manager_set_card_unmanaged(card->_wifi_card);
  }
  OHDUtil::run_command("rfkill",{"unblock","all"});
  // TODO: sometimes this happens:
  // 1) Running openhd fist time: pcap_compile doesn't work (fatal error)
  // 2) Running openhd second time: works
  // I cannot find what's causing the issue - a sleep here is the worst solution, but r.n the only one I can come up with
  // perhaps we'd need to wait for network manager to finish switching to ignoring the monitor mode cards ?!
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // now we can enable monitor mode on the given cards.
  for(const auto& card: m_broadcast_cards) {
    WifiCardCommandHelper::set_card_state(card->_wifi_card, false);
    WifiCardCommandHelper::enable_monitor_mode(card->_wifi_card);
    WifiCardCommandHelper::set_card_state(card->_wifi_card, true);
    //wifi::commandhelper2::set_wifi_monitor_mode(card->_wifi_card.interface_name);
  }
  m_console->debug("WBStreams::takeover_cards_monitor_mode() end");
}

void WBLink::configure_cards() {
  m_console->debug("WBStreams::configure_cards() begin");
  apply_frequency_and_channel_width();
  apply_txpower();
  m_console->debug("WBStreams::configure_cards() end");
}

void WBLink::configure_streams() {
  m_console->debug("Streams::configure() begin");
  // Increase the OS max UDP buffer size (only works as root) such that the wb video UDP receiver
  // doesn't fail when requesting a bigger UDP buffer size
  // NOTE: This value is quite high, but that doesn't matter - this is the max allowed, not what is set,
  // and doesn't change the actual / default size
  OHDUtil::run_command("sysctl ",{"-w","net.core.rmem_max=26214400"});
  configure_telemetry();
  configure_video();
  m_console->debug("Streams::configure() end");
}

void WBLink::configure_telemetry() {
  m_console->debug("Streams::configure_telemetry()isAir:"+OHDUtil::yes_or_no(m_profile.is_air));
  // Setup the tx & rx instances for telemetry. Telemetry is bidirectional,aka
  // uses 2 UDP streams in oposite directions.
  auto radioPort1 = m_profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT : OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT;
  auto radioPort2 = m_profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT : OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT;
  auto udpPort1 = m_profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX
                                  : OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX;
  auto udpPort2 = m_profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX
                                  : OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX;
  udpTelemetryRx = createUdpWbRx(radioPort1, udpPort1);
  udpTelemetryTx = createUdpWbTx(radioPort2, udpPort2,false);
  udpTelemetryRx->runInBackground();
  udpTelemetryTx->runInBackground();
}

void WBLink::configure_video() {
  m_console->debug("Streams::configure_video()");
  // Video is unidirectional, aka always goes from air pi to ground pi
  if (m_profile.is_air) {
    auto primary = createUdpWbTx(OHD_VIDEO_PRIMARY_RADIO_PORT, OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP,true,1024*1024*5);
    primary->runInBackground();
    auto secondary = createUdpWbTx(OHD_VIDEO_SECONDARY_RADIO_PORT, OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP,true,1024*1024*2);
    secondary->runInBackground();
    udpVideoTxList.push_back(std::move(primary));
    udpVideoTxList.push_back(std::move(secondary));
  } else {
    auto primary = createUdpWbRx(OHD_VIDEO_PRIMARY_RADIO_PORT, OHD_VIDEO_GROUND_VIDEO_STREAM_1_UDP);
    primary->runInBackground();
    auto secondary = createUdpWbRx(OHD_VIDEO_SECONDARY_RADIO_PORT, OHD_VIDEO_GROUND_VIDEO_STREAM_2_UDP);
    secondary->runInBackground();
    udpVideoRxList.push_back(std::move(primary));
    udpVideoRxList.push_back(std::move(secondary));
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

std::unique_ptr<UDPWBTransmitter> WBLink::createUdpWbTx(uint8_t radio_port, int udp_port,bool enableFec,
                                                           std::optional<int> udp_recv_buff_size)const {
  const auto settings=m_settings->get_settings();
  TOptions options{};
  options.radio_port = radio_port;
  const char *keypair_file =
      m_profile.is_air ? KEYPAIR_FILE_DRONE : KEYPAIR_FILE_GROUND;
  if(OHDFilesystemUtil::exists(keypair_file)){
    options.keypair = std::string(keypair_file);
    m_console->debug("Using key from file {}",options.keypair->c_str());
  }else{
    options.keypair = std::nullopt;
  }
  if(enableFec){
    options.enable_fec= true;
    options.tx_fec_options.fixed_k=static_cast<int>(settings.wb_video_fec_block_length);
    options.tx_fec_options.overhead_percentage=static_cast<int>(settings.wb_video_fec_percentage);
  }else{
    options.enable_fec= false;
    options.tx_fec_options.fixed_k=0;
    options.tx_fec_options.overhead_percentage=0;
  }
  options.wlan = m_broadcast_cards.at(0)->_wifi_card.interface_name;
  //m_console->debug("Starting WFB_TX with MCS:{}",mcs_index);
  RadiotapHeader::UserSelectableParams wifiParams= create_radiotap_params();
  return std::make_unique<UDPWBTransmitter>(wifiParams, options, "127.0.0.1", udp_port,udp_recv_buff_size);
}

std::unique_ptr<UDPWBReceiver> WBLink::createUdpWbRx(uint8_t radio_port, int udp_port){
  ROptions options{};
  options.radio_port = radio_port;
  const char *keypair_file =
      m_profile.is_air ? KEYPAIR_FILE_DRONE : KEYPAIR_FILE_GROUND;
  if(OHDFilesystemUtil::exists(keypair_file)){
    options.keypair = std::string(keypair_file);
    m_console->debug("Using key from file {}",options.keypair->c_str());
  }else{
    options.keypair = std::nullopt;
  }
  const auto cards = get_rx_card_names();
  assert(!cards.empty());
  options.rxInterfaces = cards;
  // use rx queue depth of 1 for now, this should at least reduce the problem of the burst /
  // high latency when blocks are lost.
  // Multiple rx wifi card's won't provide a benefit with this parameter set though.
  options.rx_queue_depth = 1;//_broadcast_cards.size() > 1 ? 10 : 2;
  const auto wifi_card_type=m_broadcast_cards.at(0)->get_wifi_card().type;
  options.rtl8812au_rssi_fixup=wifi_card_type==WiFiCardType::Realtek8812au;
  return std::make_unique<UDPWBReceiver>(options, "127.0.0.1", udp_port);
}

std::string WBLink::createDebug()const{
  std::stringstream ss;
  // we use telemetry data only here
  bool any_data_received=false;
  if(udpTelemetryRx && udpTelemetryRx->anyDataReceived()){
    any_data_received=true;
  }
  ss<<"Any data received: "<<(any_data_received ? "Y":"N")<<"\n";
  if (udpTelemetryRx) {
    ss<<"TeleRx: "<<udpTelemetryRx->createDebug();
  }
  if (udpTelemetryTx) {
    ss<<"TeleTx: "<<udpTelemetryTx->createDebug();
  }
  for (const auto &txvid: udpVideoTxList) {
    ss<<"VidTx: "<<txvid->createDebug();
  }
  for (const auto &rxvid: udpVideoRxList) {
    ss<<"VidRx :"<<rxvid->createDebug();
  }
  return ss.str();
}

void WBLink::addExternalDeviceIpForwardingVideoOnly(const std::string& ip) {
  bool first= true;
  assert(udpVideoRxList.size()==2);
  m_console->info("WBStreams::addExternalDeviceIpForwardingVideoOnly:"+ip);
  // forward video
  for(auto& rxVid:udpVideoRxList){
    const auto udpPort=first ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
    first= false;
    rxVid->addForwarder(ip,udpPort);
  }
}

void WBLink::removeExternalDeviceIpForwardingVideoOnly(const std::string& ip) {
  bool first= true;
  assert(udpVideoRxList.size()==2);
  for(auto& rxVid:udpVideoRxList){
    const auto udpPort=first ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
    first= false;
    rxVid->removeForwarder(ip,udpPort);
  }
}

std::vector<std::string> WBLink::get_rx_card_names() const {
  std::vector<std::string> ret{};
  for(const auto& card: m_broadcast_cards){
    ret.push_back(card->_wifi_card.interface_name);
  }
  return ret;
}

bool WBLink::ever_received_any_data(){
  if(m_profile.is_air){
    // check if we got any telemetry data, we never receive video data
    assert(udpTelemetryRx);
    return udpTelemetryRx->anyDataReceived();
  }
  // ground
  bool any_data_received=false;
  // any telemetry data
  assert(udpTelemetryRx);
  if(udpTelemetryRx->anyDataReceived()){
    any_data_received=true;
  }
  // or any video data
  for(const auto& vidrx:udpVideoRxList){
    if(vidrx->anyDataReceived()){
      any_data_received=true;
    }
  }
  return any_data_received;
}

bool WBLink::request_set_frequency(int frequency) {
  m_console->debug("WBStreams::request_set_frequency {}",frequency);
  if(m_disable_all_frequency_checks){
    m_console->warn("Not sanity checking frequency");
  }else{
    // channels below and above the normal channels, not allowed in most countries
    bool cards_support_extra_2G_channels=false;
    if(m_platform.platform_type==PlatformType::RaspberryPi ||
        m_platform.platform_type==PlatformType::Jetson){
      // modified kernel
      cards_support_extra_2G_channels= all_cards_support_extra_channels_2G(m_broadcast_cards);
    }
    m_console->debug("cards_support_extra_2G_channels:"+OHDUtil::yes_or_no(cards_support_extra_2G_channels));
    // check if the card supports both 2G and 5G
    const bool cards_supports_both_frequencies = all_cards_support_2G(m_broadcast_cards) &&
                                                 all_cards_support_5G(m_broadcast_cards);
    m_console->debug("cards_supports_both_frequencies:"+OHDUtil::yes_or_no(cards_supports_both_frequencies));
    if(cards_supports_both_frequencies){
      // allow switching between 2G and 5G
      const bool frequency_valid=openhd::is_valid_frequency_2G(frequency,cards_support_extra_2G_channels)
                                   || openhd::is_valid_frequency_5G(frequency);
      if(!frequency_valid){
        m_console->debug("Not a valid 2G or 5G frequency {}",frequency);
        return false;
      }
    }else{
      // do not allow switching between 2G and 5G
      if(m_settings->get_settings().configured_for_2G()){
        if(!openhd::is_valid_frequency_2G(frequency,cards_support_extra_2G_channels)){
          m_console->warn("Invalid 2.4G frequency {}",frequency);
          return false;
        }
      }else{
        if(!openhd::is_valid_frequency_5G(frequency)){
          m_console->warn("Invalid 5G frequency {}",frequency);
          return false;
        }
      }
    }
  }
  if(!check_work_queue_empty())return false;
  m_settings->unsafe_get_settings().wb_frequency=frequency;
  m_settings->persist();
  // We need to delay the change to make sure the mavlink ack has enough time to make it to the ground
  auto work_item=std::make_shared<WorkItem>([this](){
    apply_frequency_and_channel_width();
  },std::chrono::steady_clock::now()+ DELAY_FOR_TRANSMIT_ACK);
  schedule_work_item(work_item);
  return true;
}

void WBLink::apply_frequency_and_channel_width() {
  const auto settings=m_settings->get_settings();
  const bool width_40= settings.wb_channel_width==40;
  for(const auto& card: m_broadcast_cards){
    WifiCardCommandHelper::set_frequency_and_channel_width(card->_wifi_card,settings.wb_frequency,width_40);
  }
}

bool WBLink::request_set_txpower(int tx_power) {
  m_console->debug("WBStreams::request_set_txpower {}",tx_power);
  if(!openhd::is_valid_tx_power_milli_watt(tx_power)){
    m_console->warn("Invalid tx power:{}",tx_power);
    return false;
  }
  if(!check_work_queue_empty())return false;
  m_settings->unsafe_get_settings().wb_tx_power_milli_watt=tx_power;
  m_settings->persist();
  // No need to delay the change, but perform it async anyways.
  auto work_item=std::make_shared<WorkItem>([this](){
    apply_txpower();
  },std::chrono::steady_clock::now());
  schedule_work_item(work_item);
  return true;
}

void WBLink::apply_txpower() {
  const auto settings=m_settings->get_settings();
  for(const auto& holder: m_broadcast_cards){
    const auto& card=holder->_wifi_card;
    if(card.type==WiFiCardType::Realtek8812au){
      // requires corresponding driver workaround for dynamic tx power
      const auto tmp=settings.wb_rtl8812au_tx_pwr_idx_override;
      m_console->debug("RTL8812AU tx_pwr_idx_override: {}",tmp);
      WifiCardCommandHelper::iw_set_tx_power_mBm(card,tmp);
    }else{
      const auto tmp=openhd::milli_watt_to_mBm(settings.wb_tx_power_milli_watt);
      WifiCardCommandHelper::iw_set_tx_power_mBm(card,tmp);
      //WifiCardCommandHelper::iwconfig_set_txpower(card,settings.wb_tx_power_milli_watt);
    }
  }
}

bool WBLink::request_set_mcs_index(int mcs_index) {
  m_console->debug("WBStreams::request_set_mcs_index {}",mcs_index);
  if(!openhd::is_valid_mcs_index(mcs_index)){
    m_console->warn("Invalid mcs index{}",mcs_index);
    return false;
  }
  if(!validate_cards_support_setting_mcs_index()){
    m_console->warn("Cannot change mcs index, it is fixed for at least one of the used cards");
    return false;
  }
  if(!check_work_queue_empty())return false;
  m_settings->unsafe_get_settings().wb_mcs_index=mcs_index;
  m_settings->persist();
  // We need to delay the change to make sure the mavlink ack has enough time to make it to the ground
  auto work_item=std::make_shared<WorkItem>([this](){
    apply_mcs_index();
  },std::chrono::steady_clock::now()+ DELAY_FOR_TRANSMIT_ACK);
  schedule_work_item(work_item);
  return true;
}

void WBLink::apply_mcs_index() {
  // we need to change the mcs index on all tx-es
  const auto settings=m_settings->get_settings();
  if(udpTelemetryTx){
    udpTelemetryTx->update_mcs_index(settings.wb_mcs_index);
  }
  for(auto& tx:udpVideoTxList){
    tx->update_mcs_index(settings.wb_mcs_index);
  }
}

bool WBLink::request_set_channel_width(int channel_width) {
  m_console->debug("WBStreams::request_set_channel_width {}",channel_width);
  if(!openhd::is_valid_channel_width(channel_width)){
    m_console->warn("Invalid channel width {}",channel_width);
    return false;
  }
  if(!validate_cards_support_setting_channel_width()){
    m_console->warn("Cannot change channel width, at least one card doesn't support it");
    return false;
  }
  if(!check_work_queue_empty())return false;
  m_settings->unsafe_get_settings().wb_channel_width=channel_width;
  m_settings->persist();
  // We need to delay the change to make sure the mavlink ack has enough time to make it to the ground
  auto work_item=std::make_shared<WorkItem>([this](){
    apply_frequency_and_channel_width();
  },std::chrono::steady_clock::now()+ DELAY_FOR_TRANSMIT_ACK);
  schedule_work_item(work_item);
  return true;
}

bool WBLink::set_video_fec_block_length(const int block_length) {
  m_console->debug("WBStreams::set_video_fec_block_length {}",block_length);
  if(!openhd::is_valid_fec_block_length(block_length)){
    m_console->warn("Invalid fec block length:{}",block_length);
    return false;
  }
  m_settings->unsafe_get_settings().wb_video_fec_block_length=block_length;
  m_settings->persist();
  // we only use the fec blk length for video tx-es, and changing it is fast
  for(auto& tx:udpVideoTxList){
    tx->get_wb_tx().update_fec_k(block_length);
  }
  return true;
}

bool WBLink::set_video_fec_percentage(int fec_percentage) {
  m_console->debug("WBStreams::set_video_fec_percentage {}",fec_percentage);
  if(!openhd::is_valid_fec_percentage(fec_percentage)){
    m_console->warn("Invalid fec percentage:{}",fec_percentage);
    return false;
  }
  m_settings->unsafe_get_settings().wb_video_fec_percentage=fec_percentage;
  m_settings->persist();
  // we only use the fec percentage for video txes, and changing it is fast
  for(auto& tx:udpVideoTxList){
    tx->get_wb_tx().update_fec_percentage(fec_percentage);
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
                                                  return request_set_mcs_index(value);
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
      return set_wb_rtl8812au_tx_pwr_idx_override(value);
    };
    ret.push_back(openhd::Setting{WB_RTL8812AU_TX_PWR_IDX_OVERRIDE,openhd::IntSetting{(int)settings.wb_rtl8812au_tx_pwr_idx_override,cb_wb_rtl8812au_tx_pwr_idx_override}});
  }else{
    auto cb_wb_tx_power_milli_watt=[this](std::string,int value){
      return request_set_txpower(value);
    };
    auto change_tx_power=openhd::IntSetting{(int)m_settings->get_settings().wb_tx_power_milli_watt,cb_wb_tx_power_milli_watt};
    ret.push_back(Setting{WB_TX_POWER_MILLI_WATT,change_tx_power});
  }
  openhd::validate_provided_ids(ret);
  return ret;
}

bool WBLink::validate_cards_support_setting_mcs_index() {
  for(const auto& card_handle: m_broadcast_cards){
    if(!wifi_card_supports_variable_mcs(card_handle->_wifi_card)){
      return false;
    }
  }
  return true;
}

bool WBLink::validate_cards_support_setting_channel_width() {
  for(const auto& card_handle: m_broadcast_cards){
    if(!wifi_card_supports_40Mhz_channel_width(card_handle->_wifi_card)){
      return false;
    }
  }
  return true;
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
    update_statistics();
    //const auto delta_calc_stats=std::chrono::steady_clock::now()-begin_calculate_stats;
    //m_console->debug("Calculating stats took:{} ms",std::chrono::duration_cast<std::chrono::microseconds>(delta_calc_stats).count()/1000.0f);
    perform_rate_adjustment();
    // Dirty - deliberately crash openhd and let the service restart it
    // if we think a wi-fi card disconnected
    bool any_rx_wifi_disconnected_errors=false;
    if(udpTelemetryRx->get_latest_stats().wb_rx_stats.n_receiver_likely_disconnect_errors>100){
      any_rx_wifi_disconnected_errors= true;
    }
    for(auto& rx:udpVideoRxList){
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
  m_last_stats_recalculation=std::chrono::steady_clock::now();
  // telemetry is available on both air and ground
  openhd::link_statistics::StatsAirGround stats{};
  if(udpTelemetryTx){
    const auto curr_tx_stats=udpTelemetryTx->get_latest_stats();
    stats.telemetry.curr_tx_bps=curr_tx_stats.current_provided_bits_per_second;
    stats.telemetry.curr_tx_pps=curr_tx_stats.current_injected_packets_per_second;
  }
  if(udpTelemetryRx){
    const auto curr_rx_stats=udpTelemetryRx->get_latest_stats();
    stats.telemetry.curr_rx_bps=curr_rx_stats.wb_rx_stats.curr_incoming_bits_per_second;
    //stats.telemetry.curr_rx_pps=curr_rx_stats.wb_rx_stats;
  }
  if(m_profile.is_air){
    // video on air
    for(int i=0;i<udpVideoTxList.size();i++){
      auto& wb_tx=udpVideoTxList.at(i)->get_wb_tx();
      auto& air_video=i==0 ? stats.air_video0 : stats.air_video1;
      const auto curr_tx_stats=wb_tx.get_latest_stats();
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
    }
  }else{
    // video on ground
    for(int i=0;i<udpVideoRxList.size();i++){
      auto& wb_rx=udpVideoRxList.at(i)->get_wb_rx();
      const auto wb_rx_stats=wb_rx.get_latest_stats();
      auto& ground_video= i==0 ? stats.ground_video0 : stats.ground_video1;
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
    }
  }
  // DIRTY: On air, we use the telemetry lost packets percentage, on ground,
  // we use the video lost packets' percentage. Once we have one global rx, we can change that
  if(m_profile.is_air){
    if(udpTelemetryRx){
      stats.monitor_mode_link.curr_rx_packet_loss_perc=udpTelemetryRx->get_latest_stats().wb_rx_stats.curr_packet_loss_percentage;
    }
  }else{
    if(!udpVideoRxList.empty()){
      stats.monitor_mode_link.curr_rx_packet_loss_perc=udpVideoRxList.at(0)->get_latest_stats().wb_rx_stats.curr_packet_loss_percentage;
    }
  }
  // temporary, accumulate tx error(s) and dropped packets
  uint64_t acc_tx_injections_error_hint=0;
  uint64_t acc_tx_n_dropped_packets=0;
  acc_tx_injections_error_hint+=udpTelemetryTx->get_latest_stats().count_tx_injections_error_hint;
  acc_tx_n_dropped_packets+=udpTelemetryTx->get_latest_stats().count_tx_injections_error_hint;
  for(const auto& videoTx:udpVideoTxList){
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
      card.rx_rssi=
          udpTelemetryRx->get_latest_stats().rssiPerCard.at(i).last_rssi;
    }else{
      // on ground, we use the dBm reported by the video stream (if available), otherwise
      // we use the dBm reported by the telemetry rx instance.
      const int8_t rssi_telemetry=udpTelemetryRx->get_latest_stats().rssiPerCard.at(i).last_rssi;
      const int8_t rssi_video0=udpVideoRxList.at(0)->get_latest_stats().rssiPerCard.at(i).last_rssi;
      if(rssi_video0<=-127){
        // use telemetry, most likely no video data (yet)
        card.rx_rssi=rssi_telemetry;
      }else{
        card.rx_rssi=rssi_video0;
      }
    }
    card.exists_in_openhd= true;
    // not yet supported
    card.count_p_injected=0;
    card.count_p_received=0;
  }
  stats.is_air=m_profile.is_air;
  if(m_opt_action_handler){
    m_opt_action_handler->action_wb_link_statistcs_handle(stats);
  }
}

void WBLink::perform_rate_adjustment() {
  if(m_profile.is_air && m_settings->get_settings().enable_wb_video_variable_bitrate) {
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
        openhd::is_valid_frequency_2G(settings.wb_frequency, true);
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
    UDPWBTransmitter* primary_video_tx = udpVideoTxList.at(0).get();
    const auto primary_video_tx_stats = primary_video_tx->get_latest_stats();
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
    // or the tx queue is running full
    const auto n_buffered_packets_estimate =
        udpVideoTxList.at(0)->get_estimate_buffered_packets();
    m_console->debug("Video estimates {} buffered packets",
                     n_buffered_packets_estimate);
    if (n_buffered_packets_estimate >
        50) {  // half of the wifibroadcast extra tx queue
      bitrate_is_still_too_high = true;
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
}

bool WBLink::set_enable_wb_video_variable_bitrate(int value) {
  if(openhd::validate_yes_or_no(value)){
    // value is read in regular intervals.
    m_settings->unsafe_get_settings().enable_wb_video_variable_bitrate=value;
    return true;
  }
  return false;
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

bool WBLink::set_wb_rtl8812au_tx_pwr_idx_override(int value) {
  if(!openhd::validate_wb_rtl8812au_tx_pwr_idx_override(value))return false;
  if(!check_work_queue_empty())return false;
  m_settings->unsafe_get_settings().wb_rtl8812au_tx_pwr_idx_override=value;
  m_settings->persist();
  // No need to delay the change, but perform it async anyways.
  auto work_item=std::make_shared<WorkItem>([this](){
    apply_txpower();
  },std::chrono::steady_clock::now());
  schedule_work_item(work_item);
  return true;
}

bool WBLink::has_rtl8812au() {
  for(const auto& card_handle: m_broadcast_cards){
    if(card_handle->get_wifi_card().type==WiFiCardType::Realtek8812au){
      return true;
    }
  }
  return false;
}

void WBLink::transmit_video_data(int stream_index,const openhd::FragmentedVideoFrame& fragmented_video_frame){
  assert(m_profile.is_air);
  if(stream_index>=0 && stream_index<udpVideoTxList.size()){
    auto& tx=udpVideoTxList[stream_index]->get_wb_tx();
    const bool use_fixed_fec_instead=!m_settings->get_settings().is_video_variable_block_length_enabled();
    //tx.tmp_feed_frame_fragments(fragmented_video_frame.frame_fragments,use_fixed_fec_instead);
    uint32_t max_block_size_for_platform=m_settings->get_settings().wb_max_fec_block_size_for_platform;
    //openhd::log::get_default()->debug("max_block_size_for_platform:{}",max_block_size_for_platform);
    if(!openhd::valid_wb_max_fec_block_size_for_platform(max_block_size_for_platform)){
      openhd::log::get_default()->warn("Invalid max_block_size_for_platform:{}",max_block_size_for_platform);
      max_block_size_for_platform=openhd::DEFAULT_MAX_FEC_BLK_SIZE_FOR_PLATFORM;
    }
    if(m_settings->get_settings().is_video_variable_block_length_enabled()){
      tx.tmp_split_and_feed_frame_fragments(fragmented_video_frame.frame_fragments,max_block_size_for_platform);
    }else{
      tx.tmp_feed_frame_fragments(fragmented_video_frame.frame_fragments, true);
    }
  }
}
