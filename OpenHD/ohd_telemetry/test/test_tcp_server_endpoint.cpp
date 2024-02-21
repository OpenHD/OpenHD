//
// Created by consti10 on 19.04.23.
//

#include <endpoints/TCPEndpoint.h>

#include <iostream>

#include "../src/mav_helper.h"
#include "../src/mav_include.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"

int main() {
  openhd::log::get_default()->debug("test_tcp_server_endpoint:end");
  std::unique_ptr<TCPEndpoint> m_server = std::make_unique<TCPEndpoint>(
      openhd::TCPServer::Config{TCPEndpoint::DEFAULT_PORT});  // 1445
  auto cb = [](const std::vector<MavlinkMessage> messages) {
    for (const auto& msg : messages) {
      debugMavlinkMessage(msg.m, "TCP received");
    }
  };
  m_server->registerCallback(cb);

  // now mavlink messages should come in. Try disconnecting and reconnecting,
  // and see if messages continue
  const auto start = std::chrono::steady_clock::now();
  while ((std::chrono::steady_clock::now() - start) <
         std::chrono::seconds(30)) {
    openhd::log::get_default()->debug("Alive:{}",
                                      OHDUtil::yes_or_no(m_server->isAlive()));
    auto heartbeat = MExampleMessage::heartbeat();
    m_server->sendMessages({heartbeat});
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  openhd::log::get_default()->debug("test_tcp_server_endpoint: end");
  return 0;
}