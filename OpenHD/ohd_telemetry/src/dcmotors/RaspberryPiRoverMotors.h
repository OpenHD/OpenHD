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
#ifndef OPENHD_RASPBERRY_PI_ROVER_MOTORS_H
#define OPENHD_RASPBERRY_PI_ROVER_MOTORS_H

#include <pigpio.h>

#include <iostream>
#include <memory>
#include <vector>

namespace openhd::telemetry::rpi {

class RaspberryPiRoverMotors {
 public:
  RaspberryPiRoverMotors(int in1, int in2, int in3, int in4, int pwm);
  ~RaspberryPiRoverMotors();

  void set_speed(int speed);
  void set_direction_motor_A(bool forward);
  void set_direction_motor_B(bool forward);
  void stop();
  int mapp(int speed);

 private:
  int m_in1;
  int m_in2;
  int m_in3;
  int m_in4;
  int m_pwm;
};

}  // namespace openhd::telemetry::rpi

#endif  // OPENHD_RASPBERRY_PI_ROVER_MOTORS_H

// #ifndef OPENHD_RASPBERRY_PI_ROVER_MOTORS_H
// #define OPENHD_RASPBERRY_PI_ROVER_MOTORS_H

// #include <pigpio.h>

// #include <iostream>
// #include <memory>
// #include <vector>

// namespace openhd::telemetry::rpi {

// class RaspberryPiRoverMotors {
//    public:
//     RaspberryPiRoverMotors(int in1, int pwm);
//     ~RaspberryPiRoverMotors();

//     void set_speed(int speed);
//     void set_angle(int angle);
//     void stop();
//     int map_speed(int speed);

//    private:
//     int m_in1;
//     int m_pwm;
// };

// }  // namespace openhd::telemetry::rpi

// #endif  // OPENHD_RASPBERRY_PI_ROVER_MOTORS_H