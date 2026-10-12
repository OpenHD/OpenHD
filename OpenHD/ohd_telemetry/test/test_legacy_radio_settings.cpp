// SPDX-License-Identifier: GPL-3.0-only
#include "legacy_radio_settings.h"
#include <cassert>
#include <iostream>

int main() {
  mavlink_message_t sent{}, received{};
  mavlink_status_t status{};
  mavlink_msg_openhd_wifbroadcast_radio_settings_pack(255, 190, &sent,
                                                      100, 191, 1, 5, 6, 7);
  uint8_t bytes[MAVLINK_MAX_PACKET_LEN];
  const auto length = mavlink_msg_to_send_buffer(bytes, &sent);
  auto parse = [&]() {
    mavlink_reset_channel_status(0);
    unsigned count = 0;
    for (unsigned i = 0; i < length; ++i)
      count += openhd_parse_char(0, bytes[i], &received, &status) == MAVLINK_FRAMING_OK;
    return count;
  };
  assert(parse() == 1); // canonical generated CRC 178
  auto crc = crc_calculate(bytes + 1, length - 3);
  crc_accumulate(177, &crc);
  bytes[length - 2] = crc & 255;
  bytes[length - 1] = crc >> 8;
  assert(parse() == 1); // published legacy CRC 177
  assert(received.checksum == sent.checksum); // canonical on forwarding
  mavlink_openhd_wifbroadcast_radio_settings_t settings{};
  mavlink_msg_openhd_wifbroadcast_radio_settings_decode(&received, &settings);
  assert(settings.target_system == 100 && settings.mcs_via_rc_channel == 5);
  bytes[12] ^= 1;
  assert(parse() == 0); // corrupt legacy payload rejected
  std::cout << "Generated, legacy and corrupt radio frames verified\n";
}
