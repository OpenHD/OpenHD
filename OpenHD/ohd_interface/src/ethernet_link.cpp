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

#include "ethernet_link.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include "config_paths.h"
#include "openhd_action_handler.h"
#include "openhd_config.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "openhd_util_time.h"

namespace {
constexpr int ETHERNET_DISCOVERY_PORT = 49891;
constexpr const char* ETHERNET_DISCOVERY_PING = "OPENHD_ETHERNET_V1 PING";
constexpr const char* ETHERNET_DISCOVERY_PONG_PREFIX =
    "OPENHD_ETHERNET_V1 PONG";

int16_t clamp_int16(int value) {
  if (value > std::numeric_limits<int16_t>::max()) {
    return std::numeric_limits<int16_t>::max();
  }
  if (value < std::numeric_limits<int16_t>::min()) {
    return std::numeric_limits<int16_t>::min();
  }
  return static_cast<int16_t>(value);
}

int32_t clamp_int32(int64_t value) {
  if (value > std::numeric_limits<int32_t>::max()) {
    return std::numeric_limits<int32_t>::max();
  }
  if (value < std::numeric_limits<int32_t>::min()) {
    return std::numeric_limits<int32_t>::min();
  }
  return static_cast<int32_t>(value);
}

void set_receive_timeout(int socket_fd, int milliseconds) {
  timeval timeout{};
  timeout.tv_sec = milliseconds / 1000;
  timeout.tv_usec = (milliseconds % 1000) * 1000;
  setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

std::set<uint32_t> local_broadcast_addresses() {
  std::set<uint32_t> result{INADDR_BROADCAST};
  ifaddrs* interfaces = nullptr;
  if (getifaddrs(&interfaces) != 0) {
    return result;
  }
  for (auto* current = interfaces; current != nullptr;
       current = current->ifa_next) {
    if (current->ifa_addr == nullptr ||
        current->ifa_addr->sa_family != AF_INET ||
        (current->ifa_flags & IFF_UP) == 0) {
      continue;
    }
    if ((current->ifa_flags & IFF_BROADCAST) != 0 &&
        current->ifa_broadaddr != nullptr) {
      const auto* broadcast =
          reinterpret_cast<const sockaddr_in*>(current->ifa_broadaddr);
      result.insert(broadcast->sin_addr.s_addr);
    } else if ((current->ifa_flags & IFF_POINTOPOINT) != 0 &&
               current->ifa_dstaddr != nullptr) {
      const auto* destination =
          reinterpret_cast<const sockaddr_in*>(current->ifa_dstaddr);
      result.insert(destination->sin_addr.s_addr);
    }
  }
  freeifaddrs(interfaces);
  // Useful for development and for a ground and air process on one host.
  result.insert(htonl(INADDR_LOOPBACK));
  return result;
}

bool has_usable_ipv4_address(const std::string& interface_name) {
  ifaddrs* interfaces = nullptr;
  if (getifaddrs(&interfaces) != 0) {
    return false;
  }
  bool found = false;
  for (auto* current = interfaces; current != nullptr;
       current = current->ifa_next) {
    if (current->ifa_addr == nullptr ||
        current->ifa_addr->sa_family != AF_INET ||
        interface_name != current->ifa_name) {
      continue;
    }
    const auto* address =
        reinterpret_cast<const sockaddr_in*>(current->ifa_addr);
    const uint32_t host_address = ntohl(address->sin_addr.s_addr);
    const bool loopback = (host_address & 0xff000000U) == 0x7f000000U;
    const bool link_local = (host_address & 0xffff0000U) == 0xa9fe0000U;
    if (host_address != INADDR_ANY && !loopback && !link_local) {
      found = true;
      break;
    }
  }
  freeifaddrs(interfaces);
  return found;
}

bool has_carrier(const std::string& interface_name) {
  const auto carrier = OHDFilesystemUtil::opt_read_file(
      fmt::format("/sys/class/net/{}/carrier", interface_name));
  return carrier.has_value() && carrier->find('1') != std::string::npos;
}

std::vector<std::string> ethernet_interface_names(
    const openhd::Config& config) {
  if (openhd::nw_ethernet_card_manual_active(config)) {
    const auto configured_path =
        fmt::format("/sys/class/net/{}", config.NW_ETHERNET_CARD);
    if (OHDFilesystemUtil::exists(configured_path)) {
      return {config.NW_ETHERNET_CARD};
    }
  }
  auto interfaces =
      OHDFilesystemUtil::getAllEntriesFilenameOnlyInDirectory("/sys/class/net");
  interfaces.erase(
      std::remove_if(interfaces.begin(), interfaces.end(),
                     [](const std::string& name) {
                       return !OHDUtil::startsWith(name, "en") &&
                              !OHDUtil::startsWith(name, "eth");
                     }),
      interfaces.end());
  std::sort(interfaces.begin(), interfaces.end());
  return interfaces;
}

std::optional<std::pair<int, int>> parse_discovery_pong(
    const std::string& payload) {
  std::istringstream stream(payload);
  std::string protocol;
  std::string command;
  std::string video_field;
  std::string telemetry_field;
  if (!(stream >> protocol >> command >> video_field >> telemetry_field) ||
      protocol != "OPENHD_ETHERNET_V1" || command != "PONG") {
    return std::nullopt;
  }
  constexpr const char* VIDEO_PREFIX = "video=";
  constexpr const char* TELEMETRY_PREFIX = "telemetry=";
  if (video_field.rfind(VIDEO_PREFIX, 0) != 0 ||
      telemetry_field.rfind(TELEMETRY_PREFIX, 0) != 0) {
    return std::nullopt;
  }
  try {
    const int video_port =
        std::stoi(video_field.substr(std::strlen(VIDEO_PREFIX)));
    const int telemetry_port =
        std::stoi(telemetry_field.substr(std::strlen(TELEMETRY_PREFIX)));
    if (video_port < 1 || video_port > 65535 || telemetry_port < 1 ||
        telemetry_port > 65535) {
      return std::nullopt;
    }
    return std::make_pair(video_port, telemetry_port);
  } catch (...) {
    return std::nullopt;
  }
}
}  // namespace

static std::string ETHERNET_FILE_PATH =
    std::string(getConfigBasePath()) + "ethernet.txt";

EthernetLink::EthernetLink(const openhd::Config& config, OHDProfile profile)
    : m_config(config), m_profile(profile) {
  initialize(false);
}

EthernetLink::EthernetLink(OHDProfile profile)
    : EthernetLink(openhd::load_config(), profile) {}

EthernetLink::EthernetLink(OHDProfile profile, bool auto_discovery)
    : m_profile(profile), m_config(openhd::load_config()) {
  initialize(auto_discovery);
}

void EthernetLink::initialize(bool auto_discovery) {
  m_auto_discovery = auto_discovery;
  std::cout << "ethernet starting " << std::endl;

  if (auto_discovery) {
    VIDEO_PORT = m_config.VIDEO_PORT;
    TELEMETRY_PORT = m_config.TELEMETRY_PORT;
    std::cout << "Ethernet auto-discovery using video port " << VIDEO_PORT
              << " and telemetry port " << TELEMETRY_PORT << std::endl;
  } else if (OHDFilesystemUtil::exists(ETHERNET_FILE_PATH)) {
    std::cout << "ethernet config load " << std::endl;

    try {
      // Assign to class member variables instead of local static variables
      GROUND_UNIT_IP = m_config.GROUND_UNIT_IP;
      AIR_UNIT_IP = m_config.AIR_UNIT_IP;
      VIDEO_PORT = m_config.VIDEO_PORT;
      TELEMETRY_PORT = m_config.TELEMETRY_PORT;

      // Debugging the values after assignment
      std::cout << "Assigned ethernet parameters:" << std::endl;
      std::cout << "  GROUND_UNIT_IP: " << GROUND_UNIT_IP << std::endl;
      std::cout << "  AIR_UNIT_IP: " << AIR_UNIT_IP << std::endl;
      std::cout << "  VIDEO_PORT: " << VIDEO_PORT << std::endl;
      std::cout << "  TELEMETRY_PORT: " << TELEMETRY_PORT << std::endl;
    } catch (const std::exception& ex) {
      std::cerr << "Failed to read ethernet parameters: " << ex.what()
                << std::endl;
      throw;
    }
  } else {
    std::cerr << "Ethernet parameters not found. Using default configuration."
              << std::endl;
  }

  // Initialize either air or ground unit based on the profile
  if (m_profile.is_air) {
    initialize_air_unit();
  } else {
    initialize_ground_unit();
  }
  if (m_auto_discovery) {
    start_discovery();
  }
  start_stats_thread();
}

EthernetLink::~EthernetLink() {
  stop_discovery();
  stop_stats_thread();
  // Stop background receivers
  if (m_video_rx) m_video_rx->stopBackground();
  if (m_telemetry_rx) m_telemetry_rx->stopBackground();
}

void EthernetLink::initialize_air_unit() {
  if (!m_auto_discovery) {
    configure_peer(GROUND_UNIT_IP, VIDEO_PORT, TELEMETRY_PORT);
  }
  m_telemetry_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", TELEMETRY_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_telemetry_data(data, len);  // Process incoming telemetry
      });

  // Start telemetry receiver in the background
  if (m_telemetry_rx) m_telemetry_rx->runInBackground();
}

