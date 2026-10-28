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

#include "openhd_spdlog.h"
#include "openhd_udp.h"
#include <spdlog/spdlog.h>
#include <array>
#include <stdexcept>
#include <utility>
#ifndef _WIN32
#include <poll.h>
#endif

namespace {
auto get_console() { return openhd::log::create_or_get("UDP"); }
sockaddr_in destination(const std::string& address, int port) {
  if (port < 0 || port > 65535) throw std::invalid_argument("Invalid UDP port");
  sockaddr_in result{};
  result.sin_family = AF_INET;
  result.sin_port = htons(static_cast<uint16_t>(port));
  if (inet_pton(AF_INET, address.c_str(), &result.sin_addr) != 1)
    throw std::invalid_argument("Invalid IPv4 address: " + address);
  return result;
}
bool send_packet(openhd::net::Socket socket, const sockaddr_in& address,
                 const uint8_t* packet, std::size_t size) {
  if (size > openhd::UDPReceiver::UDP_PACKET_MAX_SIZE)
    throw std::invalid_argument("UDP datagram too large");
  const auto sent = sendto(socket, reinterpret_cast<const char*>(packet),
                           static_cast<int>(size), 0,
                           reinterpret_cast<const sockaddr*>(&address), sizeof(address));
  if (sent < 0 || static_cast<std::size_t>(sent) != size)
    get_console()->warn("UDP send failed: {}", openhd::net::error_message());
  return sent >= 0 && static_cast<std::size_t>(sent) == size;
}
}

openhd::UDPForwarder::UDPForwarder(std::string address, int port)
    : client_addr(std::move(address)), client_udp_port(port) {
  net::initialize();
  saddr = destination(client_addr, client_udp_port);
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sockfd == net::invalid_socket)
    throw std::runtime_error("UDP socket: " + net::error_message());
}
openhd::UDPForwarder::~UDPForwarder() { net::close_socket(sockfd); }
bool openhd::UDPForwarder::forwardPacketViaUDP(const uint8_t* packet, std::size_t size) const {
  return send_packet(sockfd, saddr, packet, size);
}

void openhd::UDPMultiForwarder::addForwarder(const std::string &client_addr,
                                             int client_udp_port) {
  std::lock_guard<std::mutex> guard(udpForwardersLock);
  // check if we already forward data to this IP::Port tuple
  for (const auto &udpForwarder : udpForwarders) {
    if (udpForwarder->client_addr == client_addr &&
        udpForwarder->client_udp_port == client_udp_port) {
      get_console()->info("UDPMultiForwarder: already forwarding to: {}:{}",
                          client_addr, client_udp_port);
      return;
    }
  }
  get_console()->info("UDPMultiForwarder: add forwarding to: {}:{}",
                      client_addr, client_udp_port);
  udpForwarders.emplace_back(
      std::make_unique<openhd::UDPForwarder>(client_addr, client_udp_port));
}

void openhd::UDPMultiForwarder::removeForwarder(const std::string &client_addr,
                                                int client_udp_port) {
  std::lock_guard<std::mutex> guard(udpForwardersLock);
  const auto found = std::find_if(udpForwarders.begin(), udpForwarders.end(),
                   [&client_addr, &client_udp_port](const auto &udpForwarder) {
                     return udpForwarder->client_addr == client_addr &&
                            udpForwarder->client_udp_port == client_udp_port;
                   });
  if (found != udpForwarders.end()) udpForwarders.erase(found);
}

void openhd::UDPMultiForwarder::forwardPacketViaUDP(
    const uint8_t *packet, const std::size_t packetSize) {
  std::lock_guard<std::mutex> guard(udpForwardersLock);
  for (const auto &udpForwarder : udpForwarders) {
    udpForwarder->forwardPacketViaUDP(packet, packetSize);
  }
}

const std::list<std::unique_ptr<openhd::UDPForwarder>>
    &openhd::UDPMultiForwarder::getForwarders() const {
  return udpForwarders;
}

openhd::UDPReceiver::UDPReceiver(std::string address, int port, OUTPUT_DATA_CALLBACK cb)
    : mCb(std::move(cb)), mSocket(openUdpSocketForReceiving(address, port)) {}
openhd::UDPReceiver::~UDPReceiver() {
  stopBackground();
  net::close_socket(mSocket);
}
void openhd::UDPReceiver::loopUntilError() {
  std::array<uint8_t, UDP_PACKET_MAX_SIZE> buffer{};
  while (receiving.load()) {
#ifdef _WIN32
    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(mSocket, &readable);
    // Bounded wait avoids closing a descriptor while the receive thread uses it.
    timeval timeout{0, 100000};
    const int ready = select(0, &readable, nullptr, nullptr, &timeout);
#else
    // poll also works for Linux descriptors above FD_SETSIZE.
    pollfd readable{mSocket, POLLIN, 0};
    const int ready = poll(&readable, 1, 100);
#endif
    if (!receiving.load()) break;
    if (ready < 0) {
#ifndef _WIN32
      if (errno == EINTR) continue;
#endif
      get_console()->warn("UDP wait failed: {}", net::error_message());
      break;
    }
    if (ready == 0) continue;
    const auto length = recv(mSocket, reinterpret_cast<char*>(buffer.data()),
                              static_cast<int>(buffer.size()), 0);
    if (length >= 0) {
      mCb(buffer.data(), static_cast<std::size_t>(length));
    } else if (std::chrono::steady_clock::now() - m_last_receive_error_log >=
               std::chrono::seconds(3)) {
      get_console()->warn("UDP receive failed: {}", net::error_message());
      m_last_receive_error_log = std::chrono::steady_clock::now();
    }
  }
}
void openhd::UDPReceiver::forwardPacketViaUDP(const std::string& address, int port,
                                             const uint8_t* packet, std::size_t size) const {
  send_packet(mSocket, destination(address, port), packet, size);
}
void openhd::UDPReceiver::stopLooping() { receiving.store(false); }
void openhd::UDPReceiver::runInBackground() {
  if (receiverThread) return;
  receiving.store(true);
  receiverThread = std::make_unique<std::thread>(&UDPReceiver::loopUntilError, this);
}
void openhd::UDPReceiver::stopBackground() {
  stopLooping();
  if (receiverThread && receiverThread->joinable()) receiverThread->join();
  receiverThread.reset();
}
openhd::net::Socket openhd::openUdpSocketForReceiving(const std::string& address, int port) {
  net::initialize();
  const auto addr = destination(address, port);
  const auto fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (fd == net::invalid_socket)
    throw std::runtime_error("UDP socket: " + net::error_message());
  setSocketReuse(fd);
  if (bind(fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0) {
    const auto error = net::error_message();
    net::close_socket(fd);
    throw std::runtime_error("UDP bind " + address + ":" + std::to_string(port) + ": " + error);
  }
  return fd;
}
void openhd::setSocketReuse(net::Socket fd) {
  const int enabled = 1;
#ifdef _WIN32
  // Windows SO_REUSEADDR permits competing processes to steal datagrams.
  constexpr int option = SO_EXCLUSIVEADDRUSE;
#else
  constexpr int option = SO_REUSEADDR;
#endif
  if (setsockopt(fd, SOL_SOCKET, option, reinterpret_cast<const char*>(&enabled), sizeof(enabled)) != 0)
    get_console()->warn("UDP socket option failed: {}", net::error_message());
}
