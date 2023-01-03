//
// Created by consti10 on 04.01.23.
//

#include "ohd_video_ground.h"

#include <utility>

OHDVideoGround::OHDVideoGround(std::shared_ptr<OHDLink> link_handle):
m_link_handle(std::move(link_handle)){
  udpMultiForwarder = std::make_unique<SocketHelper::UDPMultiForwarder>();
  addForwarder("127.0.0.1",5600);
  m_link_handle->register_on_receive_video_data_cb([this](int stream_index,const uint8_t * data,int data_len){
    on_video_data(stream_index,data,data_len);
  });
}

OHDVideoGround::~OHDVideoGround() {
  m_link_handle->register_on_receive_video_data_cb(nullptr);
}

void OHDVideoGround::addForwarder(std::string client_addr,
                                  int client_udp_port) {
  udpMultiForwarder->addForwarder(client_addr, client_udp_port);
}

void OHDVideoGround::removeForwarder(std::string client_addr,
                                     int client_udp_port) {
  udpMultiForwarder->removeForwarder(client_addr, client_udp_port);
}

void OHDVideoGround::on_video_data(int stream_index, const uint8_t *data,
                                   int data_len) {
  udpMultiForwarder->forwardPacketViaUDP(data,data_len);
}