#include "topotek_protocol.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace {

std::string hex_value(uint32_t value, int width) {
  char buffer[9]{};
  std::snprintf(buffer, sizeof(buffer), "%0*X", width, value);
  return buffer;
}

}  // namespace

namespace topotek {

std::string add_checksum(const std::string& body) {
  uint8_t checksum = 0;
  for (const unsigned char byte : body) checksum += byte;
  return body + hex_value(checksum, 2);
}

std::string make_command(char destination, const std::string& identifier,
                         const std::string& data, char control) {
  if (identifier.size() != 3 || data.empty() || data.size() > 15 ||
      (control != 'r' && control != 'w')) {
    throw std::invalid_argument("invalid Topotek command");
  }
  const std::string header = data.size() == 2 ? "#TP" : "#tp";
  const std::string body = header + 'P' + destination +
                           hex_value(static_cast<uint32_t>(data.size()), 1) +
                           control + identifier + data;
  return add_checksum(body);
}

std::string make_gimbal_rate_command(int8_t pitch_rate, int8_t yaw_rate) {
  const auto pitch = static_cast<uint8_t>(
      std::clamp<int>(pitch_rate, -99, 99));
  const auto yaw = static_cast<uint8_t>(
      std::clamp<int>(yaw_rate, -99, 99));
  return make_command('G', "GSM", hex_value(yaw, 2) + hex_value(pitch, 2));
}

std::string make_gimbal_angle_command(float pitch_degrees, float yaw_degrees,
                                       uint8_t speed) {
  const auto pitch = static_cast<int16_t>(std::lround(
      std::clamp(pitch_degrees, -90.0F, 90.0F) * 100.0F));
  const auto yaw = static_cast<int16_t>(std::lround(
      std::clamp(yaw_degrees, -150.0F, 150.0F) * 100.0F));
  const auto clamped_speed = std::min<uint8_t>(speed, 99);
  const std::string data =
      hex_value(static_cast<uint16_t>(yaw), 4) +
      hex_value(clamped_speed, 2) +
      hex_value(static_cast<uint16_t>(pitch), 4) +
      hex_value(clamped_speed, 2);
  return make_command('G', "GAM", data);
}

}  // namespace topotek
