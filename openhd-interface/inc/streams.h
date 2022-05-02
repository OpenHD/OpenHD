#ifndef STREAMS_H
#define STREAMS_H

#include <array>
#include <chrono>
#include <vector>
#include <utility>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "json.hpp"

#include "openhd-stream.hpp"
#include "openhd-wifi.hpp"

#include <boost/process.hpp>


typedef std::pair<boost::process::child, boost::process::child> stream_pair;

class Streams {
public:
    Streams(boost::asio::io_service &io_service, bool is_air, std::string unit_id);
    
    virtual ~Streams() {}
    
    void configure();
    
    void configure_video();
    void configure_microservice();
    void configure_telemetry();
    void configure_microservices();

    std::vector<std::string> broadcast_card_names();

    void set_broadcast_cards(std::vector<WiFiCard> cards);

    stream_pair start_telemetry_stream(Stream stream);

    boost::process::child start_video_stream(Stream stream);

private:
    boost::asio::io_service &m_io_service;

    const std::string m_unit_id;

    const bool m_is_air = false;

    int m_bandwidth = 20;
    bool m_short_gi = false;

    bool m_ldpc = false;
    bool m_stbc = false;
    int m_mcs = 3;

    int m_data_blocks = 8;
    int m_total_blocks = 12;

    std::vector<WiFiCard> m_broadcast_cards;

    std::vector<Stream> m_streams;

    std::vector<boost::process::child> m_video_processes;

    stream_pair m_microservice_processes;

    stream_pair m_telemetry_processes;
};


#endif
