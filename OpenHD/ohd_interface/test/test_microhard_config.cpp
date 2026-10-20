#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>

#include "microhard_link.h"

namespace {
using openhd::microhard::make_config;

template <class Change>
void rejects(const openhd::Config& original, Change change,
             bool is_air = true) {
  auto config = original;
  change(config);
  try {
    make_config(config, is_air);
  } catch (const std::invalid_argument&) {
    return;
  }
  throw std::runtime_error("Invalid Microhard configuration was accepted");
}

int bind_socket(const std::string& ip, int port) {
  const int fd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(fd >= 0);
  const int reuse = 1;
  assert(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == 0);
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  assert(inet_pton(AF_INET, ip.c_str(), &address.sin_addr) == 1);
  assert(bind(fd, reinterpret_cast<const sockaddr*>(&address),
              sizeof(address)) == 0);
  return fd;
}

int free_port() {
  const int fd = bind_socket("127.0.0.1", 0);
  sockaddr_in address{};
  socklen_t size = sizeof(address);
  assert(getsockname(fd, reinterpret_cast<sockaddr*>(&address), &size) == 0);
  close(fd);
  return ntohs(address.sin_port);
}

struct PeerSocket {
  int fd;
  ~PeerSocket() { close(fd); }
  void expect(const std::vector<uint8_t>& expected) {
    pollfd event{fd, POLLIN, 0};
    assert(poll(&event, 1, 3000) == 1);
    std::vector<uint8_t> received(2048);
    const auto size = recv(fd, received.data(), received.size(), 0);
    assert(size >= 0);
    received.resize(size);
    assert(received == expected);
  }
};

void check_data_path(openhd::Config config, bool is_air) {
  // No modem is present on this documentation-only subnet. The data path
  // must still work; receivers must never bind to the remote host/modem.
  config.MICROHARD_IP_AIR.clear();
  config.MICROHARD_IP_GROUND.clear();
  config.MICROHARD_IP_RANGE = "203.0.113";
  config.AIR_UNIT_IP = "127.0.0.2";
  config.GROUND_UNIT_IP = "127.0.0.3";
  config.MICROHARD_VIDEO_PORT = free_port();
  do {
    config.MICROHARD_TELEMETRY_PORT = free_port();
  } while (config.MICROHARD_VIDEO_PORT == config.MICROHARD_TELEMETRY_PORT);
  const auto peer_ip = is_air ? config.GROUND_UNIT_IP : config.AIR_UNIT_IP;
  // Exact loopback binds take precedence over the link's wildcard receivers.
  PeerSocket peer_video{bind_socket(peer_ip, config.MICROHARD_VIDEO_PORT)};
  PeerSocket peer_telemetry{bind_socket(peer_ip, config.MICROHARD_TELEMETRY_PORT)};
  std::promise<std::vector<uint8_t>> video;
  std::promise<std::vector<uint8_t>> telemetry;
  auto video_result = video.get_future();
  auto telemetry_result = telemetry.get_future();
  MicrohardLink link(config, OHDProfile(is_air, "microhard-test"));
  link.register_on_receive_video_data_cb(
      [&video](int stream, const uint8_t* data, int len) {
        assert(stream == 0);
        video.set_value(std::vector<uint8_t>(data, data + len));
      });
  link.register_on_receive_telemetry_data_cb(
      [&telemetry](auto data) { telemetry.set_value(*data); });
  const std::vector<uint8_t> video_payload{1, 3, 5, 7};
  const std::vector<uint8_t> telemetry_payload{2, 4, 6};
  link.transmit_telemetry_data(
      {std::make_shared<std::vector<uint8_t>>(telemetry_payload)});
  peer_telemetry.expect(telemetry_payload);
  openhd::UDPForwarder("127.0.0.1", config.MICROHARD_TELEMETRY_PORT)
      .forwardPacketViaUDP(telemetry_payload.data(), telemetry_payload.size());
  assert(telemetry_result.wait_for(std::chrono::seconds(3)) ==
         std::future_status::ready);
  assert(telemetry_result.get() == telemetry_payload);
  if (is_air) {
    openhd::FragmentedVideoFrame frame;
    frame.rtp_fragments.push_back(
        std::make_shared<std::vector<uint8_t>>(video_payload));
    link.transmit_video_data(0, frame);
    peer_video.expect(video_payload);
  } else {
    openhd::UDPForwarder("127.0.0.1", config.MICROHARD_VIDEO_PORT)
        .forwardPacketViaUDP(video_payload.data(), video_payload.size());
    assert(video_result.wait_for(std::chrono::seconds(3)) ==
           std::future_status::ready);
    assert(video_result.get() == video_payload);
  }
}
}  // namespace

