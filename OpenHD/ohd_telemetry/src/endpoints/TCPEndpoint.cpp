//
// Created by consti10 on 19.04.23.
//

#include "TCPEndpoint.h"

#include <utility>

TCPEndpoint::TCPEndpoint(TCPEndpoint::Config config)
    : MEndpoint("TCPServer"),
      m_config(std::move(config))
{
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  bzero(&sockaddr, sizeof(sockaddr));
}

TCPEndpoint::~TCPEndpoint() {
  close();
}

bool TCPEndpoint::sendMessagesImpl(
    const std::vector<MavlinkMessage>& messages) {
  return false;
}

int TCPEndpoint::accept(int listener_fd) {
  return 0;
}

bool TCPEndpoint::setup() {
  return false;
}

bool TCPEndpoint::reopen() {
  return false;
}

void TCPEndpoint::close() {

}
