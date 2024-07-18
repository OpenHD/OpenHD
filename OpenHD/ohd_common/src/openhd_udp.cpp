//
// Created by consti10 on 22.01.24.
//
#include "openhd_udp.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <sstream>

#include "openhd_spdlog.h"

static std::shared_ptr<spdlog::logger> get_console() {
  return openhd::log::create_or_get("UDP");
}

openhd::UDPForwarder::UDPForwarder(std::string client_addr1,
                                   int client_udp_port1)
    : client_addr(std::move(client_addr1)), client_udp_port(client_udp_port1) {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::stringstream message;
    message << "Error opening socket:" << strerror(errno) << "\n";
    get_console()->warn(message.str());
  }
  // set up the destination
  bzero((char *)&saddr, sizeof(saddr));
  saddr.sin_family = AF_INET;
  // saddr.sin_addr.s_addr = inet_addr(client_addr.c_str());
  inet_aton(client_addr.c_str(), (in_addr *)&saddr.sin_addr.s_addr);
  saddr.sin_port = htons((uint16_t)client_udp_port);
  get_console()->info("UDPForwarder::configured for {} {}", client_addr,
                      client_udp_port);
}

openhd::UDPForwarder::~UDPForwarder() { close(sockfd); }

void openhd::UDPForwarder::forwardPacketViaUDP(
    const uint8_t *packet, const std::size_t packetSize) const {
  // send(sockfd,packet,packetSize, MSG_DONTWAIT);
  // openhd::log::get_default()->debug("Forward {}",packetSize);
  const auto ret = sendto(sockfd, packet, packetSize, 0,
                          (const struct sockaddr *)&saddr, sizeof(saddr));
  if (ret < 0 || ret != packetSize) {
    get_console()->warn("Error sending packet of size:{} to {}:{} code:{} {}",
                        packetSize, client_addr, client_udp_port, ret,
                        strerror(errno));
  }
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
  udpForwarders.erase(
      std::find_if(udpForwarders.begin(), udpForwarders.end(),
                   [&client_addr, &client_udp_port](const auto &udpForwarder) {
                     return udpForwarder->client_addr == client_addr &&
                            udpForwarder->client_udp_port == client_udp_port;
                   }));
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

openhd::UDPReceiver::UDPReceiver(std::string client_addr, int client_udp_port,
                                 openhd::UDPReceiver::OUTPUT_DATA_CALLBACK cb)
    : mCb(cb) {
  mSocket = openhd::openUdpSocketForReceiving(client_addr, client_udp_port);
  get_console()->info("UDPReceiver created with {}:{}", client_addr,
                      client_udp_port);
}

openhd::UDPReceiver::~UDPReceiver() { stopBackground(); }

void openhd::UDPReceiver::loopUntilError() {
  const auto buff =
      std::make_unique<std::array<uint8_t, UDP_PACKET_MAX_SIZE>>();
  // sockaddr_in source;
  // socklen_t sourceLen= sizeof(sockaddr_in);
  while (receiving) {
    // const ssize_t message_length =
    // recvfrom(mSocket,buff->data(),UDP_PACKET_MAX_SIZE,
    // MSG_WAITALL,(sockaddr*)&source,&sourceLen);
    const ssize_t message_length =
        recv(mSocket, buff->data(), buff->size(), MSG_WAITALL);
    if (message_length > 0) {
      mCb(buff->data(), (size_t)message_length);
    } else {
      // this can also come from the shutdown, in which case it is not an error.
      // But this way we break out of the loop.
      if (receiving) {
        if (std::chrono::steady_clock::now() - m_last_receive_error_log >=
            std::chrono::seconds(3)) {
          get_console()->warn("Got message length of: {} log_skip_count:{}",
                              message_length,
                              m_last_receive_error_log_skip_count);
          m_last_receive_error_log = std::chrono::steady_clock::now();
          m_last_receive_error_log_skip_count = 0;
        } else {
          m_last_receive_error_log_skip_count++;
        }
      }
    }
  }
  get_console()->debug("UDP end");
}

void openhd::UDPReceiver::forwardPacketViaUDP(
    const std::string &destIp, const int destPort, const uint8_t *packet,
    const std::size_t packetSize) const {
  // set up the destination
  struct sockaddr_in saddr {};
  bzero((char *)&saddr, sizeof(saddr));
  saddr.sin_family = AF_INET;
  // saddr.sin_addr.s_addr = inet_addr(client_addr.c_str());
  inet_aton(destIp.c_str(), (in_addr *)&saddr.sin_addr.s_addr);
  saddr.sin_port = htons((uint16_t)destPort);
  // send from the currently bound UDP port to the destination address
  const auto ret = sendto(mSocket, packet, packetSize, 0,
                          (const struct sockaddr *)&saddr, sizeof(saddr));
  if (ret < 0 || ret != packetSize) {
    get_console()->warn("Error sending packet of size:{} to {}:{} code:{} {}",
                        packetSize, destIp, destPort, ret, strerror(errno));
  }
}

void openhd::UDPReceiver::stopLooping() {
  receiving = false;
  // from
  // https://github.com/mavlink/MAVSDK/blob/main/src/mavsdk/core/udp_connection.cpp#L102
  shutdown(mSocket, SHUT_RDWR);
  close(mSocket);
}

void openhd::UDPReceiver::runInBackground() {
  if (receiverThread) {
    get_console()->warn(
        "Receiver thread is already running or has not been properly stopped");
    return;
  }
  receiving = true;
  receiverThread =
      std::make_unique<std::thread>(&UDPReceiver::loopUntilError, this);
}

void openhd::UDPReceiver::stopBackground() {
  stopLooping();
  if (receiverThread && receiverThread->joinable()) {
    receiverThread->join();
  }
  receiverThread = nullptr;
}

int openhd::openUdpSocketForReceiving(const std::string &address,
                                      const int port) {
  int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (fd < 0) {
    std::stringstream ss;
    ss << "Error opening socket " << port << " " << strerror(errno) << "\n";
    get_console()->warn(ss.str());
    return -1;
  }
  setSocketReuse(fd);
  struct sockaddr_in saddr {};
  bzero((char *)&saddr, sizeof(saddr));
  saddr.sin_family = AF_INET;
  // saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  inet_aton(address.c_str(), (in_addr *)&saddr.sin_addr.s_addr);
  saddr.sin_port = htons((unsigned short)port);
  if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
    std::stringstream ss;
    ss << "Bind error on socket " << address.c_str() << ":" << port << " "
       << strerror(errno) << "\n";
    get_console()->warn(ss.str());
    return -1;
  }
  return fd;
}

void openhd::setSocketReuse(int sockfd) {
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    get_console()->warn("Cannot set socket reuse");
  }
}
