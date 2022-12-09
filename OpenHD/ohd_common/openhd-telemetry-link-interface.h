//
// Created by consti10 on 07.12.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_LINK_INTERFACE_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_LINK_INTERFACE_H_

#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace openhd{

// Telemetry communication between air and ground is lossy, but bidirectional
// e.g both air and ground send and receive data (over wifibroadcast or perhaps something else in the future)
// Re-transmissions are done by mavlink, not the link itself
class ITransmitReceiveTelemetry{
  // Telemetry, bidirectional (receive and transmit each)
  // valid on both air and ground instance
  // send telemetry data to the ground if air unit and vice versa.
  //virtual void transmit_telemetry_data(std::shared_ptr<std::vector<uint8_t>> data)=0;

  // called every time telemetry data is received
  // valid on both air and ground instance
  // (on the air,the message is coming from the ground unit, on the ground, it is coming from the air unit
  typedef std::function<void(std::shared_ptr<std::vector<uint8_t>> data)> ON_DATA_CALLBACK;

 public:
  void register_on_receive_callback(ON_DATA_CALLBACK cb){
    std::lock_guard<std::mutex> guard(m_data_cb_mutex);
    m_cb_on_receive =std::move(cb);
  }

  void forward_to_on_receive_cb_if_set(std::shared_ptr<std::vector<uint8_t>> data){
    std::lock_guard<std::mutex> guard(m_data_cb_mutex);
    if(m_cb_on_receive){
      m_cb_on_receive(std::move(data));
    }
  }
  void register_on_send_data_cb(ON_DATA_CALLBACK cb){
    std::lock_guard<std::mutex> guard(m_data_cb_mutex);
    m_cb_on_send =std::move(cb);
  }
  void forward_to_send_data_cb(std::shared_ptr<std::vector<uint8_t>> data){
    std::lock_guard<std::mutex> guard(m_data_cb_mutex);
    //openhd::log::get_default()->debug("forward_to_send_data_cb {}",data->size());
    if(m_cb_on_send){
      m_cb_on_send(std::move(data));
    }
  }
 private:
  std::mutex m_data_cb_mutex;
  ON_DATA_CALLBACK m_cb_on_receive = nullptr;
  ON_DATA_CALLBACK m_cb_on_send=nullptr;
};
}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_TELEMETRY_LINK_INTERFACE_H_
