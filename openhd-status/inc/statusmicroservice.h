#ifndef STATUS_H
#define STATUS_H

#include <openhd/mavlink.h>

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include "openhd-microservice.hpp"
#include "openhd-platform.hpp"

struct StatusMessage {
    std::string message;
    int sysid;
    int compid;
    MAV_SEVERITY severity;
    uint64_t timestamp;
};

#define MAX_RX_INTERFACES 8;

class StatusMicroservice: public Microservice {
public:

    typedef struct{
            int32_t count_all;
            int32_t rssi_sum;
            int8_t rssi_min;
            int8_t rssi_max;
        }RSSIForWifiCard_t;

    typedef struct {
        // all these values are absolute (like done previously in OpenHD)
            uint8_t radioport=0;
            // all received packets
            uint64_t count_p_all=0;
            // n packets that were received but could not be used (after already filtering for the right port)
            uint64_t count_p_bad=0;
            // n packets that could not be decrypted
            uint64_t count_p_dec_err=0;
            // n packets that were successfully decrypted
            uint64_t count_p_dec_ok=0;
            // n packets that were corrected by FEC
            uint64_t count_p_fec_recovered=0;
            // n packets that were completely lost though FEC
            uint64_t count_p_lost=0;
            // min max and avg rssi for each wifi card since the last call.
            // if count_all for a card at position N is 0 nothing has been received on this card from the last call (or the card at position N is not used for this instance)
            std::array<RSSIForWifiCard_t, 8> rssiPerCard{};    
    }localWifiStatusmessage_t;

    StatusMicroservice(boost::asio::io_service &io_service, PlatformType platform, bool is_air, std::string unit_id);

    void setup();
    void Send_air_load();
    void Send_air_telemetrystatus(localWifiStatusmessage_t& msg,
                                size_t bytes_transferred);

    void start_udp_read();

    void handle_udp_read(const boost::system::error_code& error,
                         size_t bytes_transferred);

    void send_status_message(MAV_SEVERITY severity, std::string message);

    virtual void process_mavlink_message(mavlink_message_t msg);

 private:
    std::string m_unit_id;

    bool m_is_air = false;

    enum { max_length = 1024 };
    char data[max_length];

    std::vector<StatusMessage> m_status_messages;

    boost::asio::ip::udp::socket m_udp_socket;

    std::string m_openhd_version = "unknown";
};

#endif
