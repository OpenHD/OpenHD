#include "WBStreams.h"
#include "WifiCardCommandHelper.hpp"

#include "openhd-platform.hpp"
#include "openhd-log.hpp"
#include "openhd-wifi.hpp"
#include "openhd-global-constants.h"

#include <cstdio>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <utility>

WBStreams::WBStreams(OHDProfile profile,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards) :
   _profile(std::move(profile)),_broadcast_cards(broadcast_cards) {
  std::cout<<"WBStreams::WBStreams:"<<broadcast_cards.size()<<"\n";
  // sanity checks
  if(_broadcast_cards.empty()) {
    // NOTE: Here we crash, since it would be a programmer(s) error to create a WBStreams instance without at least 1 wifi card.
    // In OHDInterface, we handle it more gracefully with an error code.
    std::cerr << "Without at least one wifi card, the stream(s) cannot be started\n";
    exit(1);
  }
  // more than 4 cards would be completely insane, most likely a programming error
  assert(_broadcast_cards.size()<=4);
  // sanity checking
  for(const auto& card: _broadcast_cards){
    assert(card->get_settings().use_for==WifiUseFor::MonitorMode);
  }
  if (_profile.is_air && _broadcast_cards.size() > 1) {
    // We cannot use more than 1 wifi card for injection
    std::cerr << "dangerous, the air unit should not have more than 1 wifi card for wifibroadcast\n";
    // We just use the first one, this points to a upper level programming error or something weird.
    _broadcast_cards.resize(1);
  }
  // We need to take "ownership" from the system over the cards used for monitor mode / wifibroadcast.
  // However, with the image set up by raphael they should be free from any (OS) prcoesses already
  for(const auto& card: _broadcast_cards){
    //TODO we might not need this one
    //OHDUtil::run_command("rfkill",{"unblock",card->_wifi_card.interface_name});
    WifiCardCommandHelper::set_card_state(card->_wifi_card, false);
    WifiCardCommandHelper::enable_monitor_mode(card->_wifi_card);
    WifiCardCommandHelper::set_card_state(card->_wifi_card, true);
    assert(!card->get_settings().frequency.empty());
    WifiCardCommandHelper::set_frequency(card->_wifi_card, card->get_settings().frequency);
    assert(!card->get_settings().txpower.empty());
    // TODO check if this works - on rtl8812au, the displayed value at least changes
    WifiCardCommandHelper::set_txpower(card->_wifi_card, card->get_settings().txpower);
    //WifiCards::set_txpower(card->_wifi_card, card->get_settings().txpower);
  }
  configure();
}

void WBStreams::configure() {
  std::cout << "Streams::configure() begin\n";
  // Static for the moment
  configure_telemetry();
  configure_video();
  std::cout << "Streams::configure() end\n";
}

void WBStreams::configure_telemetry() {
  std::cout << "Streams::configure_telemetry()isAir:"<<(_profile.is_air ? "Y":"N")<<std::endl;
  // Setup the tx & rx instances for telemetry. Telemetry is bidirectional,aka
  // uses 2 UDP streams in oposite directions.
  auto radioPort1 =
      _profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT : OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT;
  auto radioPort2 =
      _profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT : OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT;
  auto udpPort1 = _profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX
								 : OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX;
  auto udpPort2 = _profile.is_air ? OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX
								 : OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX;
  udpTelemetryRx = createUdpWbRx(radioPort1, udpPort1);
  udpTelemetryTx = createUdpWbTx(radioPort2, udpPort2,false);
  udpTelemetryRx->runInBackground();
  udpTelemetryTx->runInBackground();
}

