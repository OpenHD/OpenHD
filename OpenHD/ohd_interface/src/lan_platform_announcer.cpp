#include "lan_platform_announcer.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <memory>
#include <string>

namespace {
// Keep in sync with QOpenHD's LanPlatformDiscovery.
constexpr uint16_t kDiscoveryPort = 42042;
constexpr auto kInterval = std::chrono::seconds(3);

bool is_wired_interface(const char* name) {
  const std::string path = std::string("/sys/class/net/") + name;
  return access((path + "/device").c_str(), F_OK) == 0 &&
         access((path + "/wireless").c_str(), F_OK) != 0 &&
         access((path + "/phy80211").c_str(), F_OK) != 0;
}

void announce(int socket_fd, const char* prefix) {
  ifaddrs* interfaces = nullptr;
  if (getifaddrs(&interfaces) != 0) return;
  const std::unique_ptr<ifaddrs, decltype(&freeifaddrs)> entries(interfaces,
                                                               freeifaddrs);
  for (const auto* entry = entries.get(); entry; entry = entry->ifa_next) {
    constexpr unsigned required = IFF_UP | IFF_RUNNING | IFF_BROADCAST;
    if ((entry->ifa_flags & required) != required || !entry->ifa_addr ||
        entry->ifa_addr->sa_family != AF_INET || !entry->ifa_broadaddr ||
        !is_wired_interface(entry->ifa_name)) {
      continue;
    }

    const auto source = reinterpret_cast<const sockaddr_in*>(entry->ifa_addr);
    auto target = *reinterpret_cast<const sockaddr_in*>(entry->ifa_broadaddr);
    target.sin_port = htons(kDiscoveryPort);
    if (target.sin_addr.s_addr == INADDR_ANY) continue;

    char ip[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &source->sin_addr, ip, sizeof(ip))) continue;
    std::string payload = std::string(prefix) + ip;

    const auto interface_index = if_nametoindex(entry->ifa_name);
    if (!interface_index) continue;
    in_pktinfo route{};
    route.ipi_ifindex = static_cast<int>(interface_index);
    route.ipi_spec_dst = source->sin_addr;
    // Keep each announcement on its own interface when subnets overlap.
    alignas(cmsghdr) char control[CMSG_SPACE(sizeof(route))]{};
    iovec data{payload.data(), payload.size()};
    msghdr message{};
    message.msg_name = &target;
    message.msg_namelen = sizeof(target);
    message.msg_iov = &data;
    message.msg_iovlen = 1;
    message.msg_control = control;
    message.msg_controllen = sizeof(control);
    auto* header = CMSG_FIRSTHDR(&message);
    header->cmsg_level = IPPROTO_IP;
    header->cmsg_type = IP_PKTINFO;
    header->cmsg_len = CMSG_LEN(sizeof(route));
    std::memcpy(CMSG_DATA(header), &route, sizeof(route));
    sendmsg(socket_fd, &message, MSG_DONTWAIT);
  }
}
}  // namespace

LanPlatformAnnouncer::LanPlatformAnnouncer(bool is_air)
    : m_thread(&LanPlatformAnnouncer::run, this, is_air) {}

LanPlatformAnnouncer::~LanPlatformAnnouncer() {
  {
    const std::lock_guard lock(m_mutex);
    m_stopping = true;
  }
  m_stop_requested.notify_one();
  m_thread.join();
}

void LanPlatformAnnouncer::run(bool is_air) {
  const int socket_fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if (socket_fd < 0) return;

  const int enabled = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &enabled,
                 sizeof(enabled)) == 0) {
    const char* prefix = is_air ? "PLATFORM:AIR:" : "PLATFORM:GROUND:";
    std::unique_lock lock(m_mutex);
    while (!m_stopping) {
      lock.unlock();
      announce(socket_fd, prefix);
      lock.lock();
      m_stop_requested.wait_for(lock, kInterval, [this] { return m_stopping; });
    }
  }
  close(socket_fd);
}
