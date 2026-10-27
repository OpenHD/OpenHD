// SPDX-License-Identifier: GPL-3.0-only
#include "endpoints/SerialEndpoint.h"
#include "legacy_radio_settings.h"
#include <fcntl.h>
#include <pty.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

const char* getVideoPath() { return "/tmp/"; }
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (0)

MavlinkMessage message(uint32_t id, int sys = 1) {
  MavlinkMessage result{};
  result.m.msgid = id; result.m.sysid = sys; result.m.compid = 1;
  return result;
}
class ParserEndpoint : public MEndpoint {
 public:
  ParserEndpoint() : MEndpoint("parser_test") {}
  void feed(const uint8_t* data, int size) { parseNewData(data, size); }
 private:
  bool sendMessagesImpl(const std::vector<MavlinkMessage>&) override { return true; }
};

int main() {
  try {
    const auto now = SerialTxQueue::Clock::now();
    auto bulk = message(MAVLINK_MSG_ID_PARAM_VALUE);
    auto rc = message(MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE);
    auto command = message(MAVLINK_MSG_ID_COMMAND_LONG);
    SerialTxQueue queue;
    CHECK(queue.push(bulk, now)); CHECK(queue.push(command, now)); CHECK(queue.push(rc, now));
    CHECK(queue.pop(now)->message.m.msgid == rc.m.msgid);
    CHECK(queue.pop(now)->message.m.msgid == command.m.msgid);
    CHECK(queue.pop(now)->message.m.msgid == bulk.m.msgid);
    queue.push(rc, now); queue.push(rc, now);
    CHECK(queue.size() == 1 && queue.replaced == 1);
    CHECK(!queue.pop(now + std::chrono::milliseconds(251)) && queue.expired == 1);
    for (size_t i = 0; i < SerialTxQueue::capacity; ++i) CHECK(queue.push(bulk, now));
    CHECK(!queue.push(bulk, now)); CHECK(queue.push(command, now));
    CHECK(queue.size() == SerialTxQueue::capacity && queue.dropped == 2);
    CHECK(queue.pop(now)->message.m.msgid == command.m.msgid);
    queue.clear();
    // Source priority settings survive across enqueue calls.
    bulk.uart_priority = 10;
    queue.push(command, now); queue.push(bulk, now);
    CHECK(queue.pop(now)->message.uart_priority == 10);
    queue.clear(); bulk.uart_priority = -1;
    queue.push(bulk, now);
    for (int i = 0; i < 16; ++i) queue.push(command, now);
    bool bulk_sent = false;
    for (int i = 0; i < 9; ++i) bulk_sent |= queue.pop(now)->message.m.msgid == bulk.m.msgid;
    CHECK(bulk_sent);
    CHECK(!queue.pop(now + std::chrono::seconds(6)));
    queue.clear();
    bulk.m.incompat_flags = command.m.incompat_flags = MAVLINK_IFLAG_SIGNED;
    queue.push(bulk, now); queue.push(command, now);
    CHECK(queue.pop(now)->message.m.msgid == bulk.m.msgid);
    CHECK(queue.pop(now)->message.m.msgid == command.m.msgid);

    MavlinkMessage card{};
    mavlink_openhd_stats_monitor_mode_wifi_card_t payload{};
    payload.rx_rssi = -61;
    mavlink_msg_openhd_stats_monitor_mode_wifi_card_encode(100, 191, &card.m, &payload);
    auto bytes = card.pack();
    unsigned received = 0;
    // The old allocation ran past MAVLINK_COMM_NUM_BUFFERS after 16 creations.
    for (int i = 0; i < 40; ++i) {
      ParserEndpoint endpoint;
      endpoint.registerCallback([&](std::vector<MavlinkMessage> messages) {
        received += messages.size();
        mavlink_openhd_stats_monitor_mode_wifi_card_t decoded{};
        mavlink_msg_openhd_stats_monitor_mode_wifi_card_decode(&messages[0].m, &decoded);
        CHECK(decoded.rx_rssi == -61);
      });
      endpoint.feed(bytes.data(), 5);
      endpoint.feed(bytes.data() + 5, bytes.size() - 5);
    }
    CHECK(received == 40);
    ParserEndpoint first, second;
    first.registerCallback([&](auto messages) { received += messages.size(); });
    second.registerCallback([&](auto messages) { received += messages.size(); });
    first.feed(bytes.data(), 5); second.feed(bytes.data(), bytes.size());
    first.feed(bytes.data() + 5, bytes.size() - 5);
    CHECK(received == 42);
    bytes[12] ^= 1; first.feed(bytes.data(), bytes.size()); CHECK(received == 42);

    int master, slave; char path[128]{};
    CHECK(openpty(&master, &slave, path, nullptr, nullptr) == 0);
    close(slave);
    CHECK(fcntl(master, F_SETFL, O_NONBLOCK) == 0);
    SerialEndpoint::HWOptions options{};
    options.linux_filename = path; options.enable_reading = false;
    SerialEndpoint endpoint("async_test", options);
    endpoint.registerCallback([](auto) {});
    mavlink_message_t heartbeat{};
    mavlink_msg_heartbeat_pack(255, 190, &heartbeat, MAV_TYPE_GCS,
                             MAV_AUTOPILOT_INVALID, 0, 0, MAV_STATE_ACTIVE);
    bool connected = false;
    uint8_t data[4096];
    for (int i = 0; i < 100 && !connected; ++i) {
      endpoint.sendMessages({MavlinkMessage{heartbeat}});
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      connected = read(master, data, sizeof(data)) > 0;
    }
    CHECK(connected);
    // Pause the reader long enough to fill a PTY with large frames, then
    // verify that backpressure/short writes never splice or truncate frames.
    MavlinkMessage large{};
    mavlink_encapsulated_data_t large_payload{};
    std::memset(large_payload.data, 0x55, sizeof(large_payload.data));
    mavlink_msg_encapsulated_data_encode(1, 1, &large.m, &large_payload);
    endpoint.sendMessages(std::vector<MavlinkMessage>(100, large));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mavlink_message_t rx_buffer{}, parsed{};
    mavlink_status_t rx_parser{}, status{};
    unsigned complete = 0, invalid = 0;
    const auto read_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (complete < 100 && std::chrono::steady_clock::now() < read_deadline) {
      const auto count = read(master, data, sizeof(data));
      for (ssize_t i = 0; i < count; ++i) {
        const auto result = mavlink_frame_char_buffer(&rx_buffer, &rx_parser, data[i], &parsed, &status);
        if (result == MAVLINK_FRAMING_OK && parsed.msgid == large.m.msgid) ++complete;
        else if (result == MAVLINK_FRAMING_BAD_CRC) ++invalid;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    CHECK(complete == 100 && invalid == 0);
    // Saturate an unread PTY. sendMessages must stay responsive and stop must
    // interrupt the writer even with a full kernel buffer and pending queue.
    MavlinkMessage parameter{}; mavlink_param_value_t parameter_payload{};
    mavlink_msg_param_value_encode(1, 1, &parameter.m, &parameter_payload);
    std::vector<MavlinkMessage> burst(3000, parameter);
    auto begin = std::chrono::steady_clock::now();
    endpoint.sendMessages(burst);
    CHECK(std::chrono::steady_clock::now() - begin < std::chrono::seconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    begin = std::chrono::steady_clock::now();
    endpoint.sendMessages({MavlinkMessage{heartbeat}});
    CHECK(std::chrono::steady_clock::now() - begin < std::chrono::milliseconds(100));
    begin = std::chrono::steady_clock::now(); endpoint.stop();
    CHECK(std::chrono::steady_clock::now() - begin < std::chrono::milliseconds(1500));
    // Exercise restart and disconnect/FD teardown while producers are active.
    endpoint.start();
    std::atomic<bool> run{true};
    std::thread producer([&] { while (run) {
      endpoint.sendMessages({MavlinkMessage{heartbeat}});
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }});
    close(master);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    endpoint.stop(); run = false; producer.join();
    std::cout << "Async UART queue, congestion, stop/restart, disconnect and 40 parser instances passed\n";
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return EXIT_FAILURE; }
}
