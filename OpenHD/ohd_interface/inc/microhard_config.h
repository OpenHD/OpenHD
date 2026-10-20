#ifndef OPENHD_MICROHARD_CONFIG_H
#define OPENHD_MICROHARD_CONFIG_H

#include <arpa/inet.h>

#include <stdexcept>
#include <string>

#include "openhd_config.h"

namespace openhd::microhard {

struct RuntimeConfig {
  std::string peer_ip;
  std::string modem_ip;
  std::string ip_range;
  std::string username;
  std::string password;
  int video_port;
  int telemetry_port;
};

inline void validate_ip(const std::string& ip, const char* setting) {
  in_addr address{};
  if (inet_pton(AF_INET, ip.c_str(), &address) != 1 ||
      ntohl(address.s_addr) == INADDR_ANY ||
      (ntohl(address.s_addr) & 0xf0000000U) >= 0xe0000000U) {
    throw std::invalid_argument(
        std::string("Microhard requires a unicast IPv4 address for ") +
        setting);
  }
}

inline RuntimeConfig make_config(const Config& config, bool is_air) {
  RuntimeConfig result{
      is_air ? config.GROUND_UNIT_IP : config.AIR_UNIT_IP,
      is_air ? config.MICROHARD_IP_AIR : config.MICROHARD_IP_GROUND,
      config.MICROHARD_IP_RANGE.empty() ? "192.168.168"
                                        : config.MICROHARD_IP_RANGE,
      config.MICROHARD_USERNAME,
      config.MICROHARD_PASSWORD,
      config.MICROHARD_VIDEO_PORT,
      config.MICROHARD_TELEMETRY_PORT};
  validate_ip(result.peer_ip, is_air ? "GROUND_UNIT_IP" : "AIR_UNIT_IP");
  if (!result.modem_ip.empty()) {
    validate_ip(result.modem_ip,
                is_air ? "MICROHARD_IP_AIR" : "MICROHARD_IP_GROUND");
  }
  for (const int port : {result.video_port, result.telemetry_port}) {
    if (port < 1 || port > 65535) {
      throw std::invalid_argument("Microhard UDP ports must be in 1..65535");
    }
  }
  if (result.video_port == result.telemetry_port) {
    throw std::invalid_argument(
        "Microhard video and telemetry ports must differ");
  }
  for (const auto* credential : {&result.username, &result.password}) {
    if (credential->find_first_of("\r\n") != std::string::npos ||
        credential->find('\0') != std::string::npos) {
      throw std::invalid_argument(
          "Microhard credentials must be single-line strings");
    }
  }
  return result;
}

// The range is an IPv4 prefix, not a shell expression or a substring match.
inline bool matches_ip_range(const std::string& ip, std::string prefix) {
  if (prefix.empty()) return false;
  if (prefix.back() != '.') prefix += '.';
  return ip.compare(0, prefix.size(), prefix) == 0;
}

}  // namespace openhd::microhard

#endif
