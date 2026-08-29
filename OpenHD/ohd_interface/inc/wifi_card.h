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

#ifndef OPENHD_WIFI_H
#define OPENHD_WIFI_H

#include <fstream>
#include <optional>
#include <string>

#include "openhd_platform.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "validate_settings_helper.h"
#include "wifi_channel.h"

// After discovery, the capabilities of a WiFi-Card are immutable !

// Telemetry ABI: never renumber an existing value. QOpenHD mirrors this table.
enum class WiFiCardType : int {
  OPENHD_RTL_88X2AU = 0,  // Supported
  OPENHD_RTL_88X2BU = 1,  // Supported
  OPENHD_RTL_88X2CU = 2,  // Supported
  OPENHD_RTL_88X2EU = 3,  // Supported
  // These are all unsupported, but might / might not work
  RTL_88X2AU = 4,  // right card, but wrong driver
  RTL_88X2BU = 5,  // right card, but wrong driver
  ATHEROS = 6,     // not supported anymore
  MT_7921u = 7,    // TODO add support
  RALINK = 8,
  INTEL = 9,
  BROADCOM = 10,
  OPENHD_RTL_8852BU = 11,  // testing phase
  OPENHD_EMULATED = 12,
  AIC = 13,
  QUALCOMM = 14,
  UNKNOWN = 15,
  // Non-monitor-mode Artosyn link pseudo card type for telemetry/UI display.
  ARTOSYN = 16,
  // Exact silicon identities selected for the Devourer userspace backend.
  DEVOURER_RTL8812A = 17,
  DEVOURER_RTL8821A = 18,
  DEVOURER_RTL8814A = 19,
  DEVOURER_RTL8821C = 20,
  DEVOURER_RTL8822B = 21,
  DEVOURER_RTL8822C = 22,
  DEVOURER_RTL8822E = 23,
  DEVOURER_RTL8733B = 24,
  DEVOURER_RTL8852B = 25,
  DEVOURER_RTL8852C = 26,
  DEVOURER_RTL8811A = 27
};

static_assert(static_cast<int>(WiFiCardType::OPENHD_RTL_88X2AU) == 0);
static_assert(static_cast<int>(WiFiCardType::ARTOSYN) == 16);
static_assert(static_cast<int>(WiFiCardType::DEVOURER_RTL8812A) == 17);
static_assert(static_cast<int>(WiFiCardType::DEVOURER_RTL8811A) == 27);

static constexpr uint8_t WIFI_CARD_SUB_TYPE_UNKNOWN = 0;
static constexpr uint8_t WIFI_CARD_SUB_TYPE_RTL8812AU_ASUS = 1;
static constexpr uint8_t WIFI_CARD_SUB_TYPE_RTL8812AU_X20 = 2;