void EthernetLink::initialize_ground_unit() {
  // Initialize video receiver for receiving video from the air unit
  m_video_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", VIDEO_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_video_data(0, data, len);  // Process incoming video
      });

  if (!m_auto_discovery) {
    configure_peer(AIR_UNIT_IP, VIDEO_PORT, TELEMETRY_PORT);
  }
  m_telemetry_rx = std::make_unique<openhd::UDPReceiver>(
      "0.0.0.0", TELEMETRY_PORT, [this](const uint8_t* data, std::size_t len) {
        handle_telemetry_data(data, len);  // Process incoming telemetry
      });

  // Start video and telemetry receivers in the background
  if (m_video_rx) m_video_rx->runInBackground();
  if (m_telemetry_rx) m_telemetry_rx->runInBackground();
}

void EthernetLink::configure_peer(const std::string& peer_ip, int video_port,
                                  int telemetry_port) {
  std::lock_guard<std::mutex> lock(m_forwarders_mutex);
  const bool endpoints_ready =
      m_telemetry_tx &&
      (m_profile.is_air
           ? static_cast<bool>(m_video_tx)
           : (!m_auto_discovery || (m_video_rx && m_telemetry_rx)));
  if (m_peer_ip == peer_ip && VIDEO_PORT == video_port &&
      TELEMETRY_PORT == telemetry_port && endpoints_ready) {
    return;
  }
  const bool rebind_ground_receivers =
      m_auto_discovery && m_profile.is_ground() &&
      (VIDEO_PORT != video_port || TELEMETRY_PORT != telemetry_port ||
       !m_video_rx || !m_telemetry_rx);
  if (rebind_ground_receivers) {
    if (m_video_rx) {
      m_video_rx->stopBackground();
      m_video_rx.reset();
    }
    if (m_telemetry_rx) {
      m_telemetry_rx->stopBackground();
      m_telemetry_rx.reset();
    }
  }
  m_peer_ip = peer_ip;
  VIDEO_PORT = video_port;
  TELEMETRY_PORT = telemetry_port;
  m_telemetry_tx =
      std::make_shared<openhd::UDPForwarder>(peer_ip, TELEMETRY_PORT);
  if (m_profile.is_air) {
    m_video_tx = std::make_shared<openhd::UDPForwarder>(peer_ip, VIDEO_PORT);
  } else if (rebind_ground_receivers) {
    try {
      m_video_rx = std::make_unique<openhd::UDPReceiver>(
          "0.0.0.0", VIDEO_PORT,
          [this](const uint8_t* data, std::size_t len) {
            handle_video_data(0, data, len);
          });
      m_telemetry_rx = std::make_unique<openhd::UDPReceiver>(
          "0.0.0.0", TELEMETRY_PORT,
          [this](const uint8_t* data, std::size_t len) {
            handle_telemetry_data(data, len);
          });
      m_video_rx->runInBackground();
      m_telemetry_rx->runInBackground();
    } catch (...) {
      m_video_rx.reset();
      m_telemetry_rx.reset();
      throw;
    }
  }
  openhd::log::get_default()->info(
      "Ethernet link configured peer {} (video UDP {}, telemetry UDP {})",
      peer_ip, VIDEO_PORT, TELEMETRY_PORT);
}

