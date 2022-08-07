//
// Created by consti10 on 07.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_IEXTERNALDEVICEIP_H_
#define OPENHD_OPENHD_OHD_COMMON_IEXTERNALDEVICEIP_H_

#include <string>
#include "openhd-util.hpp"

namespace openhd {

// This is how we handle "external" devices in regards to forwarding.
// 1) What are external device(s): A smartphone, tablet or similar connected via (WIFI,USB Tethering) to
//    the ground unit
// 2) How are these device(s) handled: Once detected (e.g. when a USB tethering connection is detected), find their IP adress,
//    then call the callback function with the ip and connected==true when new device connected, as well as connected==false
// when this device disconnects

struct ExternalDevice {
  // TODO I don't know exactly how to call this
  // This is the IP address the network this device is connected to has on the local station
  // E.g. If the external device sends a UDP packet to a specific port to the local (ground) station, it will arrive
  // at local_network_ip::port. This is needed for mavlink, where we need to send data from a "bound" UDP port,
  // such that the GroundControl Application knows where to send data to to be picked up by OpenHD
  std::string local_network_ip = "";
  // This is the IP address of the external device itself, e.g. for a smartphone connected via USB tethering, the IP of the smartphone
  // (In the network where it connects to the grund station)
  std::string external_device_ip = "";
  // True if this device is connected, false otherwise. On a change with connected=true, start forwarding
  // On a change with connected==false, stop forwarding (remove already existing forwarder)
  bool connected = false;
  // returns true if both IP addresses are valid
  bool is_valid() const {
	return OHDUtil::is_valid_ip(local_network_ip) && OHDUtil::is_valid_ip(external_device_ip);
  }
  // For when using a map of external device(s)
  std::string create_identifier() {
	return local_network_ip + "_" + external_device_ip;
  }
};

// connected=true: A new external device uniquely indexed by "IP address" has been detected - start forwarding of video and telemetry data
// connected=false: A connected device uniquely indexed by "IP address" disconnected - stop forwarding of video and telemetry data.
typedef std::function<void(ExternalDevice external_device)> EXTERNAL_DEVICE_CALLBACK;

}
#endif //OPENHD_OPENHD_OHD_COMMON_IEXTERNALDEVICEIP_H_