static std::string wifi_card_type_to_string(const WiFiCardType& card_type) {
  switch (card_type) {
    case WiFiCardType::OPENHD_RTL_88X2AU:
      return "OPENHD_RTL_88X2AU";
    case WiFiCardType::OPENHD_RTL_88X2BU:
      return "OPENHD_RTL_88X2BU";
    case WiFiCardType::OPENHD_RTL_88X2CU:
      return "OPENHD_RTL_88X2CU";
    case WiFiCardType::OPENHD_RTL_88X2EU:
      return "OPENHD_RTL_88X2EU";
    case WiFiCardType::OPENHD_RTL_8852BU:
      return "OPENHD_RTL_8852BU";
    case WiFiCardType::RTL_88X2AU:
      return "RTL_88X2AU";
    case WiFiCardType::RTL_88X2BU:
      return "RTL_88X2BU";
    case WiFiCardType::ATHEROS:
      return "ATHEROS";
    case WiFiCardType::MT_7921u:
      return "MT_7921u";
    case WiFiCardType::RALINK:
      return "RALINK";
    case WiFiCardType::INTEL:
      return "INTEL";
    case WiFiCardType::BROADCOM:
      return "BROADCOM";
    case WiFiCardType::AIC:
      return "AIC";
    case WiFiCardType::QUALCOMM:
      return "QUALCOMM";
    case WiFiCardType::ARTOSYN:
      return "ARTOSYN";
    case WiFiCardType::DEVOURER_RTL8812A:
      return "DEVOURER_RTL8812A";
    case WiFiCardType::DEVOURER_RTL8821A:
      return "DEVOURER_RTL8821A";
    case WiFiCardType::DEVOURER_RTL8814A:
      return "DEVOURER_RTL8814A";
    case WiFiCardType::DEVOURER_RTL8821C:
      return "DEVOURER_RTL8821C";
    case WiFiCardType::DEVOURER_RTL8822B:
      return "DEVOURER_RTL8822B";
    case WiFiCardType::DEVOURER_RTL8822C:
      return "DEVOURER_RTL8822C";
    case WiFiCardType::DEVOURER_RTL8822E:
      return "DEVOURER_RTL8822E";
    case WiFiCardType::DEVOURER_RTL8733B:
      return "DEVOURER_RTL8733B";
    case WiFiCardType::DEVOURER_RTL8852B:
      return "DEVOURER_RTL8852B";
    case WiFiCardType::DEVOURER_RTL8852C:
      return "DEVOURER_RTL8852C";
    case WiFiCardType::DEVOURER_RTL8811A:
      return "DEVOURER_RTL8811A";
    case WiFiCardType::UNKNOWN:
    default:
      return "UNKNOWN";
  }
}

static std::optional<WiFiCardType> wifi_card_type_from_string(
    const std::string& value) {
  if (OHDUtil::equal_after_uppercase(value, "OPENHD_RTL_88X2AU")) {
    return WiFiCardType::OPENHD_RTL_88X2AU;
  }
  if (OHDUtil::equal_after_uppercase(value, "OPENHD_RTL_88X2BU")) {
    return WiFiCardType::OPENHD_RTL_88X2BU;
  }
  if (OHDUtil::equal_after_uppercase(value, "OPENHD_RTL_88X2CU")) {
    return WiFiCardType::OPENHD_RTL_88X2CU;
  }
  if (OHDUtil::equal_after_uppercase(value, "OPENHD_RTL_88X2EU")) {
    return WiFiCardType::OPENHD_RTL_88X2EU;
  }
  if (OHDUtil::equal_after_uppercase(value, "OPENHD_RTL_8852BU")) {
    return WiFiCardType::OPENHD_RTL_8852BU;
  }
  if (OHDUtil::equal_after_uppercase(value, "RTL_88X2AU")) {
    return WiFiCardType::RTL_88X2AU;
  }
  if (OHDUtil::equal_after_uppercase(value, "RTL_88X2BU")) {
    return WiFiCardType::RTL_88X2BU;
  }
  if (OHDUtil::equal_after_uppercase(value, "ATHEROS")) {
    return WiFiCardType::ATHEROS;
  }
  if (OHDUtil::equal_after_uppercase(value, "MT_7921U") ||
      OHDUtil::equal_after_uppercase(value, "MT_7921u")) {
    return WiFiCardType::MT_7921u;
  }
  if (OHDUtil::equal_after_uppercase(value, "RALINK")) {
    return WiFiCardType::RALINK;
  }
  if (OHDUtil::equal_after_uppercase(value, "INTEL")) {
    return WiFiCardType::INTEL;
  }
  if (OHDUtil::equal_after_uppercase(value, "BROADCOM")) {
    return WiFiCardType::BROADCOM;
  }
  if (OHDUtil::equal_after_uppercase(value, "AIC")) {
    return WiFiCardType::AIC;
  }
  if (OHDUtil::equal_after_uppercase(value, "QUALCOMM")) {
    return WiFiCardType::QUALCOMM;
  }
  if (OHDUtil::equal_after_uppercase(value, "ARTOSYN")) {
    return WiFiCardType::ARTOSYN;
  }
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8812A")) return WiFiCardType::DEVOURER_RTL8812A;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8821A")) return WiFiCardType::DEVOURER_RTL8821A;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8814A")) return WiFiCardType::DEVOURER_RTL8814A;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8821C")) return WiFiCardType::DEVOURER_RTL8821C;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8822B")) return WiFiCardType::DEVOURER_RTL8822B;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8822C")) return WiFiCardType::DEVOURER_RTL8822C;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8822E")) return WiFiCardType::DEVOURER_RTL8822E;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8733B")) return WiFiCardType::DEVOURER_RTL8733B;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8852B")) return WiFiCardType::DEVOURER_RTL8852B;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8852C")) return WiFiCardType::DEVOURER_RTL8852C;
  if (OHDUtil::equal_after_uppercase(value, "DEVOURER_RTL8811A")) return WiFiCardType::DEVOURER_RTL8811A;
  if (OHDUtil::equal_after_uppercase(value, "OPENHD_EMULATED")) {
    return WiFiCardType::OPENHD_EMULATED;
  }
  if (OHDUtil::equal_after_uppercase(value, "UNKNOWN")) {
    return WiFiCardType::UNKNOWN;
  }
  return std::nullopt;
}
static int wifi_card_type_to_int(const WiFiCardType& card_type) {
  return static_cast<int>(card_type);
}

