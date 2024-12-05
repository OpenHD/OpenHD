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

// Executable to test the serial endpoint implementation
// For testing, just connect a flight controller via uart and look at the
// console logs
//

#include <csignal>
#include <iostream>
#include <memory>

#include "../src/endpoints/SerialEndpoint.h"
#include "openhd_spdlog_include.h"
#include "serial_endpoint_test_helper.h"

int main(int argc, char *argv[]) {
  const auto serial_options =
      serial_endpoint_test_helper::parse_args(argc, argv);

  std::cout << "SerialEndpointTest::start with "
            << serial_endpoint_test_helper::options_to_string(serial_options);

  auto options = SerialEndpoint::HWOptions{};
  options.linux_filename = serial_options.filename;
  options.baud_rate = serial_options.baud_rate;
  options.flow_control = false;
  options.enable_debug = true;

  auto serial_endpoint = std::make_unique<SerialEndpoint>("ser_test", options);
  serial_endpoint->registerCallback([](std::vector<MavlinkMessage> messages) {
    // debugMavlinkMessage(msg.m, "SerialTest3");
  });
  // now mavlink messages should come in. Try disconnecting and reconnecting,
  // and see if messages continue
  const auto start = std::chrono::steady_clock::now();
  static bool quit = false;
  signal(SIGTERM, [](int sig) { quit = true; });
  while (
      ((std::chrono::steady_clock::now() - start) < std::chrono::seconds(60)) &&
      !quit) {
    std::cout << serial_endpoint->createInfo();
    // some implementations need a heartbeat before they start sending data.
    auto msg = MExampleMessage::heartbeat();
    serial_endpoint->sendMessages({msg});
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  serial_endpoint.reset();
  serial_endpoint = nullptr;
  std::cout << "SerialEndpointTest3::end" << std::endl;
  return 0;
}