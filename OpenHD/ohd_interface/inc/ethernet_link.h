#ifndef OPENHD_ETHERNET_LINK_H
#define OPENHD_ETHERNET_LINK_H

#include "openhd_link.hpp"
#include "openhd_config.h"
#include "openhd_util.h"
#include "openhd_udp.h"
#include <memory>
#include <thread>

class EthernetLink : public OHDLink {
public:
    explicit EthernetLink(OHDProfile profile);
    ~EthernetLink();

    // OHDLink implementations
    void transmit_telemetry_data(TelemetryTxPacket packet) override;
    void transmit_video_data(int stream_index, const openhd::FragmentedVideoFrame& fragmented_video_frame) override;
    void transmit_audio_data(const openhd::AudioPacket& audio_packet) override;

private:
    OHDProfile m_profile;

    // Configuration variables (private)
    std::string GROUND_UNIT_IP = "192.168.2.1"; // Default IP
    std::string AIR_UNIT_IP = "192.168.2.18";    // Default IP
    int VIDEO_PORT = 5910;                      // Default video port
    int TELEMETRY_PORT = 5920;                  // Default telemetry port

    std::unique_ptr<openhd::UDPForwarder> m_video_tx;   // Video transmitter
    std::unique_ptr<openhd::UDPReceiver> m_video_rx;   // Video receiver
    std::unique_ptr<openhd::UDPForwarder> m_telemetry_tx; // Telemetry transmitter
    std::unique_ptr<openhd::UDPReceiver> m_telemetry_rx; // Telemetry receiver

    void initialize_air_unit();
    void initialize_ground_unit();

    void handle_video_data(int stream_index, const uint8_t* data, int data_len);
    void handle_telemetry_data(const uint8_t* data, int data_len);
};

#endif // OPENHD_ETHERNET_LINK_H
