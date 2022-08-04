//
// Created by consti10 on 19.04.22.
// Executable to test the serial endpoint implementation
// For testing, just connect a flight controller via uart and look at the console logs
//

#include <iostream>
#include "../src/endpoints/SerialEndpoint3.h"
#include "serial_endpoint_test_helper.h"
#include <memory>

int main(int argc, char *argv[]) {

  const auto serial_options=serial_endpoint_test_helper::parse_args(argc,argv);

  std::cout << "SerialEndpointTest::start with "<<serial_endpoint_test_helper::options_to_string(serial_options);

  auto options=SerialEndpoint3::HWOptions{};
  options.linux_filename=serial_options.filename;
  options.baud_rate=serial_options.baud_rate;
  options.flow_control=false;
  options.enable_debug=true;

  auto serial_endpoint=std::make_unique<SerialEndpoint3>("SerialEndpoint3Test",options);
  serial_endpoint->registerCallback([](MavlinkMessage &msg) {
	debugMavlinkMessage(msg.m, "SerialTest3");
  });
  // now mavlink messages should come in. Try disconnecting and reconnecting, and see if messages continue
  const auto start = std::chrono::steady_clock::now();
  while ((std::chrono::steady_clock::now() - start) < std::chrono::minutes(5)) {
	std::cout<<serial_endpoint->createInfo();
    // some implementations need a heartbeat before they start sending data.
    auto msg = MExampleMessage::heartbeat();
    serial_endpoint->sendMessage(msg);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "SerialEndpointTest3::end" << std::endl;
  return 0;
}