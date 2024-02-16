#include "connection.h"

#include <memory>
#include <utility>
// #include "mavsdk_impl.h"
#include <atomic>

#include "mavlink_channels.h"

namespace mavsdk {

std::atomic<unsigned> Connection::_forwarding_connections_count = 0;

Connection::Connection(receiver_callback_t receiver_callback,
                       ForwardingOption forwarding_option)
    : _receiver_callback(std::move(receiver_callback)),
      _mavlink_receiver(nullptr),
      _forwarding_option(forwarding_option) {
  // Insert system ID 0 in all connections for broadcast.
  _system_ids.insert(0);

  if (forwarding_option == ForwardingOption::ForwardingOn) {
    _forwarding_connections_count++;
  }
}

Connection::~Connection() {
  // Just in case a specific connection didn't call it already.
  stop_mavlink_receiver();
  _receiver_callback = {};
}

bool Connection::start_mavlink_receiver() {
  if (_mavlink_receiver) {
    // mavlink receiver already created
    return true;
  }
  uint8_t channel;
  if (!MAVLinkChannels::Instance().checkout_free_channel(channel)) {
    return false;
  }

  _mavlink_receiver = std::make_unique<MAVLinkReceiver>(channel);
  return true;
}

void Connection::stop_mavlink_receiver() {
  if (_mavlink_receiver) {
    uint8_t used_channel = _mavlink_receiver->get_channel();
    // Destroy receiver before giving the channel back.
    _mavlink_receiver.reset();
    MAVLinkChannels::Instance().checkin_used_channel(used_channel);
  }
}

void Connection::receive_message(mavlink_message_t& message,
                                 Connection* connection) {
  // Register system ID when receiving a message from a new system.
  if (_system_ids.find(message.sysid) == _system_ids.end()) {
    _system_ids.insert(message.sysid);
  }
  _receiver_callback(message, connection);
}

bool Connection::should_forward_messages() const {
  return _forwarding_option == ForwardingOption::ForwardingOn;
}

unsigned Connection::forwarding_connections_count() {
  return _forwarding_connections_count;
}

bool Connection::has_system_id(uint8_t system_id) {
  return _system_ids.find(system_id) != _system_ids.end();
}

}  // namespace mavsdk
