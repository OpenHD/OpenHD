#ifndef WIFI_H
#define WIFI_H

#include <array>
#include <chrono>
#include <vector>

#include "DPlatform.h"

#include "json.hpp"

#include "openhd-wifi.hpp"
#include "openhd-platform.hpp"

/**
 * Discover all connected wifi cards and write them to json.
 */
class DWifiCards : public OHD::IDiscoverable {
 public:
  DWifiCards(PlatformType platform_type, BoardType board_type);
  virtual ~DWifiCards() = default;
  void discover() override;
  void write_manifest() override;

  void process_card(const std::string &interface_name);
 private:
  std::vector<WiFiCard> m_wifi_cards;
  const PlatformType m_platform_type;
  const BoardType m_board_type;
};

#endif

