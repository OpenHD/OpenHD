//
// Created by consti10 on 04.01.23.
//

#include "ohd_video_ground.h"

#include <utility>

OHDVideoGround::OHDVideoGround(std::shared_ptr<OHDLink> link_handle):
m_link_handle(std::move(link_handle)){
  m_console = openhd::log::create_or_get("v_gnd");
  m_primary_video_forwarder = std::make_unique<SocketHelper::UDPMultiForwarder>();
  m_secondary_video_forwarder = std::make_unique<SocketHelper::UDPMultiForwarder>();
  // We always forward video to localhost::5600 (primary) and 5601 (secondary) for the default Ground control application (e.g. QOpenHD) to pick up
  addForwarder("127.0.0.1");

  // Adding forwarder for WebRTC
  m_primary_video_forwarder->addForwarder("127.0.0.1", 5800);

  if(m_link_handle){
    m_link_handle->register_on_receive_video_data_cb([this](int stream_index,const uint8_t * data,int data_len){
      on_video_data(stream_index,data,data_len);
    });
  }else{
    m_console->warn("No link handle, no video forwarding");
  }
}

OHDVideoGround::~OHDVideoGround() {
  if(m_link_handle){
    m_link_handle->register_on_receive_video_data_cb(nullptr);
  }
}

void OHDVideoGround::addForwarder(const std::string& client_addr) {
  m_primary_video_forwarder->addForwarder(client_addr,5600);
  m_secondary_video_forwarder->addForwarder(client_addr,5601);
}

void OHDVideoGround::removeForwarder(const std::string& client_addr) {
  m_primary_video_forwarder->removeForwarder(client_addr,5600);
  m_secondary_video_forwarder->removeForwarder(client_addr,5601);
}

void OHDVideoGround::on_video_data(int stream_index, const uint8_t *data,
                                   int data_len) {
  if(stream_index==0){
    m_primary_video_forwarder->forwardPacketViaUDP(data,data_len);
  }else if(stream_index==1){
    m_secondary_video_forwarder->forwardPacketViaUDP(data,data_len);
  }else{
    openhd::log::get_default()->debug("Invalid stream index {}",stream_index);
  }
}

void OHDVideoGround::set_ext_devices_manager(std::shared_ptr<openhd::ExternalDeviceManager> ext_device_manager) {
  ext_device_manager->register_listener([this](openhd::ExternalDevice external_device,bool connected){
    if(connected){
      addForwarder(external_device.external_device_ip);
    }else{
      removeForwarder(external_device.external_device_ip);
    }
  });
}
