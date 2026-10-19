#include "openhd_udp.h"
#include <condition_variable>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
  try {
    // Bind ephemeral ports and query them so parallel tests never share ports.
    openhd::net::initialize();
    auto reserve = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(reserve, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
      throw std::runtime_error("Cannot allocate test port");
#ifdef _WIN32
    int size = sizeof(addr);
#else
    socklen_t size = sizeof(addr);
#endif
    getsockname(reserve, reinterpret_cast<sockaddr*>(&addr), &size);
    const auto port = ntohs(addr.sin_port);
    openhd::net::close_socket(reserve);
    std::mutex mutex;
    std::condition_variable changed;
    std::vector<std::vector<uint8_t>> received;
    openhd::UDPReceiver receiver("127.0.0.1", port,
      [&](const uint8_t* bytes, std::size_t count) {
        std::lock_guard<std::mutex> lock(mutex);
        received.emplace_back(bytes, bytes + count);
        changed.notify_all();
      });
    openhd::UDPForwarder sender("127.0.0.1", port);
    const std::vector<uint8_t> payload{0, 10, 13, 26, 255, 42};
    receiver.runInBackground();
    sender.forwardPacketViaUDP(payload.data(), payload.size());
    sender.forwardPacketViaUDP(payload.data(), 0);
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (!changed.wait_for(lock, std::chrono::seconds(3), [&] { return received.size() == 2; }) ||
          received[0] != payload || !received[1].empty())
        throw std::runtime_error("UDP binary/empty datagram roundtrip failed");
    }
    receiver.stopBackground();
    receiver.stopBackground();
    receiver.runInBackground();
    sender.forwardPacketViaUDP(payload.data(), payload.size());
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (!changed.wait_for(lock, std::chrono::seconds(3), [&] { return received.size() == 3; }))
        throw std::runtime_error("UDP restart failed");
    }
    const auto before = std::chrono::steady_clock::now();
    receiver.stopBackground();
    if (std::chrono::steady_clock::now() - before > std::chrono::seconds(1))
      throw std::runtime_error("Idle UDP shutdown hung");
    bool rejected = false;
    try { openhd::UDPForwarder invalid("invalid-address", 5600); }
    catch (const std::invalid_argument&) { rejected = true; }
    if (!rejected) throw std::runtime_error("Invalid address accepted");
    openhd::UDPMultiForwarder multi;
    multi.removeForwarder("127.0.0.1", port);
    std::cout << "UDP binary/empty datagrams, restart, idle stop and validation passed\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
