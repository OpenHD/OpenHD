// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "mav_include.h"

// Before XML generation was restored, the manually maintained 1231 header
// used CRC 177 instead of 178. Accept only that exact unsigned legacy frame;
// all other CRC/signature failures remain failures. Forward it canonically.
inline uint8_t openhd_parse_char(uint8_t channel, uint8_t byte,
                                 mavlink_message_t* message,
                                 mavlink_status_t* status) {
  auto result = mavlink_frame_char(channel, byte, message, status);
  if (result == MAVLINK_FRAMING_BAD_CRC &&
      message->msgid == MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_RADIO_SETTINGS &&
      message->magic == MAVLINK_STX && message->incompat_flags == 0 &&
      message->len <= MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_RADIO_SETTINGS_LEN) {
    uint8_t frame[MAVLINK_MAX_PACKET_LEN];
    mavlink_msg_to_send_buffer(frame, message);
    auto checksum = crc_calculate(frame + 1, 9 + message->len);
    auto legacy = checksum;
    crc_accumulate(177, &legacy);
    if (legacy == message->checksum) {
      crc_accumulate(MAVLINK_MSG_ID_OPENHD_WIFBROADCAST_RADIO_SETTINGS_CRC,
                     &checksum);
      message->checksum = checksum;
      return MAVLINK_FRAMING_OK;
    }
  }
  if (result == MAVLINK_FRAMING_BAD_CRC ||
      result == MAVLINK_FRAMING_BAD_SIGNATURE) {
    auto* parser = mavlink_get_channel_status(channel);
    parser->parse_error++;
    parser->msg_received = MAVLINK_FRAMING_INCOMPLETE;
    parser->parse_state = MAVLINK_PARSE_STATE_IDLE;
    if (byte == MAVLINK_STX) {
      parser->parse_state = MAVLINK_PARSE_STATE_GOT_STX;
      mavlink_get_channel_buffer(channel)->len = 0;
      mavlink_start_checksum(mavlink_get_channel_buffer(channel));
    }
    return MAVLINK_FRAMING_INCOMPLETE;
  }
  return result;
}