int main() {
  openhd::Config config;
  config.AIR_UNIT_IP = "192.0.2.11";
  config.GROUND_UNIT_IP = "192.0.2.10";
  config.MICROHARD_IP_AIR = "198.51.100.1";
  config.MICROHARD_IP_GROUND = "198.51.100.2";
  config.MICROHARD_IP_RANGE = "198.51.100";
  config.MICROHARD_VIDEO_PORT = 15910;
  config.MICROHARD_TELEMETRY_PORT = 15920;
  config.MICROHARD_USERNAME = "operator";
  config.MICROHARD_PASSWORD = " space;'quoted\"password; ";
  const auto air = make_config(config, true);
  const auto ground = make_config(config, false);
  assert(air.peer_ip == config.GROUND_UNIT_IP);
  assert(ground.peer_ip == config.AIR_UNIT_IP);
  assert(air.modem_ip == config.MICROHARD_IP_AIR);
  assert(ground.modem_ip == config.MICROHARD_IP_GROUND);
  assert(air.video_port == 15910 && ground.telemetry_port == 15920);
  assert(air.username == config.MICROHARD_USERNAME);
  assert(ground.password == config.MICROHARD_PASSWORD);
  assert(air.ip_range == "198.51.100");
  config.MICROHARD_IP_RANGE.clear();
  assert(make_config(config, true).ip_range == "192.168.168");
  assert(openhd::microhard::matches_ip_range("192.168.168.1", "192.168.168"));
  assert(openhd::microhard::matches_ip_range("10.20.1.1", "10.20."));
  assert(!openhd::microhard::matches_ip_range("192.168.1680.1", "192.168.168"));
  assert(!openhd::microhard::matches_ip_range("10.2.0.1", ""));
  rejects(config, [](auto& c) { c.GROUND_UNIT_IP.clear(); });
  rejects(
      config, [](auto& c) { c.AIR_UNIT_IP.clear(); }, false);
  rejects(config, [](auto& c) { c.GROUND_UNIT_IP = "not-an-ip"; });
  rejects(config, [](auto& c) { c.GROUND_UNIT_IP = "0.0.0.0"; });
  rejects(config, [](auto& c) { c.GROUND_UNIT_IP = "224.1.2.3"; });
  rejects(config, [](auto& c) { c.MICROHARD_IP_AIR = "bad-modem-ip"; });
  rejects(config, [](auto& c) { c.MICROHARD_VIDEO_PORT = 0; });
  rejects(config, [](auto& c) { c.MICROHARD_TELEMETRY_PORT = 65536; });
  rejects(config,
          [](auto& c) { c.MICROHARD_TELEMETRY_PORT = c.MICROHARD_VIDEO_PORT; });
  rejects(config, [](auto& c) { c.MICROHARD_USERNAME = "admin\nAT+COMMAND"; });
  rejects(config, [](auto& c) { c.MICROHARD_PASSWORD = "secret\r"; });
  rejects(config,
          [](auto& c) { c.MICROHARD_PASSWORD = std::string("a\0b", 3); });
  check_data_path(config, true);
  check_data_path(config, false);
  std::cout << "Microhard configuration and UDP data path regression passed\n";
}
