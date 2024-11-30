#include "ethernet_link.h"
#include "openhd_util_filesystem.h"
#include "config_paths.h"
#include "openhd_util.h"
#include "../lib/ini/ini.hpp" // INI parser
#include "openhd_spdlog.h"   // Logging library

#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

// Configuration structure for Ethernet
struct EthernetConfig {
    std::string ground_unit_ip = "192.168.1.1";  // Default values
    std::string air_unit_ip = "192.168.1.2";
    int video_port = 5000;
    int telemetry_port = 6000;
};

// Logging setup
static std::shared_ptr<spdlog::logger> get_logger() {
    return openhd::log::create_or_get("ethernet_link");
}

// Static configuration file path
static std::string ETHERNET_FILE_PATH =
    std::string(getConfigBasePath()) + "ethernet.txt";

// Function to load Ethernet configuration
EthernetConfig load_ethernet_config(const std::string& filepath) {
    EthernetConfig config;

    if (!OHDFilesystemUtil::exists(filepath)) {
        get_logger()->warn("Configuration file [{}] not found. Using defaults.", filepath);
        return config;
    }

    try {
        get_logger()->info("Loading configuration from [{}]", filepath);
        inih::INIReader reader(filepath);
        if (reader.ParseError() < 0) {
            throw std::runtime_error("Failed to parse configuration file");
        }

        // Parse configuration values
        config.ground_unit_ip = reader.Get<std::string>("ethernet", "ground_unit_ip", config.ground_unit_ip);
        config.air_unit_ip = reader.Get<std::string>("ethernet", "air_unit_ip", config.air_unit_ip);
        config.video_port = reader.Get<int>("ethernet", "video_port", config.video_port);
        config.telemetry_port = reader.Get<int>("ethernet", "telemetry_port", config.telemetry_port);
    } catch (const std::exception& ex) {
        get_logger()->error("Error reading configuration file [{}]: {}", filepath, ex.what());
        throw;
    }

    return config;
}

EthernetLink::EthernetLink(OHDProfile profile) : m_profile(profile) {
    // Load the Ethernet configuration
    EthernetConfig config;
    try {
        config = load_ethernet_config(ETHERNET_FILE_PATH);
    } catch (const std::exception& ex) {
        get_logger()->error("Falling back to defaults due to error: {}", ex.what());
    }

    // Assign configuration values
    GROUND_UNIT_IP = config.ground_unit_ip;
    AIR_UNIT_IP = config.air_unit_ip;
    VIDEO_PORT = config.video_port;
    TELEMETRY_PORT = config.telemetry_port;

    // Initialize either air or ground unit based on the profile
    if (m_profile.is_air) {
        initialize_air_unit();
    } else {
        initialize_ground_unit();
    }
}

EthernetLink::~EthernetLink() {
    // Stop background receivers
    if (m_video_rx) m_video_rx->stopBackground();
    if (m_telemetry_rx) m_telemetry_rx->stopBackground();
}

void EthernetLink::initialize_air_unit() {
    // Initialize video transmitter for sending video to the ground unit
    m_video_tx = std::make_unique<openhd::UDPForwarder>(GROUND_UNIT_IP, VIDEO_PORT);

    // Initialize telemetry transmitter and receiver for bidirectional telemetry
    m_telemetry_tx = std::make_unique<openhd::UDPForwarder>(GROUND_UNIT_IP, TELEMETRY_PORT);
    m_telemetry_rx = std::make_unique<openhd::UDPReceiver>("0.0.0.0", TELEMETRY_PORT,
        [this](const uint8_t* data, std::size_t len) {
            handle_telemetry_data(data, len); // Process incoming telemetry
        });

    // Start telemetry receiver in the background
    if (m_telemetry_rx) m_telemetry_rx->runInBackground();
}

void EthernetLink::initialize_ground_unit() {
    // Initialize video receiver for receiving video from the air unit
    m_video_rx = std::make_unique<openhd::UDPReceiver>("0.0.0.0", VIDEO_PORT,
        [this](const uint8_t* data, std::size_t len) {
            handle_video_data(0, data, len); // Process incoming video
        });

    // Initialize telemetry transmitter and receiver for bidirectional telemetry
    m_telemetry_tx = std::make_unique<openhd::UDPForwarder>(AIR_UNIT_IP, TELEMETRY_PORT);
    m_telemetry_rx = std::make_unique<openhd::UDPReceiver>("0.0.0.0", TELEMETRY_PORT,
        [this](const uint8_t* data, std::size_t len) {
            handle_telemetry_data(data, len); // Process incoming telemetry
        });

    // Start video and telemetry receivers in the background
    if (m_video_rx) m_video_rx->runInBackground();
    if (m_telemetry_rx) m_telemetry_rx->runInBackground();
}

void EthernetLink::transmit_telemetry_data(TelemetryTxPacket packet) {
    // Send telemetry data to the destination
    if (m_telemetry_tx) {
        m_telemetry_tx->forwardPacketViaUDP(packet.data->data(), packet.data->size());
    }
}

void EthernetLink::transmit_video_data(int stream_index, const openhd::FragmentedVideoFrame& fragmented_video_frame) {
    // Send video data fragments to the destination
    if (m_video_tx) {
        for (const auto& fragment : fragmented_video_frame.rtp_fragments) {
            m_video_tx->forwardPacketViaUDP(fragment->data(), fragment->size());
        }
    }
}

void EthernetLink::transmit_audio_data(const openhd::AudioPacket& audio_packet) {
    // Currently not implemented for EthernetLink
}

void EthernetLink::handle_video_data(int stream_index, const uint8_t* data, int data_len) {
    // Forward incoming video data to the upper layer
    on_receive_video_data(stream_index, data, data_len);
}

void EthernetLink::handle_telemetry_data(const uint8_t* data, int data_len) {
    // Forward incoming telemetry data to the upper layer
    auto shared = std::make_shared<std::vector<uint8_t>>(data, data + data_len);
    on_receive_telemetry_data(shared);
}
