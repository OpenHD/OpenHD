//
// Created by consti10 on 31.01.24.
//

#include "openhd_spdlog_include.h"
#include "openhd_tcp.h"

class TestServer : public openhd::TCPServer {
 public:
  explicit TestServer()
      : openhd::TCPServer("Test", openhd::TCPServer::Config{5760}){};
  void on_external_device(std::string ip, int port, bool connected) override {
    if (connected) {
      openhd::log::get_default()->debug("Device {}:{} connected", ip, port);
    } else {
      openhd::log::get_default()->debug("Device {}:{} disconnected", ip, port);
    }
  };
  void on_packet_any_tcp_client(const uint8_t* data, int data_len) override {
    // do nothing
    openhd::log::get_default()->debug("Got data {}", data_len);
  };
};

int main(int argc, char* argv[]) {
  auto test_server = std::make_unique<TestServer>();
  // Run netcat 0.0.0.0 5760 to test
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string buff = "Hello\n";
    const int buff_size = buff.length() + 1;
    test_server->send_message_to_all_clients((uint8_t*)buff.c_str(), buff_size);
  }
  std::this_thread::sleep_for(std::chrono::seconds(100));
}