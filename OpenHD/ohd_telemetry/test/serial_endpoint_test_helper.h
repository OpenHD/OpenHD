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

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_TESTS_SERIAL_ENDPOINT_TEST_HELPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_TESTS_SERIAL_ENDPOINT_TEST_HELPER_H_

#include <getopt.h>

#include <sstream>

namespace serial_endpoint_test_helper {

static const char optstr[] = "?s:b:";
static const struct option long_options[] = {
    {"serial", required_argument, nullptr, 's'},
    {"baud", required_argument, nullptr, 'b'},
    {nullptr, 0, nullptr, 0},
};

struct SerialOptions {
  std::string filename = "/dev/ttyACM0";
  int baud_rate = 115200;
  bool flow_controll = false;
};

static SerialOptions parse_args(int argc, char *argv[]) {
  std::string serial_port_filename = "/dev/ttyACM0";
  int baud_rate = 115200;
  int c;
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
      case 's':
        serial_port_filename = std::string(tmp_optarg);
        break;
      case 'b':
        baud_rate = std::atoi(tmp_optarg);
        break;
      case '?':
      default:
        std::cout << "Usage: \n"
                  << "--serial -s [serial port string filename, default: "
                     "/dev/ttyACMO]\n"
                  << "--baud -b [baud rate, default: 115200]\n";
        break;
    }
  }
  return SerialOptions{serial_port_filename, baud_rate};
}

static std::string options_to_string(const SerialOptions &option) {
  std::stringstream ss;
  ss << "[" << option.filename << " : " << option.baud_rate << "]\n";
  return ss.str();
}

}  // namespace serial_endpoint_test_helper

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_TESTS_SERIAL_ENDPOINT_TEST_HELPER_H_
