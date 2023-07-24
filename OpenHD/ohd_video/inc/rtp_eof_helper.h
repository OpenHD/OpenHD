//
// Created by consti10 on 06.12.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_RTP_EOF_HELPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_RTP_EOF_HELPER_H_

#include <cstdint>

namespace openhd::rtp_eof_helper{

// rather than adding a dependency on gstreamer (for example), write the bit of code that determines the end of a NALU
// inside a h264 / h265 RTP packet

// Use if input is rtp h264 stream
// returns true if this is the end of a rtp fragmentation unit
bool h264_end_block(const uint8_t *payload, std::size_t payloadSize);
bool h265_end_block(const uint8_t *payload, std::size_t payloadSize);
bool mjpeg_end_block(const uint8_t *payload, std::size_t payloadSize);

}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_RTP_EOF_HELPER_H_
