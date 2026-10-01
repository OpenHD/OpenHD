#include "openhd_tcp.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <stdexcept>

class Server final : public openhd::TCPServer {
 public:
  Server() : TCPServer("test", {15760}) {}
  std::atomic<int> connects{0}, disconnects{0}, packets{0};
  void on_packet_any_tcp_client(const uint8_t*, int) override { ++packets; }
  void on_external_device(std::string, int, bool connected) override {
    if (connected) ++connects; else ++disconnects;
  }
};

template <typename Predicate> void wait_for(Predicate predicate) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!predicate()) {
    if (std::chrono::steady_clock::now() >= deadline)
      throw std::runtime_error("TCP host lifetime check timed out");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

int connect_client() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(15760);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  wait_for([&] { return connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0; });
  return fd;
}

int main() {
  Server server;
  int viewer = connect_client();
  wait_for([&] { return server.connects == 1; });
  int diagnostic = connect_client();
  send(diagnostic, "x", 1, 0);
  wait_for([&] { return server.packets == 1; });
  close(diagnostic);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  if (server.connects != 1 || server.disconnects != 0)
    throw std::runtime_error("Closing diagnostic socket disconnected the viewer host");
  send(viewer, "x", 1, 0);
  wait_for([&] { return server.packets == 2; });
  close(viewer);
  wait_for([&] { return server.disconnects == 1; });
  int reconnect = connect_client();
  wait_for([&] { return server.connects == 2; });
  close(reconnect);
  wait_for([&] { return server.disconnects == 2; });
  std::cout << "TCP host lifetime regression passed\n";
}
