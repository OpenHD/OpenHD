#ifndef OPENHD_TOPOTEK_PROTOCOL_H
#define OPENHD_TOPOTEK_PROTOCOL_H

#include <cstdint>
#include <string>

namespace topotek {

std::string add_checksum(const std::string& body);
std::string make_command(char destination, const std::string& identifier,
                         const std::string& data, char control = 'w');
std::string make_gimbal_rate_command(int8_t pitch_rate, int8_t yaw_rate);
std::string make_gimbal_angle_command(float pitch_degrees, float yaw_degrees,
                                       uint8_t speed);

}  // namespace topotek

#endif  // OPENHD_TOPOTEK_PROTOCOL_H
