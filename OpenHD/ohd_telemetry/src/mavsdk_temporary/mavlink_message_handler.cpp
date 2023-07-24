#include <mutex>
#include "mavlink_message_handler.h"

namespace mavsdk {

void MavlinkMessageHandler::register_one(
    uint16_t msg_id, const Callback& callback, const void* cookie)
{
    std::lock_guard<std::mutex> lock(_mutex);

    Entry entry = {msg_id, {}, callback, cookie};
    _table.push_back(entry);
}

void MavlinkMessageHandler::register_one(
    uint16_t msg_id,
    std::optional<uint8_t> component_id,
    const Callback& callback,
    const void* cookie)
{
    std::lock_guard<std::mutex> lock(_mutex);

    Entry entry = {msg_id, component_id, callback, cookie};
    _table.push_back(entry);
}

void MavlinkMessageHandler::unregister_one(uint16_t msg_id, const void* cookie)
{
    std::lock_guard<std::mutex> lock(_mutex);

    for (auto it = _table.begin(); it != _table.end();
         /* no ++it */) {
        if (it->msg_id == msg_id && it->cookie == cookie) {
            it = _table.erase(it);
        } else {
            ++it;
        }
    }
}

void MavlinkMessageHandler::unregister_all(const void* cookie)
{
    std::lock_guard<std::mutex> lock(_mutex);

    for (auto it = _table.begin(); it != _table.end();
         /* no ++it */) {
        if (it->cookie == cookie) {
            it = _table.erase(it);
        } else {
            ++it;
        }
    }
}

void MavlinkMessageHandler::process_message(const mavlink_message_t& message)
{
    std::lock_guard<std::mutex> lock(_mutex);

#if MESSAGE_DEBUGGING == 1
    bool forwarded = false;
#endif
    for (auto& entry : _table) {
        if (entry.msg_id == message.msgid &&
            (!entry.cmp_id.has_value() || entry.cmp_id == message.compid)) {
#if MESSAGE_DEBUGGING == 1
            LogDebug() << "Forwarding msg " << int(message.msgid) << " to "
                       << size_t(entry->cookie);
            forwarded = true;
#endif
            entry.callback(message);
        }
    }

#if MESSAGE_DEBUGGING == 1
    if (!forwarded) {
        LogDebug() << "Ignoring msg " << int(message.msgid);
    }
#endif
}

void MavlinkMessageHandler::update_component_id(
    uint16_t msg_id, uint8_t component_id, const void* cookie)
{
    std::lock_guard<std::mutex> lock(_mutex);

    for (auto& entry : _table) {
        if (entry.msg_id == msg_id && entry.cookie == cookie) {
            entry.cmp_id = component_id;
        }
    }
}

} // namespace mavsdk
