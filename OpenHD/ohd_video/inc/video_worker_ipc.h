#pragma once

#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <vector>

namespace openhd::video_worker {
// Private, same-executable IPC, deliberately not a network/wire protocol.
constexpr std::size_t kChunkSize = 48 * 1024;
constexpr std::size_t kMaxFrameSize = 16 * 1024 * 1024;
enum class Type : uint32_t {
  Control,
  Settings,
  Status,
  FrameBegin,
  Fragment,
  Raw,
  FrameEnd,
  Stop,
  Log
};
struct Header {
  Type type;
  uint32_t sequence = 0;
  uint32_t flags = 0;
  uint32_t size = 0;
  int64_t created_ns = 0;
};
struct Control {
  int bitrate = -1;
  int qp_min = -1;
  int qp_max = -1;
  bool armed = false;
  bool capacity_limit = false;
  unsigned keyframe_revision = 0;
  int rotation_degree = -1;
  int flip = -1;
};

inline bool send_packet(int fd, Header header, const void* data = nullptr,
                        std::size_t size = 0) {
  if (size > kChunkSize) return false;
  header.size = static_cast<uint32_t>(size);
  iovec parts[2]{{&header, sizeof(header)}, {const_cast<void*>(data), size}};
  msghdr message{};
  message.msg_iov = parts;
  message.msg_iovlen = size ? 2 : 1;
  ssize_t result;
  do {
    result = sendmsg(fd, &message, MSG_DONTWAIT | MSG_NOSIGNAL);
  } while (result < 0 && errno == EINTR);
  return result == static_cast<ssize_t>(sizeof(header) + size);
}

// 1 = packet, 0 = would block, -1 = disconnected or invalid packet.
inline int receive_packet(int fd, Header& header, std::vector<uint8_t>& data) {
  std::array<uint8_t, sizeof(Header) + kChunkSize> buffer;
  ssize_t size;
  do {
    size = recv(fd, buffer.data(), buffer.size(), MSG_DONTWAIT | MSG_TRUNC);
  } while (size < 0 && errno == EINTR);
  if (size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return 0;
  if (size < static_cast<ssize_t>(sizeof(Header)) ||
      size > static_cast<ssize_t>(buffer.size()))
    return -1;
  std::memcpy(&header, buffer.data(), sizeof(header));
  if (header.size != size - sizeof(Header)) return -1;
  data.assign(buffer.begin() + sizeof(Header), buffer.begin() + size);
  return 1;
}
}  // namespace openhd::video_worker
