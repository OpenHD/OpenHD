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

#ifndef OPENHD_OPENHD_UTIL_TIME_H
#define OPENHD_OPENHD_UTIL_TIME_H

#include <chrono>
#include <string>

namespace openhd::util {

std::string verbose_timespan(
    const std::chrono::steady_clock::duration& duration);

int steady_clock_time_epoch_ms();

// R stands for readable. Convert a std::chrono::duration into a readable
// format. Readable format is somewhat arbitrary, in this case readable means
// that for example 1second has 'ms' resolution since for values that big ns
// resolution probably isn't needed
std::string time_readable(const std::chrono::steady_clock::duration& dur);
std::string time_readable_ns(uint64_t nanoseconds);

uint32_t get_micros(std::chrono::nanoseconds ns);

void store_air_unit_time_offset_us(int64_t offset_us);
int64_t get_air_unit_time_offset_us();

}  // namespace openhd::util

#endif  // OPENHD_OPENHD_UTIL_TIME_H
