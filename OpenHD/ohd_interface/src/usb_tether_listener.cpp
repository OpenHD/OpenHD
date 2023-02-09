//
// Created by consti10 on 21.05.22.
//

#include "usb_tether_listener.h"

#include <arpa/inet.h>

#include <utility>

#include "openhd_spdlog.hpp"

USBTetherListener::USBTetherListener(std::shared_ptr<openhd::ExternalDeviceManager> external_device_manager) :
    m_external_device_manager(std::move(external_device_manager)){
  m_console = openhd::log::create_or_get("usb_listener");
  assert(m_console);
  m_check_connection_thread_stop =false;
  m_check_connection_thread =std::make_unique<std::thread>([this](){loopInfinite();});
}

USBTetherListener::~USBTetherListener() {
  m_check_connection_thread_stop =true;
  if(m_check_connection_thread->joinable()){
    m_check_connection_thread->join();
  }
  m_check_connection_thread.reset();
}

void USBTetherListener::loopInfinite() {
  while (!m_check_connection_thread_stop){
    connectOnce();
  }
}

void USBTetherListener::connectOnce() {
  const char* connectedDevice="/sys/class/net/usb0";
  // in regular intervals, check if the device becomes available - if yes, the user connected an ethernet hotspot device.
  while (!m_check_connection_thread_stop){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if(OHDFilesystemUtil::exists(connectedDevice)) {
      openhd::log::get_default()->info("Found USB tethering device");
      break;
    }
  }
  // now we find the IP of the connected device so we can forward video and more to it.
  // example on my Ubuntu pc:
  // ip route list dev usb0
  // default via 192.168.18.229 proto dhcp metric 101
  // 192.168.18.0/24 proto kernel scope link src 192.168.18.155 metric 101
  const auto run_command_result_opt=OHDUtil::run_command_out("ip route list dev usb0");
  if(run_command_result_opt==std::nullopt){
    m_console->warn("run command out no result");
    return;
  }
  const auto& run_command_result=run_command_result_opt.value();
  const auto ip_external_device= OHDUtil::string_in_between("default via "," proto",run_command_result);
  const auto ip_self_network= OHDUtil::string_in_between("src "," metric",run_command_result);

  const auto external_device=openhd::ExternalDevice{"USB0",ip_external_device};
  // Check if both are valid IPs (otherwise, perhaps the parsing got fucked up)
  if(!external_device.is_valid()){
    m_console->warn("{} not valid",external_device.to_string());
    return;
  }
  m_console->info("found device:{}",external_device.to_string());
  if(m_external_device_manager){
    m_external_device_manager->on_new_external_device(external_device, true);
  }
  // check in regular intervals if the tethering device disconnects.
  while (!m_check_connection_thread_stop){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if(!OHDFilesystemUtil::exists(connectedDevice)){
      m_console->warn("USB Tether device disconnected");
      break;
    }
  }
  if(m_external_device_manager){
    m_external_device_manager->on_new_external_device(external_device, false);
  }
}


