#include <array>

#include "wb_link_helper.h"
#include "channel_survey_average.h"

int main() {
  openhd::wb::ChannelSurveyAverage average;
  if (average.metrics() != 0x3fff) return 40;
  average.add(20, 0xc000 | (80 << 7) | 60);
  average.add(0, 0x3fff);  // failed visit must not halve the first reading
  if (average.packets() != 20) return 41;
  average.add(40, 0xc000 | (20 << 7) | 40);
  if (average.packets() != 30 || average.metrics() != (0xc000 | (50 << 7) | 50)) return 42;
  average.add(0, 0); // valid quiet packets, unsupported CCX
  if (average.packets() != 20 || average.metrics() != (0xc000 | (50 << 7) | 50)) return 43;
  WiFiCard card{};
  // Full-band analysis must include both halves of 40 MHz channel pairs.
  card.supported_frequencies_5G = {5100, 5180, 5200, 5220, 5240,
                                   5745, 5765, 5785, 5805, 5845, 5865, 5885};
  const auto survey = openhd::wb::get_analyze_channels_frequencies(card, 2);
  const auto expected = card.supported_frequencies_5G;
  if (survey.size() != expected.size()) return 30;
  for (size_t i = 0; i < expected.size(); ++i)
    if (survey[i].frequency != expected[i]) return 31;
  card.supported_frequencies_2G = openhd::get_all_channel_frequencies(
      openhd::get_channels_2G_legal_at_least_one_country());
  const auto survey2g = openhd::wb::get_analyze_channels_frequencies(card, 1);
  if (survey2g.size() != 14) return 32;
  for (int i = 0; i < 13; ++i)
    if (survey2g[i].frequency != 2412 + i * 5) return 33;
  if (survey2g.back().frequency != 2484) return 34;
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

  // Disabled widths must be rejected even when the radio supports them.
  auto logger = openhd::log::get_default();
  // Advertised 2.4 GHz primaries must also pass the actual setting validator.
  for (const auto frequency : card.supported_frequencies_2G)
    if (!openhd::wb::all_cards_support_frequency_and_channel_width(
            frequency, 20, {card}, logger)) return 35;
  for (const auto width : {5}) {
    if (openhd::wb::validate_air_channel_width_change(width, card, logger) ||
        openhd::wb::all_cards_support_frequency_and_channel_width(
            5785, width, {card}, logger))
      return 7;
  }
  if (!openhd::wb::validate_air_channel_width_change(10, card, logger) ||
      openhd::wb::calculate_bitrate_for_wifi_config_kbits(
          card, 5785, 10, 2, 100, false) != 7600 ||
      !openhd::wb::validate_air_channel_width_change(20, card, logger) ||
      !openhd::wb::validate_air_channel_width_change(40, card, logger))
    return 8;

  // Every backend identity is checked, including Devourer's EU mapping.
  for (const auto type : devourer_types) {
    card.type = type;
    if (wifi_card_supports_10Mhz_channel_width_injection(card) !=
        (type == WiFiCardType::DEVOURER_RTL8822E)) return 9;
  }
  card.type = WiFiCardType::OPENHD_RTL_88X2EU;
  if (!wifi_card_supports_10Mhz_channel_width_injection(card)) return 10;
  card.supported_frequencies_5G = {5785};
  WiFiCard bu = card;
  bu.type = WiFiCardType::DEVOURER_RTL8822B;
  if (openhd::wb::all_cards_support_frequency_and_channel_width(
          5785, 10, {card, bu}, logger)) return 11;
  if (!openhd::wb::all_cards_support_frequency_and_channel_width(
          5785, 10, {card}, logger)) return 12;

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
