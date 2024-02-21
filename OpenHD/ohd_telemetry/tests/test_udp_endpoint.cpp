//
// Created by consti10 on 22.04.22.
// For testing, run QOpenHD and look at the console logs
//

#include <endpoints/UDPEndpoint.h>

#include <iostream>

#include "../src/mav_helper.h"
#include "../src/mav_include.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"

// test if the connection to QOpenHD / QGroundControll can be sucesfully
// established. Run this application on the same system QOpenHD is running on,
// and you should see heartbeat messages from the ground controll application,
// and a changed artificial horizon in the gc application if the gc supports
// that

int main() {
  std::cout << "UdpEndpointTest::start" << std::endl;
  UDPEndpoint udpEndpoint("UdpEndpoint", OHD_GROUND_CLIENT_UDP_PORT_OUT,
                          OHD_GROUND_CLIENT_UDP_PORT_IN);
  auto cb = [](const std::vector<MavlinkMessage> messages) {
    for (const auto& msg : messages) {
      debugMavlinkMessage(msg.m, "Udp");
    }
  };
  udpEndpoint.registerCallback(cb);
  // now mavlink messages should come in. Try disconnecting and reconnecting,
  // and see if messages continue
  const auto start = std::chrono::steady_clock::now();
  while ((std::chrono::steady_clock::now() - start) < std::chrono::minutes(5)) {
    openhd::log::get_default()->debug(
        "Alive:{}", OHDUtil::yes_or_no(udpEndpoint.isAlive()));
    auto heartbeat = MExampleMessage::heartbeat();
    udpEndpoint.sendMessages({heartbeat});
    auto position = MExampleMessage::position();
    udpEndpoint.sendMessages({position});
    auto attitude = MExampleMessage::attitude();
    udpEndpoint.sendMessages({attitude});
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "UdpEndpointTest::end" << std::endl;
  return 0;
}