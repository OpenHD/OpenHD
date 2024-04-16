//
// Created by consti10 on 22.01.24.
//

#ifndef OPENHD_OPENHD_UDP_H
#define OPENHD_OPENHD_UDP_H

#include <netinet/in.h>

#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

//
// openhd UDP helpers
//
namespace openhd {
// Wrapper around an UDP port you can send data to
// opens port on construction, closes port on destruction
class UDPForwarder {
 public:
  explicit UDPForwarder(std::string client_addr1, int client_udp_port1);
  UDPForwarder(const UDPForwarder &) = delete;
  UDPForwarder &operator=(const UDPForwarder &) = delete;
  ~UDPForwarder();
  void forwardPacketViaUDP(const uint8_t *packet, std::size_t packetSize) const;

 private:
  struct sockaddr_in saddr {};
  int sockfd;

 public:
  const std::string client_addr;
  const int client_udp_port;
};

/**
 * Similar to UDP forwarder, but allows forwarding the same data to 0 or more
 * IP::Port tuples
 */
class UDPMultiForwarder {
 public:
  explicit UDPMultiForwarder() = default;
  UDPMultiForwarder(const UDPMultiForwarder &) = delete;
  UDPMultiForwarder &operator=(const UDPMultiForwarder &) = delete;
  /**
   * Start forwarding data to another IP::Port tuple
   */
  void addForwarder(const std::string &client_addr, int client_udp_port);
  /**
   * Remove an already existing udp forwarding instance.
   * Do nothing if such an instance is not found.
   */
  void removeForwarder(const std::string &client_addr, int client_udp_port);
  /**
   * Forward data to all added IP::Port tuples via UDP
   */
  void forwardPacketViaUDP(const uint8_t *packet, std::size_t packetSize);
  [[nodiscard]] const std::list<std::unique_ptr<UDPForwarder>> &getForwarders()
      const;

 private:
  // list of host::port tuples where we send the data to.
  std::list<std::unique_ptr<UDPForwarder>> udpForwarders;
  // modifying the list of forwarders must be thread-safe
  std::mutex udpForwardersLock;
};

// Open the specified port for udp receiving
// sets SO_REUSEADDR to true if possible
// throws a runtime exception if opening the socket fails
static int openUdpSocketForReceiving(const std::string &address, int port);

// Set the reuse flag on the socket, so it doesn't care if there is a broken
// down process still on the socket or not.
static void setSocketReuse(int sockfd);

class UDPReceiver {
 public:
  typedef std::function<void(const uint8_t *payload,
                             const std::size_t payloadSize)>
      OUTPUT_DATA_CALLBACK;
  static constexpr const size_t UDP_PACKET_MAX_SIZE = 65507;
  /**
   * Receive data from socket and forward it via callback until stopLooping() is
   * called
   */
  explicit UDPReceiver(std::string client_addr, int client_udp_port,
                       OUTPUT_DATA_CALLBACK cb);
  ~UDPReceiver();
  void loopUntilError();
  // Now this one is kinda special - for mavsdk we need to send messages from
  // the port we are listening on to a specific IP::PORT tuple (such that the
  // source address of the then received packet matches the address we are
  // listening on).
  void forwardPacketViaUDP(const std::string &destIp, int destPort,
                           const uint8_t *packet, std::size_t packetSize) const;
  void stopLooping();
  void runInBackground();
  void stopBackground();

 private:
  const OUTPUT_DATA_CALLBACK mCb;
  bool receiving = true;
  int mSocket;
  std::unique_ptr<std::thread> receiverThread = nullptr;
  // Limit receive error log to every 3 seconds
  std::chrono::steady_clock::time_point m_last_receive_error_log =
      std::chrono::steady_clock::now();
  int m_last_receive_error_log_skip_count = 0;
};

static const std::string ADDRESS_LOCALHOST = "127.0.0.1";
static const std::string ADDRESS_ANY = "0.0.0.0";
}  // namespace openhd

#endif  // OPENHD_OPENHD_UDP_H
