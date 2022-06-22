#ifndef WIFI_H
#define WIFI_H

#include <array>
#include <chrono>
#include <vector>


#include "openhd-wifi.hpp"
#include "openhd-profile.hpp"

/**
 * Provides access to the discovered wifi cards on the system.
 * There should only be one instance of this class in the whole OpenHD project.
 */
class WifiCards {
 public:
  explicit WifiCards(const OHDProfile &profile);
  virtual ~WifiCards() = default;
  void configure();
  void setup_hotspot(const WiFiCard &card);
  // Verbose string about the current state.
  [[nodiscard]] std::string createDebug() const;
  /**
   * Get the names for the broadcast cards so we can start the wfb_tx/rx instances.
   * @return a list of the names (as can be used for wifibroadcast) of all cards currently in monitor mode.
   */
  [[nodiscard]] std::vector<std::string> get_broadcast_card_names() const;
  static bool set_card_state(const WiFiCard &card, bool up);
  static bool set_frequency(const WiFiCard &card, const std::string &frequency);
  static bool set_txpower(const WiFiCard &card, const std::string &txpower);
  static bool enable_monitor_mode(const WiFiCard &card);
  static void save_settings(const std::vector<WiFiCard> &cards, const std::string &settings_file);
 private:
  // Setup the wifi card for its intended use case.
  // Note that his method does not modify any member of the WifiCard class,
  // Since after it has been executed the actual state of the wifi card should match
  // the specified state of WifiCard by its struct members.
  void setup_card(const WiFiCard &card);
 private:
  const OHDProfile &profile;
  std::vector<WiFiCard> m_wifi_cards;
  // todo: read from settings file once new settings system merged
  static constexpr auto m_default_5ghz_frequency = "5180";
  static constexpr auto m_default_2ghz_frequency = "2412";
};

#endif

