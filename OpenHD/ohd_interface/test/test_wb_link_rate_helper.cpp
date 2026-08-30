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

  // Exercise a complete clocked RC settings frame and duplicate suppression.
  openhd::wb::RCSettingsProtocol protocol;
  std::array<int, 18> rc{};
  rc.fill(1000);
  bool clock = false;
  protocol.update(rc, 9);  // establish the initial clock level
  const uint32_t frame =
      openhd::wb::RCSettingsProtocol::encode_frame(1, 1165, 3);
  auto send_symbol = [&](uint8_t symbol) {
    rc[8] = (symbol & 4) ? 2000 : 1000;
    rc[9] = (symbol & 2) ? 2000 : 1000;
    rc[10] = (symbol & 1) ? 2000 : 1000;
    clock = !clock;
    rc[11] = clock ? 2000 : 1000;
    return protocol.update(rc, 9);
  };
  if (send_symbol(7)) return 5;
  std::optional<openhd::wb::RCSettingsProtocol::Command> decoded;
  for (int n = 7; n >= 0; --n)
    decoded = send_symbol(static_cast<uint8_t>((frame >> (n * 3)) & 7));
  if (!decoded || decoded->setting_id != 1 || decoded->value != 1165 ||
      decoded->sequence != 3)
    return 6;
  return 0;
}