void EthernetLink::start_discovery() {
  if (m_discovery_running.exchange(true)) {
    return;
  }
  m_discovery_thread = std::thread([this]() { discovery_loop(); });
}

void EthernetLink::stop_discovery() {
  m_discovery_running = false;
  if (m_discovery_thread.joinable()) {
    m_discovery_thread.join();
  }
}

void EthernetLink::discovery_loop() {
  configure_static_address_if_dhcp_is_unavailable();
  const int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd < 0) {
    openhd::log::get_default()->error("Cannot create Ethernet discovery socket");
    return;
  }
  const int enabled = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
  setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled));
  set_receive_timeout(socket_fd, 500);

  if (m_profile.is_air) {
    sockaddr_in listen_address{};
    listen_address.sin_family = AF_INET;
    listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_address.sin_port = htons(ETHERNET_DISCOVERY_PORT);
    if (bind(socket_fd, reinterpret_cast<sockaddr*>(&listen_address),
             sizeof(listen_address)) != 0) {
      openhd::log::get_default()->error(
          "Cannot bind Ethernet discovery service to UDP {}: {}",
          ETHERNET_DISCOVERY_PORT, std::strerror(errno));
      close(socket_fd);
      return;
    }
    openhd::log::get_default()->info(
        "Ethernet fallback waiting for ground discovery on UDP {}",
        ETHERNET_DISCOVERY_PORT);
    air_discovery_loop(socket_fd);
  } else {
    ground_discovery_loop(socket_fd);
  }
  close(socket_fd);
}