static std::string wifi_card_sub_type_as_string(uint8_t sub_type) {
  if (sub_type == WIFI_CARD_SUB_TYPE_UNKNOWN) {
    return "UNKNOWN";
  }
  if (sub_type == WIFI_CARD_SUB_TYPE_RTL8812AU_ASUS) {
    return "ASUS";
  } else if (sub_type == WIFI_CARD_SUB_TYPE_RTL8812AU_X20) {
    return "X20";
  }
  return "ERROR";
}

struct WiFiCard {
  // These 3 are all (slightly different) identifiers of a card on linux.
  std::string device_name;
  std::string mac;
  // phy0, phy1,.., needed for iw commands that don't take the device name
  int phy80211_index = -1;
  // Name of the driver that runs this card.
  std::string driver_name;
  // Detected wifi card type, generated by checking known drivers.
  WiFiCardType type = WiFiCardType::UNKNOWN;
  // Silicon identity from OpenHD's Devourer-compatible USB probe. Empty for
  // non-Devourer devices (which remain usable as normal network/hotspot cards).
  std::string chipset_name;
  std::string devourer_generation;
  uint8_t devourer_chip_id = 0;
  // Admission-table result. Detection is independent from broadcast support.
  bool devourer_wb_enabled = false;
  // More info about a given wifi card - e.g. for rtl8812au, which
  // manufacturer produced the card (required for TX power levels)
  uint8_t sub_type = 0;
  [[nodiscard]] bool supports_2GHz() const {
    return !supported_frequencies_2G.empty();
  };
  [[nodiscard]] bool supports_5GHz() const {
    return !supported_frequencies_5G.empty();
  };
  // Physical wifibroadcast is limited to supported Realtek radios. Other
  // adapters (including Qualcomm and Ralink) remain available for hotspot and
  // normal networking roles.
  [[nodiscard]] bool supports_openhd_wifibroadcast() const {
    const bool kernel_backend_supported =
        type == WiFiCardType::OPENHD_RTL_88X2AU ||
           type == WiFiCardType::OPENHD_RTL_88X2BU ||
           type == WiFiCardType::OPENHD_RTL_88X2CU ||
           type == WiFiCardType::OPENHD_RTL_88X2EU ||
           type == WiFiCardType::OPENHD_EMULATED;
#ifdef OHD_ENABLE_DEVOURER
    return kernel_backend_supported || devourer_wb_enabled;
#else
    return kernel_backend_supported;
#endif
  };
  // Returns true if the given card is exatly rtl8812au on x20 (custom HW) and
  // we know power levels ;)
  [[nodiscard]] bool is_openhd_rtl8812au_x20() const {
    return type == WiFiCardType::OPENHD_RTL_88X2AU &&
           sub_type == WIFI_CARD_SUB_TYPE_RTL8812AU_X20;
  };
  [[nodiscard]] bool is_rtl88x2eu() const {
    return type == WiFiCardType::OPENHD_RTL_88X2EU;
  };
  // supported 2G frequencies, in mhz
  std::vector<uint32_t> supported_frequencies_2G{};
  // supported 5G frequencies, in mhz
  std::vector<uint32_t> supported_frequencies_5G{};
  [[nodiscard]] std::vector<uint32_t> get_supported_frequencies_2G_5G() const {
    std::vector<uint32_t> ret{};
    OHDUtil::vec_append(ret, supported_frequencies_2G);
    OHDUtil::vec_append(ret, supported_frequencies_5G);
    return ret;
  };
};

