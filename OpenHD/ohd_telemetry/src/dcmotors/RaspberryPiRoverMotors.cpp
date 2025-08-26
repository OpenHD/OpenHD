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

namespace openhd::telemetry::rpi {

RaspberryPiRoverMotors::RaspberryPiRoverMotors(int in1, int in2, int in3, int in4, int pwm)
    : m_in1(in1), m_in2(in2), m_in3(in3), m_in4(in4), m_pwm(pwm) {
    gpioTerminate();
    gpioInitialise();
    gpioSetMode(m_in1, PI_OUTPUT);
    gpioSetMode(m_in2, PI_OUTPUT);
    gpioSetMode(m_in3, PI_OUTPUT);
    gpioSetMode(m_in4, PI_OUTPUT);
    gpioSetMode(m_pwm, PI_OUTPUT);
}

RaspberryPiRoverMotors::~RaspberryPiRoverMotors() {
    gpioWrite(m_in1, 0);
    gpioWrite(m_in2, 0);
    gpioWrite(m_in3, 0);
    gpioWrite(m_in4, 0);
    gpioPWM(m_pwm, 0);
    gpioTerminate();
}

void RaspberryPiRoverMotors::set_speed(int speed) {
    gpioPWM(m_pwm, speed);
}

void RaspberryPiRoverMotors::set_direction_motor_A(bool forward) {
    if (forward) {
        gpioWrite(m_in1, 1);
        gpioWrite(m_in2, 0);
    } else {
        gpioWrite(m_in1, 0);
        gpioWrite(m_in2, 1);
    }
}

void RaspberryPiRoverMotors::set_direction_motor_B(bool forward) {
    if (forward) {
        gpioWrite(m_in3, 1);
        gpioWrite(m_in4, 0);
    } else {
        gpioWrite(m_in3, 0);
        gpioWrite(m_in4, 1);
    }
}

void RaspberryPiRoverMotors::stop() {
    gpioWrite(m_in1, 0);
    gpioWrite(m_in2, 0);
    gpioWrite(m_in3, 0);
    gpioWrite(m_in4, 0);
    gpioPWM(m_pwm, 0);
}

int RaspberryPiRoverMotors::mapp(int speed) {
    return (speed - 1000) * 254 / (1999 - 1000);
}
}
// #include "RaspberryPiRoverMotors.h"

// namespace openhd::telemetry::rpi {

// RaspberryPiRoverMotors::RaspberryPiRoverMotors(int in1, int pwm)
//     :m_in1(in1), m_pwm(pwm) {
//     gpioTerminate();
//     gpioInitialise();
//     gpioSetMode(m_in1, PI_OUTPUT);
//     gpioSetMode(m_pwm, PI_OUTPUT);
// }

// RaspberryPiRoverMotors::~RaspberryPiRoverMotors() {
//     gpioServo(m_in1, 0);
//     gpioPWM(m_pwm, 0);
//     gpioTerminate();
// }

// void RaspberryPiRoverMotors::set_speed(int speed) {
//     gpioPWM(m_pwm, speed);
// }

// void RaspberryPiRoverMotors::set_angle(int angle) {
//     int pulseWidth = (2000 - pulseWidth) * 180 / (2000 - 500);
//     gpioServo(m_in1, pulseWidth);
//     gpioTerminate();
// }

// void RaspberryPiRoverMotors::stop() {
//     gpioWrite(m_in1, 0);
//     gpioPWM(m_pwm, 0);
// }

// int RaspberryPiRoverMotors::map_speed(int speed) {
//     return (speed - 1000) * 254 / (1999 - 1000);
// }
// }