void WBStreams::configure_video() {
  std::cout << "Streams::configure_video()" << std::endl;
  // Video is unidirectional, aka always goes from air pi to ground pi
  if (_profile.is_air) {
	auto primary = createUdpWbTx(OHD_VIDEO_PRIMARY_RADIO_PORT, OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP,true);
	primary->runInBackground();
	auto secondary = createUdpWbTx(OHD_VIDEO_SECONDARY_RADIO_PORT, OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP,true);
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

std::unique_ptr<UDPWBTransmitter> WBStreams::createUdpWbTx(uint8_t radio_port, int udp_port,bool enableFec)const {
  RadiotapHeader::UserSelectableParams wifiParams{20, false, 0, false, DEFAULT_MCS_INDEX};
  RadiotapHeader radiotapHeader{wifiParams};
  TOptions options{};
  // We log them all manually together
  options.enableLogAlive= false;
  options.radio_port = radio_port;
  options.keypair = std::nullopt;
  if(enableFec){
	options.fec_k=8;
	options.fec_percentage=20; // Default to 20% fec overhead
  }else{
	options.fec_k=0;
	options.fec_percentage=0;
  }
  options.wlan = _broadcast_cards.at(0)->_wifi_card.interface_name;
  return std::make_unique<UDPWBTransmitter>(radiotapHeader, options, "127.0.0.1", udp_port);
}

std::unique_ptr<UDPWBReceiver> WBStreams::createUdpWbRx(uint8_t radio_port, int udp_port){
  ROptions options{};
  // We log them all manually together
  options.enableLogAlive= false;
  options.radio_port = radio_port;
  options.keypair = std::nullopt;
  const auto cards = get_rx_card_names();
  assert(!cards.empty());
  options.rxInterfaces = cards;
  return std::make_unique<UDPWBReceiver>(options, "127.0.0.1", udp_port,[this](OpenHDStatisticsWriter::Data stats){
	this->onNewStatisticsData(stats);
  });
}

std::string WBStreams::createDebug() const {
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

void WBStreams::addExternalDeviceIpForwarding(const std::string& ip) {
  bool first= true;
  assert(udpVideoRxList.size()==2);
  // forward video
  for(auto& rxVid:udpVideoRxList){
	const auto udpPort=first ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
	first= false;
	rxVid->addForwarder(ip,udpPort);
  }
  // TODO how do we deal with telemetry
}
void WBStreams::removeExternalDeviceIpForwarding(const std::string& ip) {
  bool first= true;
  assert(udpVideoRxList.size()==2);
  for(auto& rxVid:udpVideoRxList){
	const auto udpPort=first ? OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP : OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP;
	first= false;
	rxVid->removeForwarder(ip,udpPort);
  }
}

std::vector<std::string> WBStreams::get_rx_card_names() const {
  std::vector<std::string> ret{};
  for(const auto& card: _broadcast_cards){
    ret.push_back(card->_wifi_card.interface_name);
  }
  return ret;
}

bool WBStreams::ever_received_any_data() const {
    if(_profile.is_air){
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

void WBStreams::onNewStatisticsData(const OpenHDStatisticsWriter::Data& data) {
  std::lock_guard<std::mutex> guard(_statisticsDataLock);
  // TODO make more understandable, but tele rx or tele tx is correct here
  if(data.radio_port==OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT
  || data.radio_port==OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT){
	_last_stats_per_stream.at(0)=data;
  }else if(data.radio_port==OHD_VIDEO_PRIMARY_RADIO_PORT){
	_last_stats_per_stream.at(1)=data;
  }else if(data.radio_port==OHD_VIDEO_SECONDARY_RADIO_PORT){
	_last_stats_per_stream.at(2)=data;
  }else{
	std::cerr<<"Unknown radio port on stats"<<(int)data.radio_port<<"\n";
	return;
  }
  std::cout<<"XGot stats "<<data<<"\n";
  // dBm is per card, not per stream
  assert(_stats_all_cards.size()>=4);
  // only populate actually used cards
  assert(_broadcast_cards.size()<=_stats_all_cards.size());
  for(int i=0;i<_broadcast_cards.size();i++){
	auto& card = _stats_all_cards.at(i);
	card.rx_rssi=data.rssiPerCard.at(i).getAverage();
	card.exists_in_openhd= true;
  }
  // other stuff is per stream / accumulated
  uint64_t count_p_all=0;
  uint64_t count_p_bad_all=0;
  for(int i=0;i<3;i++){
	count_p_all+=_last_stats_per_stream.at(i).count_p_all;
	count_p_bad_all+=_last_stats_per_stream.at(i).count_p_bad;
  }
  _stats_all_streams.count_p_all=count_p_all;
  _stats_all_streams.count_p_bad_all=count_p_bad_all;
  if(_stats_callback){
	_stats_callback({_stats_all_streams,_stats_all_cards});
  }
}

openhd::link_statistics::StatsTotalRxStreams WBStreams::get_stats_all_rx_streams() {
  std::lock_guard<std::mutex> guard(_statisticsDataLock);
  return _stats_all_streams;
}

openhd::link_statistics::StatsAllCards WBStreams::get_stats_all_cards() {
  std::lock_guard<std::mutex> guard(_statisticsDataLock);
  return _stats_all_cards;
}