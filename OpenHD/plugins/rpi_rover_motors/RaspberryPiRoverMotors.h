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
#ifndef OPENHD_PLUGINS_RPI_ROVER_MOTORS_H
#define OPENHD_PLUGINS_RPI_ROVER_MOTORS_H

#include <cstdint>

class RaspberryPiRoverMotors {
 public:
  RaspberryPiRoverMotors(int in1, int in2, int in3, int in4, int pwm_pin);
  ~RaspberryPiRoverMotors();

  RaspberryPiRoverMotors(const RaspberryPiRoverMotors &) = delete;
  RaspberryPiRoverMotors &operator=(const RaspberryPiRoverMotors &) = delete;

  bool initialized() const { return initialized_; }

  void set_speed(int speed);
  void set_direction_motor_a(bool forward);
  void set_direction_motor_b(bool forward);
  void stop();
  int map_speed(int speed) const;

 private:
  int in1_;
  int in2_;
  int in3_;
  int in4_;
  int pwm_pin_;
  bool initialized_ = false;
};

#endif  // OPENHD_PLUGINS_RPI_ROVER_MOTORS_H
