#pragma once

// Winsock headers must precede windows.h. SOCKET is pointer-sized on Win64.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <stdexcept>
#include <string>
#include <system_error>

namespace openhd::net {
#ifdef _WIN32
using Socket = SOCKET;
inline constexpr Socket invalid_socket = INVALID_SOCKET;
inline void initialize() {
  struct Runtime {
    Runtime() {
      WSADATA data{};
      const int error = WSAStartup(MAKEWORD(2, 2), &data);
      if (error != 0) throw std::runtime_error("WSAStartup: " + std::to_string(error));
    }
    ~Runtime() { WSACleanup(); }
  };
  static Runtime runtime;
}
inline void close_socket(Socket socket) { closesocket(socket); }
inline std::string error_message() {
  return std::system_category().message(WSAGetLastError());
}
#else
using Socket = int;
inline constexpr Socket invalid_socket = -1;
inline void initialize() {}
inline void close_socket(Socket socket) { ::close(socket); }
inline std::string error_message() {
  return std::generic_category().message(errno);
}
#endif
}  // namespace openhd::net
