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

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_BITRATE_CONVERSIONS_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_BITRATE_CONVERSIONS_H_

#include <sstream>

#include "openhd_spdlog.h"

// NOTE: I am not completely sure, but the more common approach seems to
// multiply / divide by 1000 When converting mBit/s to kBit/s or the other way
// around. Therefore, we have the conversions here globally, and it is
// recommended to use them instead of manually converting bit-rates by
// multiplication / division somewhere in code.
namespace openhd {
static int kbits_to_bits_per_second(int kbits_per_second) {
  return kbits_per_second * 1000;
}
static int kbits_to_mbits_per_second(int kbits_per_second) {
  return kbits_per_second / 1000;
}
static int mbits_to_kbits_per_second(int mbits_per_second) {
  return mbits_per_second * 1000;
}
static int bits_per_second_to_kbits_per_second(int bits_per_second) {
  return bits_per_second / 1000;
}

std::string bits_per_second_to_string(uint64_t bits_per_second);

static std::string kbits_per_second_to_string(uint64_t kbits_per_second) {
  return bits_per_second_to_string(kbits_per_second * 1000);
}

static std::string bytes_per_second_to_string(double bytes_per_second);
static std::string pps_to_string(double pps);

class BitrateDebugger {
 public:
  explicit BitrateDebugger(std::string tag, bool debug_pps = false);
  void on_packet(int64_t n_bytes);

 private:
  const bool m_debug_pps;
  std::shared_ptr<spdlog::logger> m_console;
  std::chrono::steady_clock::time_point m_last_log;
  uint64_t m_bytes;
  int m_n_packets;
};
}  // namespace openhd
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_BITRATE_CONVERSIONS_H_
