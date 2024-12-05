/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef OHD_DISCOVER_WiFI_CARDS
#define OHD_DISCOVER_WiFI_CARDS

#include <array>
#include <chrono>
#include <optional>
#include <vector>

#include "openhd_config.h"
#include "openhd_platform.h"
#include "openhd_profile.h"
#include "wifi_card.h"

/**
 * Discover all connected wifi cards.
 */
namespace DWifiCards {

void main_discover_an_process_wifi_cards(
    const openhd::Config& config, const OHDProfile& m_profile,
    std::shared_ptr<spdlog::logger>& m_console,
    std::vector<WiFiCard>& m_monitor_mode_cards,
    std::optional<WiFiCard>& m_opt_hotspot_card);

// this should never fail (return std::nullopt) if the given interface_name is
// valid
std::optional<WiFiCard> fill_linux_wifi_card_identifiers(
    const std::string& interface_name);

// helper to figure out more info about a semi-discovered wifi card
std::optional<WiFiCard> process_card(const std::string& interface_name);

// discover all connected wifi cards and their capabilities
std::vector<WiFiCard> discover_connected_wifi_cards();

// Calculate how many cards found are openhd wifibroadcast supported
int n_cards_openhd_wifibroadcast_supported(const std::vector<WiFiCard>& cards);

struct ProcessedWifiCards {
  std::vector<WiFiCard> monitor_mode_cards;
  std::optional<WiFiCard> hotspot_card;
};

ProcessedWifiCards process_and_evaluate_cards(
    const std::vector<WiFiCard>& discovered_cards, const OHDProfile& profile);

// for users who use the manual file to define their card(s)
ProcessedWifiCards find_cards_from_manual_file(
    const std::vector<std::string>& wifibroadcast_cards,
    const std::string& opt_hotspot_card);

WiFiCard create_card_monitor_emulate();
};  // namespace DWifiCards

#endif  // OHD_DISCOVER_WiFI_CARDS
