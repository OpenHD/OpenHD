//
// Created by rsaxvc on 14.08.23.
// Executable to test the FIFO endpoint implementation
// For testing, just connect both ends to the same fifo and send some packets
//

#include <iostream>
#include "../src/endpoints/FifoEndpoint.h"
#include "fifo_endpoint_test_helper.h"
#include <memory>
#include <csignal>

int main(int argc, char *argv[]) {

  const auto fifo_options=fifo_endpoint_test_helper::parse_args(argc,argv);

  std::cout << "FifoEndpointTest::start with "<<fifo_endpoint_test_helper::options_to_string(fifo_options);

  // hook up to both ends of the FIFO to emulate a simple MAVLink loopback
  auto options=FifoEndpoint::HWOptions{};
  options.linux_filename_rx=fifo_options.filename;
  options.linux_filename_tx=fifo_options.filename;
  options.enable_debug=true;
  auto fifo_endpoint=std::make_unique<FifoEndpoint>("fifo_test",options);
  fifo_endpoint->registerCallback([&fifo_endpoint](std::vector<MavlinkMessage> messages) {
        //debugMavlinkMessage(msg.m, "FifoTest3");
        fifo_endpoint->sendMessages(messages);
  });
  // now mavlink messages should come in. Try disconnecting and reconnecting, and see if messages continue
  const auto start = std::chrono::steady_clock::now();
  static bool quit=false;
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, [](int sig){ quit= true;});

  // wait 1 second to ensure fifo endpoint has opened the receive end
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // send ourselves a heartbeat to get started
  auto msg = MExampleMessage::heartbeat();
  fifo_endpoint->sendMessages({msg});

  while (((std::chrono::steady_clock::now() - start) < std::chrono::seconds(60))&& !quit) {
	std::cout<<fifo_endpoint->createInfo();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  fifo_endpoint.reset();
  fifo_endpoint= nullptr;
  std::cout << "FifoEndpointTest3::end" << std::endl;
  return 0;
}
