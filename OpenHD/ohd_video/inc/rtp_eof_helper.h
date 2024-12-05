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
