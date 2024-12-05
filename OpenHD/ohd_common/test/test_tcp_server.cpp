/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

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