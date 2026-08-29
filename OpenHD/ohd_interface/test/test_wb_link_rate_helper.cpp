#include <array>

#include "wb_link_helper.h"

int main() {
  WiFiCard card{};
  card.devourer_wb_enabled = true;

  constexpr std::array devourer_types{
      WiFiCardType::DEVOURER_RTL8812A, WiFiCardType::DEVOURER_RTL8821A,
      WiFiCardType::DEVOURER_RTL8814A, WiFiCardType::DEVOURER_RTL8821C,
      WiFiCardType::DEVOURER_RTL8822B, WiFiCardType::DEVOURER_RTL8822C,
      WiFiCardType::DEVOURER_RTL8822E, WiFiCardType::DEVOURER_RTL8733B,
      WiFiCardType::DEVOURER_RTL8852B, WiFiCardType::DEVOURER_RTL8852C,
      WiFiCardType::DEVOURER_RTL8811A};
  for (const auto type : devourer_types) {
    card.type = type;
    if (openhd::wb::calculate_bitrate_for_wifi_config_kbits(
            card, 5785, 20, 2, 100, false) != 13200)
      return 1;
    if (openhd::wb::calculate_bitrate_for_wifi_config_kbits(
            card, 5785, 40, 2, 100, false) != 20600)
      return 2;
  }

  card.type = WiFiCardType::DEVOURER_RTL8812A;
  if (openhd::wb::calculate_bitrate_for_wifi_config_kbits(
          card, 5785, 40, 3, 100, false) != 25000)
    return 3;

  card.type = WiFiCardType::DEVOURER_RTL8822E;
  if (openhd::wb::calculate_bitrate_for_wifi_config_kbits(
          card, 5785, 40, 5, 100, false) != 40000)
    return 4;
  return 0;
}
