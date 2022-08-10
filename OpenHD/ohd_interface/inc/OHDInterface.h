//
// Created by consti10 on 02.05.22.
//

#ifndef OPENHD_OPENHD_INTERFACE_H
#define OPENHD_OPENHD_INTERFACE_H

#include <memory>
#include <utility>

#include "USBTetherListener.h"
#include "WBStreams.h"
#include "WifiHotspot.h"
#include "openhd-profile.hpp"
#include "openhd-platform.hpp"
#include "openhd-led-error-codes.h"
#include "mavlink_settings/ISettingsComponent.h"
#include "OHDInterfaceSettings.h"
#include "openhd-external-device.h"

 class OHDInterface :public openhd::ISettingsComponent{
 public:
  /**
   * Takes care of everything networking related, like wifibroadcast, usb / tethering / WiFi-hotspot usw.
   */
  explicit OHDInterface(OHDPlatform platform1,OHDProfile profile1);
   OHDInterface(const OHDInterface&)=delete;
   OHDInterface(const OHDInterface&&)=delete;
  // register callback that is called in regular intervals with link statistics
  void set_stats_callback(openhd::link_statistics::STATS_CALLBACK stats_callback) const;
  // Verbose string about the current state.
  [[nodiscard]] std::string createDebug() const;
  // hacky, temporary. applies changed frequency / mcs index / bandwidth
  void restart_wb_streams_async();
  // For telemetry
  void set_external_device_callback(openhd::EXTERNAL_DEVICE_CALLBACK cb);
  // settings hacky begin
  std::vector<openhd::Setting> get_all_settings()override;
  // settings hacky end
  // easy access without polluting the headers
  static void print_internal_fec_optimization_method();
  private:
   /**
	* after calling this method with a external device's ip address
	* (for example an externally connected tablet) data will be forwarded to the device's ip address.
	* It is safe to call this method multiple times with the same IP address, since we internally keep track here.
	*/
   void addExternalDeviceIpForwarding(const openhd::ExternalDevice& external_device);
   /**
	* stop forwarding data to the device's ip address.
	* Does nothing if the device's ip address is not registered for forwarding or already has ben removed.
	*/
   void removeExternalDeviceIpForwarding(const openhd::ExternalDevice& external_device);
 private:
  const OHDProfile profile;
  const OHDPlatform platform;
  std::unique_ptr<WBStreams> wbStreams;
  std::unique_ptr<USBTetherListener> usbTetherListener;
  std::unique_ptr<WifiHotspot> _wifi_hotspot;
  std::unique_ptr<openhd::rpi::LEDBlinker> _error_blinker;
  std::shared_ptr<openhd::OHDInterfaceSettingsHolder> _interface_settings_holder;
  std::mutex _external_device_callback_mutex;
  openhd::EXTERNAL_DEVICE_CALLBACK _external_device_callback= nullptr;
};

#endif //OPENHD_OPENHD_INTERFACE_H
