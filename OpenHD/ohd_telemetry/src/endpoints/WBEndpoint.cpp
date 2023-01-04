//
// Created by consti10 on 09.12.22.
//

#include "WBEndpoint.h"

#include <utility>

WBEndpoint::WBEndpoint(std::shared_ptr<OHDLink> link,std::string TAG)
    : MEndpoint(std::move(TAG)),
    m_link_handle(std::move(link)){
  //assert(m_tx_rx_handle);
  if(!m_link_handle){
    openhd::log::get_default()->warn("WBEndpoint-tx rx handle is missing (no telemetry connection between air and ground)");
  }else{
    auto cb=[this](std::shared_ptr<std::vector<uint8_t>> data){
      MEndpoint::parseNewData(data->data(),data->size());
    };
    m_link_handle->register_on_receive_telemetry_data_cb(cb);
  }
}

WBEndpoint::~WBEndpoint() {
  if(m_link_handle){
    m_link_handle->register_on_receive_telemetry_data_cb(nullptr);
  }
}

bool WBEndpoint::sendMessagesImpl(const std::vector<MavlinkMessage>& messages) {
  auto message_buffers= pack_messages(messages);
  for(const auto& message_buffer:message_buffers){
    if(m_link_handle){
      auto shared=std::make_shared<std::vector<uint8_t>>(message_buffer);
      m_link_handle->transmit_telemetry_data(shared);
    }
  }
  return true;
}
