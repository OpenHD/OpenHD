//
// Created by consti10 on 05.04.24.
//

#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include "microhard_link.h"

// Helper function to retrieve IP addresses starting with a specific prefix
std::vector<std::string> get_ip_addresses(const std::string& prefix) {
    std::vector<std::string> ip_addresses;
    int sockfd;
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[4096];
    struct ifreq* it;
    struct ifreq* end;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return ip_addresses;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
        perror("ioctl");
        close(sockfd);
        return ip_addresses;
    }

    it = ifc.ifc_req;
    end = (struct ifreq*)(buf + ifc.ifc_len);

    for (; it != end; ++it) {
        strncpy(ifr.ifr_name, it->ifr_name, IFNAMSIZ - 1);
        if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
            perror("ioctl");
            continue;
        }
        struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
        std::string ip = inet_ntoa(addr->sin_addr);
        if (ip.find(prefix) == 0) {
            ip_addresses.push_back(ip);
        }
    }

    close(sockfd);
    return ip_addresses;
}

void log_ip_addresses() {
    auto ip_addresses = get_ip_addresses("192.168.168");
    if (!ip_addresses.empty()) {
        for (const auto& ip : ip_addresses) {
            m_console->info("Found IP address: {}", ip);
        }
    } else {
        m_console->info("No IP addresses starting with 192.168.168 found.");
    }
}

// Master
static constexpr auto MICROHARD_AIR_IP = "192.168.168.11";
// Client
static constexpr auto MICROHARD_GND_IP = "192.168.168.12";
// The assigned IPs
// NOTE: They have to be set correctly!
static constexpr auto DEVICE_IP_GND = "192.168.168.122";
static constexpr auto DEVICE_IP_AIR = "192.168.168.153";

// We send data over those port(s)
static constexpr int MICROHARD_UDP_PORT_VIDEO_AIR_TX = 5910;
static constexpr int MICROHARD_UDP_PORT_TELEMETRY_AIR_TX = 5920;

static bool is_microhard_device_attached() {
    const std::string command = "lsusb";
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        openhd::log::get_default()->debug("Failed to run lsusb command");
        return false;
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
        if (result.find("Microhard") != std::string::npos) {
            openhd::log::get_default()->debug("Microhard device found in lsusb output");
            return true;
        }
    }

    openhd::log::get_default()->debug("Microhard device not found in lsusb output");
    return false;
}

static bool check_ip_alive(const std::string &ip, int port = 80) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;
    
    struct sockaddr_in addr = {AF_INET, htons(port), inet_addr(ip.c_str())};
    bool connected = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0;
    close(sockfd);
    return connected;
}

static void wait_for_microhard_module(bool air) {
    while (true) {
        const auto microhard_device_ip = air ? MICROHARD_AIR_IP : MICROHARD_GND_IP;
        auto available = check_ip_alive(microhard_device_ip);
        if (available) {
            openhd::log::get_default()->debug("Microhard module found");
            break;
        }
        openhd::log::get_default()->debug("Waiting for microhard module");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

MicrohardLink::MicrohardLink(OHDProfile profile) : m_profile(profile) {
    // Wait for the module to become available
    wait_for_microhard_module(m_profile.is_air);

    if (m_profile.is_air) {
        // We send video
        m_video_tx = std::make_unique<openhd::UDPForwarder>(DEVICE_IP_GND, MICROHARD_UDP_PORT_VIDEO_AIR_TX);
        // We send and receive telemetry
        auto cb_telemetry_rx = [this](const uint8_t *data, const std::size_t data_len) {
            auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
            on_receive_telemetry_data(shared);
        };
        m_telemetry_tx_rx = std::make_unique<openhd::UDPReceiver>(DEVICE_IP_AIR, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, cb_telemetry_rx);
        m_telemetry_tx_rx->runInBackground();
    } else {
        auto cb_video_rx = [this](const uint8_t *payload, const std::size_t payloadSize) {
            on_receive_video_data(0, payload, payloadSize);
        };
        m_video_rx = std::make_unique<openhd::UDPReceiver>(DEVICE_IP_GND, MICROHARD_UDP_PORT_VIDEO_AIR_TX, cb_video_rx);
        m_video_rx->runInBackground();
        auto cb_telemetry_rx = [this](const uint8_t *data, const std::size_t data_len) {
            auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
            on_receive_telemetry_data(shared);
        };
        m_telemetry_tx_rx = std::make_unique<openhd::UDPReceiver>(DEVICE_IP_GND, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, cb_telemetry_rx);
        m_telemetry_tx_rx->runInBackground();
    }
}

void MicrohardLink::transmit_telemetry_data(OHDLink::TelemetryTxPacket packet) {
    const auto destination_ip = m_profile.is_air ? DEVICE_IP_GND : DEVICE_IP_AIR;
    m_telemetry_tx_rx->forwardPacketViaUDP(
        destination_ip, MICROHARD_UDP_PORT_TELEMETRY_AIR_TX, packet.data->data(), packet.data->size());
}

void MicrohardLink::transmit_video_data(int stream_index, const openhd::FragmentedVideoFrame &fragmented_video_frame) {
    assert(m_profile.is_air);
    if (stream_index == 0) {
        for (auto &fragment : fragmented_video_frame.rtp_fragments) {
            m_video_tx->forwardPacketViaUDP(fragment->data(), fragment->size());
        }
    }
}

void MicrohardLink::transmit_audio_data(const openhd::AudioPacket &audio_packet) {
    // not implemented
}

std::vector<openhd::Setting> MicrohardLink::get_all_settings() {
    using namespace openhd;
    std::vector<openhd::Setting> ret{};
    auto change_dummy = openhd::IntSetting{0, [this](std::string, int value) { return true; }};
    ret.push_back(Setting{"MICROHARD_DUMMY0", change_dummy});
    return ret;
}
