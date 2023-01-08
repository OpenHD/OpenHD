//
// Created by consti10 on 04.01.23.
//

#include "ethernet_hotspot.h"

#include <openhd-external-device.hpp>
#include <openhd-util.hpp>
#include <utility>

static constexpr auto OHD_ETHERNET_HOTSPOT_CONNECTION_NAME ="ohd_eth_hotspot";

static std::string get_ohd_eth_hotspot_connection_nm_filename(){
  return fmt::format("/etc/NetworkManager/system-connections/{}.nmconnection",OHD_ETHERNET_HOTSPOT_CONNECTION_NAME);
}

static void delete_existing_hotspot_connection(const std::string& eth_device_name){
  const auto filename=get_ohd_eth_hotspot_connection_nm_filename();
  if(OHDFilesystemUtil::exists(filename.c_str())){
    OHDUtil::run_command("nmcli",{"con","delete", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME});
  }
}

static void create_ethernet_hotspot_connection(const std::shared_ptr<spdlog::logger>& m_console,const std::string& eth_device_name){
  // sudo nmcli con add type ethernet con-name "ohd_eth_hotspot" ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
  // sudo nmcli con add type ethernet ifname eth0 con-name ohd_eth_hotspot autoconnect no
  // sudo nmcli con modify ohd_eth_hotspot ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
  m_console->debug("begin create hotspot connection");
  // delete any already existing
  delete_existing_hotspot_connection(eth_device_name);
  // then create the new one (it is a cheap operation)- note that the autoconnect is off by purpose
  OHDUtil::run_command("nmcli",{"con add type ethernet ifname",eth_device_name,"con-name", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME,"autoconnect no"});
  OHDUtil::run_command("nmcli",{"con modify", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME,"ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1"});
  m_console->debug("end create hotspot connection");
}

EthernetHotspot::EthernetHotspot(std::shared_ptr<openhd::ExternalDeviceManager> external_device_manager,std::string  device)
    :m_device(std::move(device)),m_external_device_manager(std::move(external_device_manager)) {
  m_console = openhd::log::create_or_get("wifi_hs");
  m_settings=std::make_unique<EthernetHotspotSettingsHolder>();
  if(m_settings->get_settings().enable){
    create_ethernet_hotspot_connection(m_console,m_device);
    start();
    m_check_connection_thread_stop =false;
    m_check_connection_thread =std::make_unique<std::thread>([this](){loop_infinite();});
  }else{
    delete_existing_hotspot_connection(device);
  }
}

EthernetHotspot::~EthernetHotspot() {
  m_check_connection_thread_stop=true;
  if(m_check_connection_thread){
    m_check_connection_thread->join();
  }
  stop();
  delete_existing_hotspot_connection(m_device);
}

void EthernetHotspot::start() {
  m_console->debug("Starting eth hs connection");
  const auto args=std::vector<std::string>{"con","up", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME};
  OHDUtil::run_command("nmcli",args);
  m_console->info("eth hotspot started");
}

void EthernetHotspot::stop() {
  m_console->debug("Stopping eth hotspot");
  const auto args=std::vector<std::string>{"con","down", OHD_ETHERNET_HOTSPOT_CONNECTION_NAME};
  OHDUtil::run_command("nmcli",args);
  m_console->info("eth hotspot stopped");
}

std::vector<openhd::Setting> EthernetHotspot::get_all_settings() {
  using namespace openhd;
  std::vector<openhd::Setting> ret{};
  const auto settings=m_settings->get_settings();
  auto cb_enable=[this](std::string,int value){
    if(!validate_yes_or_no(value))return false;
    m_settings->unsafe_get_settings().enable=value;
    m_settings->persist();
    // to apply, requires reboot !!
    return true;
  };
  ret.push_back(openhd::Setting{"I_ETH_HOTSPOT_E",openhd::IntSetting{settings.enable,cb_enable}});
  return ret;
}

void EthernetHotspot::loop_infinite() {
  while (!m_check_connection_thread_stop){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    connect_once();
  }
}

void EthernetHotspot::connect_once() {
  // Try and find the IP of the device connected via ethernet
  const auto run_command_result_opt=OHDUtil::run_command_out(fmt::format("arp -an -i {} | grep -v incomplete",m_device));
  if(run_command_result_opt==std::nullopt){
    m_console->warn("run command out no result");
    return;
  }
  const auto& run_command_result=run_command_result_opt.value();
  // valid ip should look something like that:
  // ? (192.168.2.158) at e0:d5:5e:e1:19:45 [ether] on eth0
  const auto ip_external_device=OHDUtil::string_in_between("(",")",run_command_result);
  if(!OHDUtil::is_valid_ip(ip_external_device)){
    m_console->warn("{} is not a valid ip",ip_external_device);
    return;
  }
  // When we reach here we have a valid ip of the device connected - now check if it disconnects
  const auto external_device=openhd::ExternalDevice{"ETH0","127.0.0.1",ip_external_device};
  m_console->info("found device:{}",external_device.to_string());
  if(m_external_device_manager){
    m_external_device_manager->on_new_external_device(external_device, true);
  }
  // now check in regular intervals if the device disconnects
  while (!m_check_connection_thread_stop){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const auto tmp=OHDUtil::run_command_out(fmt::format("arp -an -i {} | grep -v incomplete",m_device));
    if(!OHDUtil::contains(tmp.value_or(""),ip_external_device)){
      // disconnected
      break;
    }
  }
  m_console->info("disconnected {}",external_device.to_string());
  if(m_external_device_manager){
    m_external_device_manager->on_new_external_device(external_device, false);
  }
}
