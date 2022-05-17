//
// Created by consti10 on 17.05.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFIHOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFIHOTSPOT_H_

#include <vector>
#include <string>

/**
 * Wifi hotspot refers to creating a WiFi Access point on the device we are running on.
 * External clients like QOpenHD running on a tablet can then connect to the hotspot.
 * Note that video and telemetry has to be forwarded to clients connected to the wifi hotspot.
 * For that, there is a callback to register by OHDInterface.
 */

class WifiHotspot {
 public:
  WifiHotspot();

  void start();

  void stop();

 private:
  // Ip addresses of all connected clients.
  // A client might dynamically connect or disconnect from the AP at run time,
  // In this case the apropriate callbacks have to be called.
  std::vector<std::string> connectedClientsIps;
};

#endif //OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFIHOTSPOT_H_
