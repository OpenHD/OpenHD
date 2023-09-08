//
// Created by consti10 on 03.01.23.
//

#include "ethernet_listener.h"

#include <utility>

#include "ethernet_helper.hpp"

EthernetListener::EthernetListener(
    std::shared_ptr<openhd::ExternalDeviceManager> external_device_manager,std::string device):
m_external_device_manager(std::move(external_device_manager)),
m_device(std::move(device)){
  m_console = openhd::log::create_or_get("eth_listener");
  assert(m_console);
  m_console->debug("device:[{}]",m_device);
}


EthernetListener::~EthernetListener() {
  // Terminate properly before destruction if needed.
  stop();
}

void EthernetListener::start() {
  std::lock_guard<std::mutex> guard(m_enable_disable_mutex);
  if(m_check_connection_thread!= nullptr){
    m_console->warn("Already running");
    return ;
  }
  m_check_connection_thread_stop =false;
  m_check_connection_thread =std::make_unique<std::thread>([this](){loop_infinite();});
}

void EthernetListener::stop(){
  std::lock_guard<std::mutex> guard(m_enable_disable_mutex);
  if(m_check_connection_thread== nullptr){
    m_console->warn("Already disabled");
    return ;
  }
  m_check_connection_thread_stop =true;
  if(m_check_connection_thread->joinable()){
    m_check_connection_thread->join();
  }
  m_check_connection_thread= nullptr;
}

void EthernetListener::set_enabled(bool enable) {
  if(enable){
    start();
  }else{
    stop();
  }
}

void EthernetListener::loop_infinite() {
  while (!m_check_connection_thread_stop){
    connect_once();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void EthernetListener::connect_once() {
  while (!m_check_connection_thread_stop){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if(openhd::ethernet::check_eth_adapter_up("eth0")){
      m_console->debug("Eth0 is up");
      break;
    }
  }
  const auto run_command_result_opt=OHDUtil::run_command_out(fmt::format("ip route list dev {}",m_device));
  if(run_command_result_opt==std::nullopt){
    m_console->warn("run command out no result");
    return;
  }
  const auto& run_command_result=run_command_result_opt.value();
  const auto ip_external_device= OHDUtil::string_in_between("default via "," proto",run_command_result);
  //const auto ip_self_network= OHDUtil::string_in_between("src "," metric",run_command_result);

  const auto external_device=openhd::ExternalDevice{"ETH0",ip_external_device};
  // Check if both are valid IPs (otherwise, perhaps the parsing got fucked up)
  if(!external_device.is_valid()){
    m_console->warn("{} not valid",external_device.to_string());
    return;
  }
  m_console->info("found device:{}",external_device.to_string());
  if(m_external_device_manager){
    m_external_device_manager->on_new_external_device(external_device, true);
  }
  // check in regular intervals if the device disconnects
  while (!m_check_connection_thread_stop){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //TODO
  }
  if(m_external_device_manager){
    m_external_device_manager->on_new_external_device(external_device, false);
  }
}
