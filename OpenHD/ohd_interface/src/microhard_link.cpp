#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "microhard_link.h"
#include "openhd_temporary_air_or_ground.h"

// Macro to log function entry
#define LOG_FUNCTION_ENTRY() openhd::log::get_default()->warn("Entering function: {}", __FUNCTION__)

// Constants
static constexpr auto MICROHARD_AIR_IP = "192.168.168.11";
static constexpr auto MICROHARD_GND_IP = "192.168.168.12";
static constexpr auto DEVICE_IP_AIR = "192.168.168.153";
static constexpr int MICROHARD_UDP_PORT_VIDEO_AIR_TX = 5910;
static constexpr int MICROHARD_UDP_PORT_TELEMETRY_AIR_TX = 5920;

// Utility Functions
namespace utils {

std::vector<std::string> get_ip_addresses(const std::string& prefix) {
    LOG_FUNCTION_ENTRY();
    std::vector<std::string> ip_addresses;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sockfd < 0) {
        perror("socket");
        return ip_addresses;
    }

    char buf[4096];
    struct ifconf ifc = {sizeof(buf), buf};
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
        perror("ioctl");
        close(sockfd);
        return ip_addresses;
    }

    auto* it = reinterpret_cast<struct ifreq*>(buf);
    auto* end = reinterpret_cast<struct ifreq*>(buf + ifc.ifc_len);

    for (; it != end; ++it) {
        struct ifreq ifr;
        strncpy(ifr.ifr_name, it->ifr_name, IFNAMSIZ - 1);
        if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
            perror("ioctl");
            continue;
        }
        auto* addr = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
        std::string ip = inet_ntoa(addr->sin_addr);
        if (ip.find(prefix) == 0) {
            ip_addresses.push_back(ip);
        }
    }

    close(sockfd);
    return ip_addresses;
}

std::string execute_command(const std::string& cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to run command.");
    }

    char buffer[128];
    std::string result;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result = buffer;
    }
    pclose(pipe);

    // Trim trailing newline
    if (!result.empty()) {
        result.erase(result.find_last_not_of("\n\r") + 1);
    }

    return result;
}

std::string get_gateway_ip() {
    openhd::log::get_default()->warn("Getting gateway IP...");
    std::string cmd = "ip route show default | awk '/default/ {print $3}' | grep '^192\\.168\\.168'";
    std::string result = execute_command(cmd);
    openhd::log::get_default()->warn("Filtered Gateway IP: {}", result);
    return result;
}

bool check_ip_alive(const std::string& ip, int port = 23) {
    LOG_FUNCTION_ENTRY();
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        openhd::log::get_default()->warn("Socket creation failed for IP {}", ip);
        return false;
    }

    struct sockaddr_in addr = {AF_INET, htons(port), inet_addr(ip.c_str())};
    bool connected = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0;
    close(sockfd);

    if (connected) {
        openhd::log::get_default()->warn("IP {} is alive", ip);
    } else {
        openhd::log::get_default()->warn("IP {} is not alive", ip);
    }

    return connected;
}

} // namespace utils

// MicrohardLink Class
MicrohardLink::MicrohardLink(OHDProfile profile) : m_profile(profile) {
    LOG_FUNCTION_ENTRY();
    wait_for_microhard_module(m_profile.is_air);

    if (m_profile.is_air) {
        m_video_tx = std::make_unique<openhd::UDPForwarder>(DEVICE_IP_GND, MICROHARD_UDP_PORT_VIDEO_AIR_TX);
        auto cb_telemetry_rx = [this](const uint8_t* data, std::size_t data_len) {
            auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
            on_receive_telemetry_data(shared);
        };
        m_telemetry_tx_rx = std::make_unique<openhd::UDPReceiver>(DEVICE_IP_AIR, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, cb_telemetry_rx);
        m_telemetry_tx_rx->runInBackground();
    } else {
        auto cb_video_rx = [this](const uint8_t* payload, std::size_t payloadSize) {
            on_receive_video_data(0, payload, payloadSize);
        };
        m_video_rx = std::make_unique<openhd::UDPReceiver>(DEVICE_IP_GND, MICROHARD_UDP_PORT_VIDEO_AIR_TX, cb_video_rx);
        m_video_rx->runInBackground();

        auto cb_telemetry_rx = [this](const uint8_t* data, std::size_t data_len) {
            auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
            on_receive_telemetry_data(shared);
        };
        m_telemetry_tx_rx = std::make_unique<openhd::UDPReceiver>(DEVICE_IP_GND, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, cb_telemetry_rx);
        m_telemetry_tx_rx->runInBackground();
    }
}

void MicrohardLink::transmit_telemetry_data(OHDLink::TelemetryTxPacket packet) {
    LOG_FUNCTION_ENTRY();
    const auto& destination_ip = m_profile.is_air ? DEVICE_IP_GND : DEVICE_IP_AIR;
    m_telemetry_tx_rx->forwardPacketViaUDP(
        destination_ip, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, packet.data->data(), packet.data->size());
}

void MicrohardLink::transmit_video_data(int stream_index, const openhd::FragmentedVideoFrame& fragmented_video_frame) {
    LOG_FUNCTION_ENTRY();
    assert(m_profile.is_air);
    if (stream_index == 0) {
        for (const auto& fragment : fragmented_video_frame.rtp_fragments) {
            m_video_tx->forwardPacketViaUDP(fragment->data(), fragment->size());
        }
    }
}

void MicrohardLink::transmit_audio_data(const openhd::AudioPacket& audio_packet) {
    LOG_FUNCTION_ENTRY();
    // not implemented
}

std::vector<openhd::Setting> MicrohardLink::get_all_settings() {
    LOG_FUNCTION_ENTRY();
    std::vector<openhd::Setting> settings;
    auto dummy_setting = openhd::IntSetting{0, [](std::string, int) { return true; }};
    settings.push_back(openhd::Setting{"MICROHARD_DUMMY0", dummy_setting});
    return settings;
}

// Private Methods
void MicrohardLink::wait_for_microhard_module(bool air) {
    LOG_FUNCTION_ENTRY();
    const std::string microhard_device_ip = get_detected_ip_address();
    if (microhard_device_ip.empty()) {
        openhd::log::get_default()->warn("No microhard device IP address detected. Exiting.");
        return;
    }

    while (true) {
        if (utils::check_ip_alive(microhard_device_ip)) {
            openhd::log::get_default()->debug("Microhard module found at {}", microhard_device_ip);
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::string MicrohardLink::get_detected_ip_address() {
    LOG_FUNCTION_ENTRY();
    auto ip_addresses = utils::get_ip_addresses("192.168.168");
    if (!ip_addresses.empty()) {
        return ip_addresses.front();
    }
    openhd::log::get_default()->warn("No IP addresses starting with 192.168.168 found.");
    return "";
}

std::string MicrohardLink::find_device_ip_gnd() {
    LOG_FUNCTION_ENTRY();
    auto ip_addresses = utils::get_ip_addresses("192.168.168");
    for (const auto& ip : ip_addresses) {
        if (ip != MICROHARD_AIR_IP && ip != MICROHARD_GND_IP) {
            return ip;
        }
    }
    openhd::log::get_default()->warn("No suitable IP address found for DEVICE_IP_GND. Using default.");
    return "192.168.168.122";
}
