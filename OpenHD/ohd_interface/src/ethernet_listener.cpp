//
// Created by consti10 on 03.01.23.
//

#include "ethernet_listener.h"


EthernetListener::EthernetListener(
    std::shared_ptr<openhd::ExternalDeviceManager> external_device_manager) {
  m_console = openhd::log::create_or_get("eth_listener");
  assert(m_console);
  loopThreadStop=false;
  loopThread=std::make_unique<std::thread>([this](){loop_infinite();});
}


EthernetListener::~EthernetListener() {
  loopThreadStop=true;
  if(loopThread->joinable()){
    loopThread->join();
  }
  loopThread.reset();
}

void EthernetListener::loop_infinite() {
  while (!loopThreadStop){
    connect_once();
  }
}

void EthernetListener::connect_once() {
  while (!loopThreadStop){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const auto content_opt=OHDFilesystemUtil::opt_read_file("/sys/class/net/eth0/operstate");
    if(!content_opt.has_value())continue;
    const auto& content=content_opt.value();
    if(!OHDUtil::contains(content,"up")){
      m_console->debug("Eth0 is up");
      break;
    }
  }
  const auto run_command_result_opt=OHDUtil::run_command_out("ip route list dev eth0");
  if(run_command_result_opt==std::nullopt){
    m_console->warn("run command out no result");
    return;
  }
  const auto& run_command_result=run_command_result_opt.value();
  const auto ip_external_device= OHDUtil::string_in_between("default via "," proto",run_command_result);
  const auto ip_self_network= OHDUtil::string_in_between("src "," metric",run_command_result);

  const auto external_device=openhd::ExternalDevice{ip_self_network,ip_external_device};
  // Check if both are valid IPs (otherwise, perhaps the parsing got fucked up)
  if(!external_device.is_valid()){
    m_console->warn("{} not valid",external_device.to_string());
    return;
  }
  m_console->info("found device:{}",external_device.to_string());
  if(m_external_device_manager){
    m_external_device_manager->on_new_external_device("USB",external_device, true);
  }
  // check in regular intervals if the device disconnects
  while (!loopThreadStop){
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //TODO
  }
  if(m_external_device_manager){
    m_external_device_manager->on_new_external_device("USB",external_device, false);
  }

}
