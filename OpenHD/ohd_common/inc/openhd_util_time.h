//
// Created by consti10 on 21.02.24.
//

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
