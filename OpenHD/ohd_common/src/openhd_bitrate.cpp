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

#include "openhd_bitrate.h"

openhd::BitrateDebugger::BitrateDebugger(std::string tag, bool debug_pps)
    : m_debug_pps(debug_pps) {
  m_console = openhd::log::create_or_get(tag);
}

void openhd::BitrateDebugger::on_packet(int64_t n_bytes) {
  assert(n_bytes > 0);
  m_bytes += n_bytes;
  m_n_packets++;
  const auto elapsed = std::chrono::steady_clock::now() - m_last_log;
  if (elapsed > std::chrono::seconds(1)) {
    const double elapsed_us =
        (double)std::chrono::duration_cast<std::chrono::microseconds>(elapsed)
            .count();
    const double elapsed_s = elapsed_us / 1000.0 / 1000.0;
    const double bytes_per_s = static_cast<double>(m_bytes) / elapsed_s;
    const double pps = static_cast<double>(m_n_packets) / elapsed_s;
    if (m_debug_pps) {
      m_console->debug("{} {}", bytes_per_second_to_string(bytes_per_s),
                       openhd::pps_to_string(pps));
    } else {
      m_console->debug("{}", bytes_per_second_to_string(bytes_per_s));
    }
    m_bytes = 0;
    m_n_packets = 0;
    m_last_log = std::chrono::steady_clock::now();
  }
}

std::string openhd::bits_per_second_to_string(uint64_t bits_per_second) {
  const auto mBits_per_second =
      static_cast<double>(bits_per_second) / (1000.0 * 1000.0);
  std::stringstream ss;
  ss.precision(2);
  if (mBits_per_second > 1) {
    ss << mBits_per_second << "mBit/s";
    return ss.str();
  }
  const auto kBits_per_second = static_cast<float>(bits_per_second) / 1000;
  ss << kBits_per_second << "kBit/s";
  return ss.str();
}

std::string openhd::bytes_per_second_to_string(double bytes_per_second) {
  const auto bits_per_second = bytes_per_second * 8;
  std::stringstream ss;
  ss.precision(2);
  if (bits_per_second > 1000 * 1000) {
    const auto mBits_per_second =
        static_cast<double>(bits_per_second) / (1000.0 * 1000.0);
    ss << mBits_per_second << "mBit/s";
    return ss.str();
  }
  const auto kBits_per_second = static_cast<double>(bits_per_second) / 1000;
  ss << kBits_per_second << "kBit/s";
  return ss.str();
}

std::string openhd::pps_to_string(double pps) {
  std::stringstream ss;
  ss.precision(2);
  ss << pps << "pps";
  return ss.str();
}
