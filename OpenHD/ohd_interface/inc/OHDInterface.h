//
// Created by consti10 on 02.05.22.
//

#ifndef OPENHD_OPENHD_INTERFACE_H
#define OPENHD_OPENHD_INTERFACE_H

#include <memory>

#include "WBStreams.h"
#include "WifiCards.h"
#include "openhd-profile.hpp"
#include "USBTetherListener.h"

class OHDInterface {
 public:
  /**
   * Note: due to json, needs to be crated after OHDSystem has been run once.
   * The constructor reads the generated json by OHDSystem and tries to initialize all its members.
   * NOTE: We need to wire link - related settings into here.
   * For example, changing the wifi frequency (needs to be synced between air and ground) but also
   * settings that can be unsinced (like enable / disable wifi hotspot, which - as an example - is only
   * a setting that affects the ground pi.
   * @param profile the (never-changing) profile we are running with.
   */
  explicit OHDInterface(const OHDProfile &profile);
  std::unique_ptr<WifiCards> wifiCards;
  std::unique_ptr<WBStreams> wbStreams;
  std::unique_ptr<USBTetherListener> usbTetherListener;
  // Verbose string about the current state.
  [[nodiscard]] std::string createDebug() const;
  /**
   * after calling this method with a external device's ip address
   * (for example an externally connected tablet) data will be forwarded to the device's ip address.
   * It is safe to call this method multiple times with the same IP address, since we internally keep track here.
   */
  void addExternalDeviceIpForwarding(std::string ip);
  /**
   * stop forwarding data to the device's ip address.
   * Does nothing if the device's ip address is not registered for forwarding or already has ben removed.
   */
  void removeExternalDeviceIpForwarding(std::string ip);
 private:
  const OHDProfile &profile;
};

#endif //OPENHD_OPENHD_INTERFACE_H
