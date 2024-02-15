//
// Created by consti10 on 06.12.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_RTP_EOF_HELPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_RTP_EOF_HELPER_H_

#include <cstdint>

namespace openhd::rtp_eof_helper {

// rather than adding a dependency on gstreamer (for example), write the bit of
// code that determines the end of a NALU inside a h264 / h265 RTP packet

struct RTPFragmentInfo {
  bool is_fu_start;
  bool is_fu_end;
  // ONLY set if this is a fu_start frame !
  int nal_unit_type;
};

// Use if input is rtp h264 stream
// returns true if this is the end of a rtp fragmentation unit
RTPFragmentInfo h264_more_info(const uint8_t *payload, std::size_t payloadSize);
RTPFragmentInfo h265_more_info(const uint8_t *payload, std::size_t payloadSize);

}  // namespace openhd::rtp_eof_helper

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_RTP_EOF_HELPER_H_