void EthernetLink::configure_static_address_if_dhcp_is_unavailable() {
  const auto interfaces = ethernet_interface_names(m_config);
  if (interfaces.empty()) {
    openhd::log::get_default()->warn(
        "Ethernet fallback found no Ethernet interface");
    return;
  }

  for (const auto& interface_name : interfaces) {
    OHDUtil::run_command("ip",
                         {"link", "set", "dev", interface_name, "up"}, false);
  }

  // Give an existing DHCP client time to finish before adding a direct-link
  // address. This runs on the discovery thread and does not block OpenHD.
  for (int attempt = 0; attempt < 10 && m_discovery_running; ++attempt) {
    const bool any_usable_address =
        std::any_of(interfaces.begin(), interfaces.end(),
                    [](const std::string& interface_name) {
                      return has_usable_ipv4_address(interface_name);
                    });
    bool waiting_for_dhcp = false;
    for (const auto& interface_name : interfaces) {
      if (!has_usable_ipv4_address(interface_name) &&
          (has_carrier(interface_name) ||
           openhd::nw_ethernet_card_manual_active(m_config))) {
        waiting_for_dhcp = true;
        break;
      }
    }
    // With no address anywhere, also wait for carrier negotiation before
    // deciding this is a direct link without DHCP.
    waiting_for_dhcp = waiting_for_dhcp || !any_usable_address;
    if (!waiting_for_dhcp) {
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  if (!m_discovery_running) {
    return;
  }

  std::optional<std::string> fallback_interface;
  for (const auto& interface_name : interfaces) {
    if (!has_usable_ipv4_address(interface_name) &&
        has_carrier(interface_name)) {
      fallback_interface = interface_name;
      break;
    }
  }
  const bool any_usable_address =
      std::any_of(interfaces.begin(), interfaces.end(),
                  [](const std::string& interface_name) {
                    return has_usable_ipv4_address(interface_name);
                  });
  if (!fallback_interface.has_value() && !any_usable_address) {
    fallback_interface = interfaces.front();
  }
  if (!fallback_interface.has_value()) {
    return;
  }

  const std::string fallback_ip =
      m_profile.is_air ? "192.168.8.1" : "192.168.8.2";
  const auto result = OHDUtil::run_command(
      "ip", {"address", "replace", fallback_ip + "/24", "dev",
             fallback_interface.value()},
      false);
  if (result == 0) {
    openhd::log::get_default()->warn(
        "No DHCP address found on {}; using static Ethernet address {}/24",
        fallback_interface.value(), fallback_ip);
  } else {
    openhd::log::get_default()->error(
        "Cannot configure static Ethernet address {}/24 on {}", fallback_ip,
        fallback_interface.value());
  }
}

void EthernetLink::air_discovery_loop(int socket_fd) {
  std::array<char, 256> buffer{};
  while (m_discovery_running) {
    sockaddr_in sender{};
    socklen_t sender_length = sizeof(sender);
    const auto length =
        recvfrom(socket_fd, buffer.data(), buffer.size() - 1, 0,
                 reinterpret_cast<sockaddr*>(&sender), &sender_length);
    if (length <= 0) {
      continue;
    }
    const std::string request(buffer.data(), static_cast<std::size_t>(length));
    if (request != ETHERNET_DISCOVERY_PING) {
      continue;
    }
    std::array<char, INET_ADDRSTRLEN> sender_ip{};
    if (inet_ntop(AF_INET, &sender.sin_addr, sender_ip.data(),
                  sender_ip.size()) == nullptr) {
      continue;
    }
    configure_peer(sender_ip.data(), VIDEO_PORT, TELEMETRY_PORT);
    const auto response =
        fmt::format("{} video={} telemetry={}", ETHERNET_DISCOVERY_PONG_PREFIX,
                    VIDEO_PORT, TELEMETRY_PORT);
    sendto(socket_fd, response.data(), response.size(), 0,
           reinterpret_cast<sockaddr*>(&sender), sender_length);
  }
}

void EthernetLink::ground_discovery_loop(int socket_fd) {
  openhd::log::get_default()->info(
      "Ethernet fallback scanning local networks on UDP {}",
      ETHERNET_DISCOVERY_PORT);
  std::array<char, 256> buffer{};
  while (m_discovery_running) {
    for (const auto address : local_broadcast_addresses()) {
      sockaddr_in destination{};
      destination.sin_family = AF_INET;
      destination.sin_addr.s_addr = address;
      destination.sin_port = htons(ETHERNET_DISCOVERY_PORT);
      sendto(socket_fd, ETHERNET_DISCOVERY_PING,
             std::strlen(ETHERNET_DISCOVERY_PING), 0,
             reinterpret_cast<sockaddr*>(&destination), sizeof(destination));
    }

    const auto receive_until =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(750);
    while (m_discovery_running &&
           std::chrono::steady_clock::now() < receive_until) {
      sockaddr_in sender{};
      socklen_t sender_length = sizeof(sender);
      const auto length =
          recvfrom(socket_fd, buffer.data(), buffer.size() - 1, 0,
                   reinterpret_cast<sockaddr*>(&sender), &sender_length);
      if (length <= 0) {
        continue;
      }
      const std::string response(buffer.data(),
                                 static_cast<std::size_t>(length));
      const auto ports = parse_discovery_pong(response);
      if (!ports.has_value()) {
        continue;
      }
      std::array<char, INET_ADDRSTRLEN> sender_ip{};
      if (inet_ntop(AF_INET, &sender.sin_addr, sender_ip.data(),
                    sender_ip.size()) == nullptr) {
        continue;
      }
      try {
        configure_peer(sender_ip.data(), ports->first, ports->second);
      } catch (const std::exception& error) {
        openhd::log::get_default()->warn(
            "Cannot configure discovered Ethernet air unit {}: {}",
            sender_ip.data(), error.what());
      }
    }
    for (int i = 0; i < 5 && m_discovery_running; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

void EthernetLink::transmit_telemetry_data(TelemetryTxPacket packet) {
  // Send telemetry data to the destination
  std::shared_ptr<openhd::UDPForwarder> telemetry_tx;
  {
    std::lock_guard<std::mutex> lock(m_forwarders_mutex);
    telemetry_tx = m_telemetry_tx;
  }
  if (telemetry_tx) {
    telemetry_tx->forwardPacketViaUDP(packet.data->data(), packet.data->size());
    m_tx_total_bytes.fetch_add(static_cast<uint64_t>(packet.data->size()),
                               std::memory_order_relaxed);
    m_tx_total_packets.fetch_add(1, std::memory_order_relaxed);
    m_tx_tele_bytes.fetch_add(static_cast<uint64_t>(packet.data->size()),
                              std::memory_order_relaxed);
    m_tx_tele_packets.fetch_add(1, std::memory_order_relaxed);
  }
}

void EthernetLink::transmit_video_data(
    int stream_index,
    const openhd::FragmentedVideoFrame& fragmented_video_frame) {
  // Send video data fragments to the destination
  std::shared_ptr<openhd::UDPForwarder> video_tx;
  {
    std::lock_guard<std::mutex> lock(m_forwarders_mutex);
    video_tx = m_video_tx;
  }
  if (video_tx) {
    for (const auto& fragment : fragmented_video_frame.rtp_fragments) {
      video_tx->forwardPacketViaUDP(fragment->data(), fragment->size());
      m_video_bitrate_meter.on_tx_fragment(
          stream_index, static_cast<uint64_t>(fragment->size()));
      m_tx_total_bytes.fetch_add(static_cast<uint64_t>(fragment->size()),
                                 std::memory_order_relaxed);
      m_tx_total_packets.fetch_add(1, std::memory_order_relaxed);
    }
  }
}

void EthernetLink::transmit_audio_data(
    const openhd::AudioPacket& audio_packet) {
  // Currently not implemented for EthernetLink
}

void EthernetLink::handle_video_data(int stream_index, const uint8_t* data,
                                     int data_len) {
  m_rx_total_bytes.fetch_add(static_cast<uint64_t>(data_len),
                             std::memory_order_relaxed);
  m_rx_total_packets.fetch_add(1, std::memory_order_relaxed);
  m_last_rx_packet_ts_ms.store(openhd::util::steady_clock_time_epoch_ms(),
                               std::memory_order_relaxed);
  // Forward incoming video data to the upper layer
  on_receive_video_data(stream_index, data, data_len);
}

void EthernetLink::handle_telemetry_data(const uint8_t* data, int data_len) {
  m_rx_total_bytes.fetch_add(static_cast<uint64_t>(data_len),
                             std::memory_order_relaxed);
  m_rx_total_packets.fetch_add(1, std::memory_order_relaxed);
  m_rx_tele_bytes.fetch_add(static_cast<uint64_t>(data_len),
                            std::memory_order_relaxed);
  m_rx_tele_packets.fetch_add(1, std::memory_order_relaxed);
  m_last_rx_packet_ts_ms.store(openhd::util::steady_clock_time_epoch_ms(),
                               std::memory_order_relaxed);
  // Forward incoming telemetry data to the upper layer
  auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
  on_receive_telemetry_data(shared);
}

void EthernetLink::start_stats_thread() {
  if (m_stats_running.exchange(true)) {
    return;
  }
  m_stats_thread = std::thread([this]() { stats_loop(); });
}

void EthernetLink::stop_stats_thread() {
  m_stats_running = false;
  if (m_stats_thread.joinable()) {
    m_stats_thread.join();
  }
}

void EthernetLink::stats_loop() {
  while (m_stats_running) {
    update_link_stats();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void EthernetLink::update_link_stats() {
  // WB publishes its own detailed video stats. Never overwrite if WB is active.
  if (openhd::LinkActionHandler::instance().wb_get_supported_channels !=
      nullptr) {
    return;
  }
  const int64_t now_ms = openhd::util::steady_clock_time_epoch_ms();
  if (m_last_stats_ts_ms == 0) {
    m_last_stats_ts_ms = now_ms;
    m_last_stats_tx_bytes = m_tx_total_bytes.load(std::memory_order_relaxed);
    m_last_stats_tx_packets = m_tx_total_packets.load(std::memory_order_relaxed);
    m_last_stats_rx_bytes = m_rx_total_bytes.load(std::memory_order_relaxed);
    m_last_stats_rx_packets = m_rx_total_packets.load(std::memory_order_relaxed);
    m_last_stats_tx_tele_bytes = m_tx_tele_bytes.load(std::memory_order_relaxed);
    m_last_stats_tx_tele_packets =
        m_tx_tele_packets.load(std::memory_order_relaxed);
    m_last_stats_rx_tele_bytes = m_rx_tele_bytes.load(std::memory_order_relaxed);
    m_last_stats_rx_tele_packets =
        m_rx_tele_packets.load(std::memory_order_relaxed);
    return;
  }
  const int64_t dt_ms = now_ms - m_last_stats_ts_ms;
  if (dt_ms <= 0) {
    return;
  }
  const uint64_t tx_bytes = m_tx_total_bytes.load(std::memory_order_relaxed);
  const uint64_t tx_packets =
      m_tx_total_packets.load(std::memory_order_relaxed);
  const uint64_t rx_bytes = m_rx_total_bytes.load(std::memory_order_relaxed);
  const uint64_t rx_packets =
      m_rx_total_packets.load(std::memory_order_relaxed);
  const uint64_t tx_tele_bytes =
      m_tx_tele_bytes.load(std::memory_order_relaxed);
  const uint64_t tx_tele_packets =
      m_tx_tele_packets.load(std::memory_order_relaxed);
  const uint64_t rx_tele_bytes =
      m_rx_tele_bytes.load(std::memory_order_relaxed);
  const uint64_t rx_tele_packets =
      m_rx_tele_packets.load(std::memory_order_relaxed);

  const uint64_t d_tx_bytes = tx_bytes - m_last_stats_tx_bytes;
  const uint64_t d_tx_packets = tx_packets - m_last_stats_tx_packets;
  const uint64_t d_rx_bytes = rx_bytes - m_last_stats_rx_bytes;
  const uint64_t d_rx_packets = rx_packets - m_last_stats_rx_packets;
  const uint64_t d_tx_tele_bytes = tx_tele_bytes - m_last_stats_tx_tele_bytes;
  const uint64_t d_tx_tele_packets =
      tx_tele_packets - m_last_stats_tx_tele_packets;
  const uint64_t d_rx_tele_bytes = rx_tele_bytes - m_last_stats_rx_tele_bytes;
  const uint64_t d_rx_tele_packets =
      rx_tele_packets - m_last_stats_rx_tele_packets;

  const int64_t scale = 1000;
  const int64_t tx_bps =
      static_cast<int64_t>((d_tx_bytes * 8 * scale) / dt_ms);
  const int64_t rx_bps =
      static_cast<int64_t>((d_rx_bytes * 8 * scale) / dt_ms);
  const int64_t tx_pps =
      static_cast<int64_t>((d_tx_packets * scale) / dt_ms);
  const int64_t rx_pps =
      static_cast<int64_t>((d_rx_packets * scale) / dt_ms);
  const int64_t tx_tele_bps =
      static_cast<int64_t>((d_tx_tele_bytes * 8 * scale) / dt_ms);
  const int64_t rx_tele_bps =
      static_cast<int64_t>((d_rx_tele_bytes * 8 * scale) / dt_ms);
  const int64_t tx_tele_pps =
      static_cast<int64_t>((d_tx_tele_packets * scale) / dt_ms);
  const int64_t rx_tele_pps =
      static_cast<int64_t>((d_rx_tele_packets * scale) / dt_ms);

  m_last_stats_ts_ms = now_ms;
  m_last_stats_tx_bytes = tx_bytes;
  m_last_stats_tx_packets = tx_packets;
  m_last_stats_rx_bytes = rx_bytes;
  m_last_stats_rx_packets = rx_packets;
  m_last_stats_tx_tele_bytes = tx_tele_bytes;
  m_last_stats_tx_tele_packets = tx_tele_packets;
  m_last_stats_rx_tele_bytes = rx_tele_bytes;
  m_last_stats_rx_tele_packets = rx_tele_packets;

  openhd::link_statistics::StatsAirGround stats{};
  stats.is_air = m_profile.is_air;
  stats.ready = true;
  stats.monitor_mode_link.curr_tx_bps = clamp_int32(tx_bps);
  stats.monitor_mode_link.curr_rx_bps = clamp_int32(rx_bps);
  stats.monitor_mode_link.curr_tx_pps = clamp_int16(static_cast<int>(tx_pps));
  stats.monitor_mode_link.curr_rx_pps = clamp_int16(static_cast<int>(rx_pps));
  const int64_t last_rx_ts =
      m_last_rx_packet_ts_ms.load(std::memory_order_relaxed);
  const bool rx_ok = last_rx_ts > 0 && (now_ms - last_rx_ts) <= 5000;
  const auto bitfield = openhd::link_statistics::MonitorModeLinkBitfield{
      false, false, false, rx_ok};
  stats.monitor_mode_link.bitfield =
      openhd::link_statistics::write_monitor_link_bitfield(bitfield);
  stats.telemetry.curr_tx_bps = clamp_int32(tx_tele_bps);
  stats.telemetry.curr_rx_bps = clamp_int32(rx_tele_bps);
  stats.telemetry.curr_tx_pps = clamp_int16(static_cast<int>(tx_tele_pps));
  stats.telemetry.curr_rx_pps = clamp_int16(static_cast<int>(rx_tele_pps));

  if (m_profile.is_air) {
    for (int i = 0; i < openhd::non_wb::VideoBitrateMeter::kMaxStreams; ++i) {
      const auto sample = m_video_bitrate_meter.sample_stream(i, now_ms);
      openhd::link_statistics::Xmavlink_openhd_stats_wb_video_air_t air_video{};
      const auto cam_stats = openhd::LinkActionHandler::instance().get_cam_info(i);
      if (sample.total_packets == 0 && cam_stats.measured_bitrate_bps == 0) {
        continue;
      }
      air_video.link_index = static_cast<uint8_t>(i);
      air_video.curr_recommended_bitrate =
          cam_stats.target_bitrate_kbits > 0 ? cam_stats.target_bitrate_kbits
                                             : cam_stats.encoding_bitrate_kbits;
      if (cam_stats.measured_bitrate_bps > 0) {
        air_video.curr_measured_encoder_bitrate = static_cast<int32_t>(
            std::min<uint32_t>(cam_stats.measured_bitrate_bps,
                               static_cast<uint32_t>(
                                   std::numeric_limits<int32_t>::max())));
      } else {
        air_video.curr_measured_encoder_bitrate = 0;
      }
      air_video.curr_injected_bitrate = clamp_int32(sample.bitrate_bps);
      air_video.dummy2 = clamp_int32(sample.bitrate_bps);
      air_video.curr_injected_pps = sample.packets_per_second;
      air_video.curr_dropped_frames = 0;
      air_video.curr_fec_percentage = 0;
      stats.stats_wb_video_air.push_back(air_video);
    }
  }
  openhd::LinkActionHandler::instance().update_link_stats(stats);
}
