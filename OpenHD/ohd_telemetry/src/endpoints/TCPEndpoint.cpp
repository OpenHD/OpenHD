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
}

bool TCPEndpoint::sendMessagesImpl(
    const std::vector<MavlinkMessage>& messages) {
  return false;
}
