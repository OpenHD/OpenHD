//
// Created by consti10 on 07.08.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_EXTERNAL_DEVICE_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_EXTERNAL_DEVICE_H_

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace openhd {

// This is how we handle "external" devices in regard to forwarding.
// 1) What are external device(s): A smartphone, tablet or similar connected via
// (WIFI,USB Tethering) to
//    the ground unit
// 2) How are these device(s) handled: Once detected (e.g. when a USB tethering
// connection is detected), find their IP adress,
//    then call the callback function with the ip and connected==true when new
//    device connected, as well as connected==false when this device disconnects
// 3) How are these device(s) detected: Right now in oh_interface by usb,
// ethernet listeners and in ohd_telemetry by the mavlink
//    tcp endpoint.

struct ExternalDevice {
  // for debugging purposes only
  std::string tag;
  // TODO I don't know exactly how to call this
  // This is the IP address the network this device is connected to has on the
  // local station E.g. If the external device sends a UDP packet to a specific
  // port to the local (ground) station, it will arrive at
  // local_network_ip::port. This is needed for mavlink, where we need to send
  // data from a "bound" UDP port, such that the GroundControl Application knows
  // where to send data to to be picked up by OpenHD
  // std::string local_network_ip;
  // This is the IP address of the external device itself, e.g. for a smartphone
  // connected via USB tethering, the IP of the smartphone (In the network where
  // it connects to the grund station)
  std::string external_device_ip;
  // Set to true if this device was discovered by someone connecting to the
  // mavlink TCP server - We have slightly different forwarding behaviour in
  // this case
  bool discovered_by_mavlink_tcp_server = false;
  // returns true if both IP addresses are valid
  [[nodiscard]] bool is_valid() const;
  // For when using a map of external device(s)
  [[nodiscard]] std::string create_identifier() const;
  [[nodiscard]] std::string to_string() const;
};

// connected=true: A new external device uniquely indexed by "IP address" has
// been detected - start forwarding of ohd video and telemetry data
// connected=false: A connected device uniquely indexed by "IP address"
// disconnected - stop forwarding of ohd video and telemetry data.
typedef std::function<void(ExternalDevice external_device, bool connected)>
    EXTERNAL_DEVICE_CALLBACK;

class ExternalDeviceManager {
 public:
  ExternalDeviceManager();
  ~ExternalDeviceManager();
  // We only have one global instance to be accessed by all opehd modules (core,
  // telemetry,...)
  static ExternalDeviceManager& instance();
  /**
   * Thread-safe
   * Called every time someone (for example the usb tether listener) discovered
   * a new external device or detected that an external device has disconnected.
   */
  void on_new_external_device(const ExternalDevice& external_device,
                              bool connected);
  /**
   * Thread-safe
   * Register a listener that is called every time an external device is
   * discovered or lost. If devices are discovered before the cb is registered,
   * the cb is called for all the already connected devices.
   */
  void register_listener(EXTERNAL_DEVICE_CALLBACK cb);
  // This only exists to terminate openhd properly (which only happens in a test
  // environment)
  void remove_all();
  // Lock-free
  // returns the n of connected 'external devices'
  uint8_t get_external_device_count();

 private:
  std::mutex m_ext_devices_lock;
  std::map<std::string, ExternalDevice> m_curr_ext_devices;
  std::vector<EXTERNAL_DEVICE_CALLBACK> m_callbacks;
  std::vector<std::string> m_manual_ips;
  bool m_remove_all_called = false;
  std::atomic_uint8_t m_external_device_count = 0;
};

}  // namespace openhd
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_EXTERNAL_DEVICE_H_
