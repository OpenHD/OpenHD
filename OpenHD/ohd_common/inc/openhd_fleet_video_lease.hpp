#pragma once

#include <arpa/inet.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include <chrono>
#include <cstring>
#include <string>
#include "openhd_ethernet_metrics.hpp"

namespace openhd {
// Fleet outputs fail closed if FleetControl cannot renew permission. The connected
// socket accepts replies only from the configured WireGuard destination.
class FleetVideoLease {
 public:
  FleetVideoLease(const std::string& host, int port, bool required)
      : required_(required) {
    if (!required) return;
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) return;
    fd_ = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd_ >= 0 && connect(fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
      close(fd_); fd_ = -1;
    }
  }
  ~FleetVideoLease() { if (fd_ >= 0) close(fd_); }
  FleetVideoLease(const FleetVideoLease&) = delete;
  FleetVideoLease& operator=(const FleetVideoLease&) = delete;
  bool allowed() {
    if (!required_) return true;
    if (fd_ < 0) return false;
    const auto now = std::chrono::steady_clock::now();
    std::array<char, 17> reply{};
    while (recv(fd_, reply.data(), reply.size(), 0) == 16) {
      if (!pending_ || std::memcmp(reply.data() + 8, request_.data() + 8, 8) != 0) continue;
      if (std::memcmp(reply.data(), "OHDFV1Y\0", 8) == 0) {
        const auto rtt = std::chrono::duration_cast<std::chrono::microseconds>(now - sent_).count();
        probes_.complete(true, static_cast<uint32_t>(std::min<int64_t>(UINT32_MAX - 1, rtt)));
        expires_ = now + std::chrono::milliseconds(2000); pending_ = false;
      } else if (std::memcmp(reply.data(), "OHDFV1N\0", 8) == 0) {
        expires_ = {}; pending_ = false;
      }
    }
    if (now >= next_) {
      if (pending_) probes_.complete(false);
      std::memcpy(request_.data(), "OHDFV1Q\0", 8);
      pending_ = getrandom(request_.data() + 8, 8, GRND_NONBLOCK) == 8;
      sent_ = now;
      if (pending_) send(fd_, request_.data(), request_.size(), 0);
      next_ = now + std::chrono::milliseconds(750);
    }
    return now < expires_;
  }
  bool send_rtp(const uint8_t* data, size_t size) {
    return allowed() && send(fd_, data, size, MSG_DONTWAIT) == static_cast<ssize_t>(size);
  }
  openhd::ethernet::Metrics metrics() const {
    openhd::ethernet::Metrics result;
    probes_.apply(result, std::chrono::steady_clock::now() < expires_);
    return result;
  }
 private:
  bool required_, pending_ = false;
  int fd_ = -1;
  std::array<char, 16> request_{};
  std::chrono::steady_clock::time_point next_{}, expires_{};
  std::chrono::steady_clock::time_point sent_{};
  openhd::ethernet::ProbeWindow probes_;
};
}  // namespace openhd
