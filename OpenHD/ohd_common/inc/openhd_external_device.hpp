//
// Created by consti10 on 07.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_EXTERNAL_DEVICE_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_EXTERNAL_DEVICE_H_

#include <map>
#include <string>

#include "openhd_settings_directories.hpp"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_config.h"

namespace openhd {

// This is how we handle "external" devices in regard to forwarding.
// 1) What are external device(s): A smartphone, tablet or similar connected via (WIFI,USB Tethering) to
//    the ground unit
// 2) How are these device(s) handled: Once detected (e.g. when a USB tethering connection is detected), find their IP adress,
//    then call the callback function with the ip and connected==true when new device connected, as well as connected==false
//    when this device disconnects

struct ExternalDevice {
  // for debugging purposes only
  std::string tag;
  // TODO I don't know exactly how to call this
  // This is the IP address the network this device is connected to has on the local station
  // E.g. If the external device sends a UDP packet to a specific port to the local (ground) station, it will arrive
  // at local_network_ip::port. This is needed for mavlink, where we need to send data from a "bound" UDP port,
  // such that the GroundControl Application knows where to send data to to be picked up by OpenHD
  //std::string local_network_ip;
  // This is the IP address of the external device itself, e.g. for a smartphone connected via USB tethering, the IP of the smartphone
  // (In the network where it connects to the grund station)
  std::string external_device_ip;
  // Set to true if this device was discovered by someone connecting to the mavlink TCP server -
  // We have slightly different forwarding behaviour in this case
  bool discovered_by_mavlink_tcp_server= false;
  // returns true if both IP addresses are valid
  [[nodiscard]] bool is_valid() const {
    //return OHDUtil::is_valid_ip(local_network_ip) && OHDUtil::is_valid_ip(external_device_ip);
    return OHDUtil::is_valid_ip(external_device_ip);
  }
  // For when using a map of external device(s)
  [[nodiscard]] std::string create_identifier() const {
    assert(is_valid());
    //return local_network_ip + "_" + external_device_ip;
    return external_device_ip;
  }
  [[nodiscard]] std::string to_string()const{
    //return fmt::format("ExternalDevice {} [local:[{}] remote:[{}]]",tag,local_network_ip,external_device_ip);
    return fmt::format("ExternalDevice {} remote:[{}]]",tag,external_device_ip);
  }
};

// connected=true: A new external device uniquely indexed by "IP address" has been detected - start forwarding of ohd video and telemetry data
// connected=false: A connected device uniquely indexed by "IP address" disconnected - stop forwarding of ohd video and telemetry data.
typedef std::function<void(ExternalDevice external_device,bool connected)> EXTERNAL_DEVICE_CALLBACK;

class ExternalDeviceManager{
 public:
  ExternalDeviceManager(){
    // Here one can manually declare any IP addresses openhd should forward video / telemetry to
    const auto config=openhd::load_config();
    for(const auto& ip:config.NW_MANUAL_FORWARDING_IPS){
      if(OHDUtil::is_valid_ip(ip)){
        m_manual_ips.push_back(ip);
      }else{
        openhd::log::get_default()->warn("[{}] is not a valid ip",ip);
      }
    }
    for(const auto& ip:m_manual_ips){
      on_new_external_device(ExternalDevice{"manual",ip}, true);
    }
  }
  ~ExternalDeviceManager(){
    for(const auto& ip:m_manual_ips){
      on_new_external_device(ExternalDevice{"manual",ip}, false);
    }
  }
  // Might be called from different thread(s)
  void on_new_external_device(const ExternalDevice& external_device,bool connected){
    openhd::log::get_default()->debug("Got {} {}",external_device.to_string(),connected);
    const auto id=external_device.create_identifier();
    std::lock_guard<std::mutex> guard(m_ext_devices_lock);
    if(connected){
      if(m_curr_ext_devices.find(id)!=m_curr_ext_devices.end()){
        openhd::log::get_default()->warn("Device {} already exists",external_device.to_string());
        return;
      }
      // New external device connected
      // log such that the message is shown in QOpenHD
      openhd::log::log_via_mavlink(5,"External device connected");
      m_curr_ext_devices[id]=external_device;
      for(auto& cb:m_callbacks){
        cb(external_device, true);
      }
    }else{
      if(m_curr_ext_devices.find(id)==m_curr_ext_devices.end()){
        openhd::log::get_default()->warn("Device {} does not exist",external_device.to_string());
        return;
      }
      // warning in QOpenHD
      openhd::log::get_default()->warn("External device disconnected");
      // existing external device disconnected
      m_curr_ext_devices.erase(id);
      for(auto& cb:m_callbacks){
        cb(external_device, false);
      }
    }
  }
  void register_listener(EXTERNAL_DEVICE_CALLBACK cb){
    std::lock_guard<std::mutex> guard(m_ext_devices_lock);
    // Notify the callback to register of any already connected devices
    for(auto& [id,device]: m_curr_ext_devices){
      cb(device,true);
    }
    m_callbacks.push_back(cb);
  }
  /**
    * after calling this method with an external device's ip address
    * (for example an externally connected tablet) data will be forwarded to the device's ip address.
    * It is safe to call this method multiple times with the same IP address, since we internally keep track here.
   */
  /**
    * stop forwarding data to the device's ip address.
    * Does nothing if the device's ip address is not registered for forwarding or already has ben removed.
   */
 private:
  std::mutex m_ext_devices_lock;
  std::map<std::string,ExternalDevice> m_curr_ext_devices;
  std::vector<EXTERNAL_DEVICE_CALLBACK> m_callbacks;
  std::vector<std::string> m_manual_ips;
};

}
#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_EXTERNAL_DEVICE_H_
