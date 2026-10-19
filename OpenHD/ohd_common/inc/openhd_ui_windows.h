#pragma once
// Private Windows implementation details for the shared curses dashboard.
#include "openhd_socket_platform.h"
#include <iphlpapi.h>
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <vector>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
namespace ui_os {
inline int dup(int fd) { return _dup(fd); }
inline int dup2(int source, int target) { return _dup2(source, target); }
inline int close(int fd) { return _close(fd); }
inline int read(int fd, void* buffer, unsigned size) { return _read(fd, buffer, size); }
inline long lseek(int fd, long offset, int origin) { return _lseek(fd, offset, origin); }
inline FILE* fdopen(int fd, const char* mode) { return _fdopen(fd, mode); }
inline int fileno(FILE* file) { return _fileno(file); }
inline int fsync(int fd) { return _commit(fd); }
inline int isatty(int fd) { return _isatty(fd); }
}
inline void localtime_r(const time_t* time, tm* result) { localtime_s(result, time); }
inline int setenv(const char* name, const char* value, int) { return _putenv_s(name, value); }
inline int unsetenv(const char* name) { return _putenv_s(name, ""); }

inline void sample_windows(std::string& uptime, std::string& cpu, std::string& ram,
                           std::string& disk, std::vector<std::string>& network,
                           double& cpu_ratio, double& ram_ratio, double& disk_ratio) {
  const auto seconds = GetTickCount64() / 1000;
  std::ostringstream elapsed;
  elapsed << seconds / 86400 << "d " << std::setfill('0') << std::setw(2)
          << seconds / 3600 % 24 << ':' << std::setw(2) << seconds / 60 % 60
          << ':' << std::setw(2) << seconds % 60;
  uptime = elapsed.str();
  FILETIME idle{}, kernel{}, user{};
  if (GetSystemTimes(&idle, &kernel, &user)) {
    auto ticks = [](FILETIME time) { return (uint64_t(time.dwHighDateTime) << 32) | time.dwLowDateTime; };
    static uint64_t old_total = 0, old_idle = 0;
    const auto total = ticks(kernel) + ticks(user), now_idle = ticks(idle);
    if (old_total && total > old_total) {
      cpu_ratio = std::clamp(1.0 - double(now_idle - old_idle) / double(total - old_total), 0.0, 1.0);
      cpu = std::to_string(int(cpu_ratio * 100)) + '%';
    }
    old_total = total; old_idle = now_idle;
  }
  auto amount = [](uint64_t bytes) {
    std::ostringstream text;
    if (bytes >= 1073741824) text << std::fixed << std::setprecision(1) << double(bytes) / 1073741824 << 'G';
    else text << bytes / 1048576 << 'M';
    return text.str();
  };
  MEMORYSTATUSEX memory{}; memory.dwLength = sizeof(memory);
  if (GlobalMemoryStatusEx(&memory)) {
    ram_ratio = double(memory.ullTotalPhys - memory.ullAvailPhys) / memory.ullTotalPhys;
    ram = amount(memory.ullTotalPhys - memory.ullAvailPhys) + '/' + amount(memory.ullTotalPhys);
  }
  ULARGE_INTEGER available{}, total{}, free{};
  const auto root = std::filesystem::current_path().root_path().wstring();
  if (GetDiskFreeSpaceExW(root.c_str(), &available, &total, &free) && total.QuadPart) {
    disk_ratio = double(total.QuadPart - free.QuadPart) / total.QuadPart;
    disk = amount(total.QuadPart - free.QuadPart) + '/' + amount(total.QuadPart);
  }
  network.clear();
  ULONG size = 15000;
  std::vector<unsigned char> storage(size);
  auto* adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(storage.data());
  auto result = GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
                                    nullptr, adapters, &size);
  if (result == ERROR_BUFFER_OVERFLOW) {
    storage.resize(size); adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(storage.data());
    result = GetAdaptersAddresses(AF_INET, 0, nullptr, adapters, &size);
  }
  if (result == NO_ERROR) for (auto* adapter = adapters; adapter; adapter = adapter->Next) {
    if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK || adapter->OperStatus != IfOperStatusUp) continue;
    for (auto* address = adapter->FirstUnicastAddress; address; address = address->Next) {
      if (address->Address.lpSockaddr->sa_family != AF_INET) continue;
      char text[INET_ADDRSTRLEN]{};
      const auto* in = reinterpret_cast<sockaddr_in*>(address->Address.lpSockaddr);
      if (inet_ntop(AF_INET, &in->sin_addr, text, sizeof(text))) network.push_back(text);
    }
  }
}