static bool wifi_card_supports_variable_mcs(const WiFiCard& wifi_card) {
  if (wifi_card.devourer_wb_enabled) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_EMULATED) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2AU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2BU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2CU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2EU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_8852BU) return true;
#ifdef OHD_ENABLE_DEVOURER
  if (wifi_card.type == WiFiCardType::RTL_88X2AU) return true;
  if (wifi_card.type == WiFiCardType::RTL_88X2BU) return true;
#endif
  return false;
}

static bool wifi_card_supports_5Mhz_channel_width_injection(
    const WiFiCard& wifi_card) {
  return false;
}

static bool wifi_card_supports_10Mhz_channel_width_injection(
    const WiFiCard& wifi_card) {
  if (wifi_card.devourer_wb_enabled) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_EMULATED) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2AU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2BU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2CU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2EU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_8852BU) return true;
#ifdef OHD_ENABLE_DEVOURER
  if (wifi_card.type == WiFiCardType::RTL_88X2AU) return true;
  if (wifi_card.type == WiFiCardType::RTL_88X2BU) return true;
#endif
  return false;
}

static bool wifi_card_supports_40Mhz_channel_width_injection(
    const WiFiCard& wifi_card) {
  if (wifi_card.devourer_wb_enabled) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_EMULATED) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2AU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2BU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2CU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2EU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_8852BU) return true;
#ifdef OHD_ENABLE_DEVOURER
  if (wifi_card.type == WiFiCardType::RTL_88X2AU) return true;
  if (wifi_card.type == WiFiCardType::RTL_88X2BU) return true;
#endif
  return false;
}

static bool wifi_card_supports_frequency(const WiFiCard& wifi_card,
                                         const uint32_t frequency) {
  const auto channel_opt = openhd::channel_from_frequency(frequency);
  if (!channel_opt.has_value()) {
    openhd::log::get_default()->debug("OpenHD doesn't know frequency {}",
                                      frequency);
    return false;
  }
  const auto& channel = channel_opt.value();
  for (const auto& supported_frequency :
       wifi_card.get_supported_frequencies_2G_5G()) {
    if (channel.frequency == supported_frequency) {
      return true;
    }
  }
  openhd::log::get_default()->debug("Card {} does not support frequency {}",
                                    wifi_card.device_name, frequency);
  return false;
}

static bool wifi_card_supports_frequency_channel_width(
    const WiFiCard& wifi_card, const int frequency, const int channel_width) {
  auto console = openhd::log::get_default();
  const auto channel_opt = openhd::channel_from_frequency(frequency);
  if (!channel_opt.has_value()) {
    console->debug("OpenHD doesn't know frequency {}", frequency);
    return false;
  }
  const auto& channel = channel_opt.value();
  // card (rtl8812au / bu) will crash otherwise anyways
  if (channel_width == 40 && !channel.is_legal_any_country_40Mhz) {
    console->debug("Card {} doesn't support 40Mhz on {}", wifi_card.device_name,
                   frequency);
    return false;
  }
  return wifi_card_supports_frequency(wifi_card, frequency);
}

static std::string debug_cards(const std::vector<WiFiCard>& cards) {
  std::stringstream ss;
  ss << "size:" << cards.size() << "{";
  for (const auto& card : cards) {
    ss << card.device_name;
    if (!card.chipset_name.empty()) ss << "(" << card.chipset_name << ")";
    ss << ",";
  }
  ss << "}";
  return ss.str();
}

void write_wificards_manifest(const std::vector<WiFiCard>& cards);

#endif
