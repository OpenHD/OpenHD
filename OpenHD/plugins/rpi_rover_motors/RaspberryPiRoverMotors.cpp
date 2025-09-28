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
#include "RaspberryPiRoverMotors.h"

#include <algorithm>

#include <pigpio.h>

namespace {
int clamp_pwm(int value) {
  return std::clamp(value, 0, 255);
}
}  // namespace

RaspberryPiRoverMotors::RaspberryPiRoverMotors(int in1, int in2, int in3,
                                               int in4, int pwm_pin)
    : in1_(in1), in2_(in2), in3_(in3), in4_(in4), pwm_pin_(pwm_pin) {
  if (gpioInitialise() < 0) {
    return;
  }
  initialized_ = true;

  gpioSetMode(in1_, PI_OUTPUT);
  gpioSetMode(in2_, PI_OUTPUT);
  gpioSetMode(in3_, PI_OUTPUT);
  gpioSetMode(in4_, PI_OUTPUT);
  gpioSetMode(pwm_pin_, PI_OUTPUT);

  gpioWrite(in1_, 0);
  gpioWrite(in2_, 0);
  gpioWrite(in3_, 0);
  gpioWrite(in4_, 0);
  gpioPWM(pwm_pin_, 0);
}

RaspberryPiRoverMotors::~RaspberryPiRoverMotors() {
  if (!initialized_) {
    return;
  }
  stop();
  gpioTerminate();
}

void RaspberryPiRoverMotors::set_speed(int speed) {
  if (!initialized_) {
    return;
  }
  gpioPWM(pwm_pin_, clamp_pwm(speed));
}

void RaspberryPiRoverMotors::set_direction_motor_a(bool forward) {
  if (!initialized_) {
    return;
  }
  gpioWrite(in1_, forward ? 1 : 0);
  gpioWrite(in2_, forward ? 0 : 1);
}

void RaspberryPiRoverMotors::set_direction_motor_b(bool forward) {
  if (!initialized_) {
    return;
  }
  gpioWrite(in3_, forward ? 1 : 0);
  gpioWrite(in4_, forward ? 0 : 1);
}

void RaspberryPiRoverMotors::stop() {
  if (!initialized_) {
    return;
  }
  gpioWrite(in1_, 0);
  gpioWrite(in2_, 0);
  gpioWrite(in3_, 0);
  gpioWrite(in4_, 0);
  gpioPWM(pwm_pin_, 0);
}

int RaspberryPiRoverMotors::map_speed(int speed) const {
  constexpr int kInputMin = -500;
  constexpr int kInputMax = 500;
  constexpr int kOutputMax = 255;
  if (speed < kInputMin) {
    speed = kInputMin;
  } else if (speed > kInputMax) {
    speed = kInputMax;
  }
  // Scale from [-500, 500] to [-255, 255].
  return (speed * kOutputMax) / kInputMax;
}
