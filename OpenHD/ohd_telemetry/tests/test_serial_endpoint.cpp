//
// Created by consti10 on 19.04.22.
// Executable to test the serial endpoint implementation
// For testing, just connect a flight controller via uart and look at the console logs
//

#include <iostream>

#include "../src/endpoints/SerialEndpoint.h"
//#include "../src/endpoints/SerialEndpoint2.h"
#include "../src/mav_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

static const char optstr[] = "?s:";
static const struct option long_options[] = {
    {"serial", required_argument, nullptr, 's'},
    {nullptr, 0, nullptr, 0},
};


int main(int argc, char *argv[]) {
  int c;
  std::string serial_port_filename="/dev/ttyACM0";
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
      case 's':
        serial_port_filename=std::string(tmp_optarg);
        break;
      case '?':
      default:
        std::cout << "Usage: \n" <<
            "--serial -s [serial port string filename, e.g. /dev/ttyACMO]\n";
        break;
    }
  }

  std::cout << "SerialEndpointTest::start with ["<<serial_port_filename<<"]\n";

  SerialEndpoint serialEndpoint("TestSerialPort", {serial_port_filename,0}, true); //115200
  serialEndpoint.registerCallback([](MavlinkMessage &msg) {
	debugMavlinkMessage(msg.m, "SerialTest");
  });
  // now mavlink messages should come in. Try disconnecting and reconnecting, and see if messages continue
  const auto start = std::chrono::steady_clock::now();
  while ((std::chrono::steady_clock::now() - start) < std::chrono::minutes(5)) {
	serialEndpoint.debugIfAlive();
	// some implementations need a heartbeat before they start sending data.
	auto msg = MExampleMessage::heartbeat();
	serialEndpoint.sendMessage(msg);
	std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "SerialEndpointTest::end" << std::endl;
  return 0;
}


