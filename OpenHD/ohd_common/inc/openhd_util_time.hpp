//
// Created by consti10 on 03.01.23.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_TIME_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_TIME_H_

#include <chrono>

namespace openhd::util::time{
// R stands for readable. Convert a std::chrono::duration into a readable format.
// Readable format is somewhat arbitrary, in this case readable means that for example
// 1second has 'ms' resolution since for values that big ns resolution probably isn't needed
static std::string R(const std::chrono::steady_clock::duration &dur) {
  const auto durAbsolute = std::chrono::abs(dur);
  if (durAbsolute >= std::chrono::seconds(1)) {
    // More than one second, print as decimal with ms resolution.
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    return std::to_string(static_cast<float>(ms) / 1000.0f) + "s";
  }
  if (durAbsolute >= std::chrono::milliseconds(1)) {
    // More than one millisecond, print as decimal with us resolution
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
    return std::to_string(static_cast<float>(us) / 1000.0f) + "ms";
  }
  if (durAbsolute >= std::chrono::microseconds(1)) {
    // More than one microsecond, print as decimal with ns resolution
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    return std::to_string(static_cast<float>(ns) / 1000.0f) + "us";
  }
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  return std::to_string(ns) + "ns";
}
static std::string ReadableNS(uint64_t nanoseconds) {
  return R(std::chrono::nanoseconds(nanoseconds));
}
}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_UTIL_TIME_H_
