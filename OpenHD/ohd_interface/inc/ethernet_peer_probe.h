#pragma once
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "openhd_ethernet_metrics.hpp"

// Separate diagnostic datagrams never touch the video/telemetry payload format.
// Air probes its configured Ground peer at most once per second. Old peers
// simply leave diagnostic fields unknown. No raw sockets or ICMP privileges.
class EthernetPeerProbe {
 public:
  using Clock = std::chrono::steady_clock;
  EthernetPeerProbe(bool air, std::function<std::string()> peer,
                    uint16_t port = 49892)
      : m_air(air),
        m_peer(std::move(peer)),
        m_port(port),
        m_thread([this] { run(); }) {}
  ~EthernetPeerProbe() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
  }
  void apply(openhd::ethernet::Metrics& metrics) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_window.apply(metrics,
                   Clock::now() - m_last_reply < std::chrono::seconds(3));
  }

 private:
  void run() {
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;
    if (!m_air) {
      sockaddr_in local{};
      local.sin_family = AF_INET;
      local.sin_port = htons(m_port);
      if (bind(fd, reinterpret_cast<sockaddr*>(&local), sizeof(local)) < 0) {
        close(fd);
        return;
      }
    }
    std::string current_peer;
    Clock::time_point sent{}, next_send{};
    uint64_t sequence = 0, pending = 0;
    sockaddr_in destination{};
    while (m_running) {
      const auto peer = m_peer();
      const auto now = Clock::now();
      if (peer != current_peer) {
        current_peer = peer;
        pending = 0;
        next_send = now;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_window.reset();
        m_last_reply = {};
      }
      destination = {};
      destination.sin_family = AF_INET;
      destination.sin_port = htons(m_port);
      const bool valid_peer =
          inet_pton(AF_INET, peer.c_str(), &destination.sin_addr) == 1;
      if (m_air && pending && now - sent >= std::chrono::milliseconds(1500)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_window.complete(false);
        pending = 0;
      }
      if (m_air && valid_peer && !pending && now >= next_send) {
        pending = ++sequence;
        sent = now;
        next_send = now + std::chrono::seconds(1);
        const auto packet =
            std::string("OHETH1 PING ") + std::to_string(pending);
        if (sendto(fd, packet.data(), packet.size(), 0,
                   reinterpret_cast<sockaddr*>(&destination),
                   sizeof(destination)) < 0)
          pending = 0;
      }
      pollfd descriptor{fd, POLLIN, 0};
      if (poll(&descriptor, 1, 50) <= 0) continue;
      char buffer[96];
      sockaddr_in sender{};
      socklen_t size = sizeof(sender);
      const auto count = recvfrom(fd, buffer, sizeof(buffer), 0,
                                  reinterpret_cast<sockaddr*>(&sender), &size);
      if (count <= 0 || !valid_peer ||
          sender.sin_addr.s_addr != destination.sin_addr.s_addr)
        continue;
      const std::string packet(buffer, count);
      if (!m_air && packet.rfind("OHETH1 PING ", 0) == 0) {
        // Only echo bounded requests from the configured peer, without
        // amplification.
        const auto reply = std::string("OHETH1 PONG ") + packet.substr(12);
        sendto(fd, reply.data(), reply.size(), 0,
               reinterpret_cast<sockaddr*>(&sender), size);
      } else if (m_air && pending && sender.sin_port == htons(m_port) &&
                 packet ==
                     std::string("OHETH1 PONG ") + std::to_string(pending)) {
        const auto received = Clock::now();
        const auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                            received - sent)
                            .count();
        std::lock_guard<std::mutex> lock(m_mutex);
        m_window.complete(true, static_cast<uint32_t>(us));
        m_last_reply = received;
        pending = 0;
      }
    }
    close(fd);
  }
  bool m_air;
  std::function<std::string()> m_peer;
  uint16_t m_port;
  std::atomic<bool> m_running{true};
  std::mutex m_mutex;
  openhd::ethernet::ProbeWindow m_window;
  Clock::time_point m_last_reply{};
  std::thread m_thread;
};
