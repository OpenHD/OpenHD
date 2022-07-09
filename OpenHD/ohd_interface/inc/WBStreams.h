#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <vector>
#include <utility>

#include "openhd-wifi.hpp"
#include "openhd-profile.hpp"

#include "../../lib/wifibroadcast/src/UDPWfibroadcastWrapper.hpp"

/**
 * This class takes a list of discovered wifi cards (and their settings9
 * This class is responsible for setting up all the Wifibroadcast streams needed for OpenHD.
 * There should only be one instance of this class in the whole project.
 */
class WBStreams {
 public:
  explicit WBStreams(OHDProfile profile,std::vector<std::shared_ptr<WifiCardHolder>> broadcast_cards);

  // Verbose string about the current state.
  [[nodiscard]] std::string createDebug() const;
  // see interface
  void addExternalDeviceIpForwarding(const std::string& ip);
  void removeExternalDeviceIpForwarding(const std::string& ip);
 private:
  const OHDProfile _profile;
  const int DEFAULT_MCS_INDEX = 3;
  std::vector<std::shared_ptr<WifiCardHolder>> _broadcast_cards;
 private:
  void configure();
  void configure_telemetry();
  void configure_video();
  // For telemetry, bidirectional in opposite directions
  std::unique_ptr<UDPWBTransmitter> udpTelemetryTx;
  std::unique_ptr<UDPWBReceiver> udpTelemetryRx;
  // For video, on air there are only tx instances, on ground there are only rx instances.
  std::vector<std::unique_ptr<UDPWBTransmitter>> udpVideoTxList;
  std::vector<std::unique_ptr<UDPWBReceiver>> udpVideoRxList;
  // TODO make more configurable
  [[nodiscard]] std::unique_ptr<UDPWBTransmitter> createUdpWbTx(uint8_t radio_port, int udp_port,bool enableFec)const;
  [[nodiscard]] std::unique_ptr<UDPWBReceiver> createUdpWbRx(uint8_t radio_port, int udp_port) const;
  [[nodiscard]] std::vector<std::string> get_rx_card_names()const;
};

#endif
