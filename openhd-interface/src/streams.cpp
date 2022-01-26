#include <cstdio>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>

#include <boost/regex.hpp>

#include "json.hpp"

#include "openhd-camera.hpp"
#include "openhd-ethernet.hpp"
#include "openhd-platform.hpp"
#include "openhd-status.hpp"
#include "openhd-stream.hpp"
#include "openhd-wifi.hpp"

#include "streams.h"

/*
 * This file is likely temporary, there are some unresolved questions over whether stream settings should be global
 * and therefore part of the system.conf settings file, or per-camera, in which case it might make more sense for
 * the video service to be starting the video streams.
 *
 * That's why the telemetry and video stream functions, which are somewhat duplicated, are separate. They may not end up
 * in the same place.
 *
 */
Streams::Streams(boost::asio::io_service &io_service, bool is_air, std::string unit_id): m_io_service(io_service), m_is_air(is_air), m_unit_id(unit_id) {}


void Streams::configure() {
    std::cout << "Streams::configure()" << std::endl;

    configure_microservice();

    /*
     * These are static for the moment for testing purposes, they may move to another process and be split up, and will
     * likely need to run *after* the connected drone notifies the ground what the settings are, otherwise we would have
     * settings that must be manually updated on the ground to match different vehicles, which we're trying to avoid
     */
    configure_telemetry();
    configure_video();
}


void Streams::configure_microservice() {
    std::cout << "Streams::configure_microservice()" << std::endl;

    try {
        Stream stream;
        stream.index = 0;
        stream.stream_type = StreamTypeWBC;
        stream.data_type = DataTypeTelemetry;
        stream.rf_tx_port = 1;
        stream.rf_rx_port = 2;
        // these match the openhd-telemetry@microservice systemd unit and should never change
        stream.local_tx_port = 15550;
        stream.local_rx_port = 15551;
        stream.tx_keypair = "/tmp/tx.key";
        stream.rx_keypair = "/tmp/rx.key";
        stream.bandwidth = 20;
        stream.short_gi = false;
        stream.ldpc = false;
        stream.stbc = false;
        stream.mcs = 1;
        stream.data_blocks = 1;
        stream.total_blocks = 2;
        m_microservice_processes = start_telemetry_stream(stream);
    } catch (std::exception &ex) {
        std::cerr << "Failed to start microservice processes: " << ex.what() << std::endl;
    }
}


void Streams::configure_telemetry() {
    std::cout << "Streams::configure_telemetry()" << std::endl;

    try {
        /*
        * todo: these will be configurable by the new settings system, but for now they don't need to be, and non-mavlink
        *        telemetry isn't merged in the router yet anyway
        */
        Stream stream;
        stream.index = 1;
        stream.stream_type = StreamTypeWBC;
        stream.data_type = DataTypeTelemetry;
        stream.rf_tx_port = 3;
        stream.rf_rx_port = 4;
        // these match the openhd-telemetry@telemetry systemd unit and should never change
        stream.local_tx_port = 16550;
        stream.local_rx_port = 16551;
        stream.tx_keypair = "/tmp/tx.key";
        stream.rx_keypair = "/tmp/rx.key";
        stream.bandwidth = 20;
        stream.short_gi = false;
        stream.ldpc = false;
        stream.stbc = false;
        stream.mcs = 1;
        stream.data_blocks = 1;
        stream.total_blocks = 2;
        m_telemetry_processes = start_telemetry_stream(stream);
    } catch (std::exception &ex) {
        std::cerr << "Failed to start telemetry processes: " << ex.what() << std::endl;
    }
}


void Streams::configure_video() {
    std::cout << "Streams::configure_video()" << std::endl;

    try {
        /*
        * todo: this will need to change when the settings are wired in, and when streams are started based on the
        *       detected cameras on the air side. It will not just be set with 4 static streams like this with the
        *       same params for everything, it's for testing purposes.
        */
        Stream stream;
        stream.index = 0;
        stream.stream_type = StreamTypeWBC;
        stream.data_type = DataTypeVideo;
        stream.rf_tx_port = 56;
        stream.rf_rx_port = 56;
        stream.local_tx_port = 5620;
        stream.local_rx_port = 5620;
        stream.tx_keypair = "/tmp/tx.key";
        stream.rx_keypair = "/tmp/rx.key";
        stream.bandwidth = m_bandwidth;
        stream.short_gi = m_short_gi;
        stream.ldpc = m_ldpc;
        stream.stbc = m_stbc;
        stream.mcs = m_mcs;
        stream.data_blocks = m_data_blocks;
        stream.total_blocks = m_total_blocks;

        auto video1 = start_video_stream(stream);
        m_video_processes.push_back(std::move(video1));



        // reusing the above stream object because most of it is the same and this is temporary
        stream.index = 1;
        stream.rf_tx_port = 57;
        stream.rf_rx_port = 57;
        stream.local_tx_port = 5621;
        stream.local_rx_port = 5621;
        auto video2 = start_video_stream(stream);
        m_video_processes.push_back(std::move(video2));


        stream.index = 2;
        stream.rf_tx_port = 58;
        stream.rf_rx_port = 58;
        stream.local_tx_port = 5622;
        stream.local_rx_port = 5622;
        auto video3 = start_video_stream(stream);
        m_video_processes.push_back(std::move(video3));


        stream.index = 3;
        stream.rf_tx_port = 59;
        stream.rf_rx_port = 59;
        stream.local_tx_port = 5623;
        stream.local_rx_port = 5623;
        auto video4 = start_video_stream(stream);
        m_video_processes.push_back(std::move(video4));
    } catch (std::exception &ex) {
        std::cerr << "Failed to start video processes: " << ex.what() << std::endl;
    }
}


void Streams::set_broadcast_cards(std::vector<WiFiCard> cards) {
    m_broadcast_cards = cards;
}


std::vector<std::string> Streams::broadcast_card_names() {
    std::vector<std::string> names;

    for (auto card : m_broadcast_cards) {
        names.push_back(card.name);
    }

    return names;
}



boost::process::child Streams::start_video_stream(Stream stream) {
    std::cout << "Streams::start_video_stream()" << std::endl;

    m_streams.push_back(stream);

    auto broadcast_interfaces = broadcast_card_names();
    if (broadcast_interfaces.size() == 0) {
        status_message(STATUS_LEVEL_EMERGENCY, "No wifibroadcast interfaces available");
        throw std::runtime_error("no wifibroadcast interfaces available");
    }



    if (m_is_air) {    
        std::vector<std::string> tx_args { 
            "-r", std::to_string(stream.rf_tx_port),
            "-u", std::to_string(stream.local_tx_port), 
            "-K", stream.tx_keypair, 
            "-B", std::to_string(m_bandwidth), 
            "-G", stream.short_gi ? "short" : "long", 
            "-S", stream.stbc ? "1" : "0", 
            "-L", stream.ldpc ? "1" : "0", 
            "-M", std::to_string(stream.mcs), 
            //"-k", std::to_string(stream.data_blocks), 
            //"-n", std::to_string(stream.total_blocks),
        };

        tx_args.insert(tx_args.end(), broadcast_interfaces.begin(), broadcast_interfaces.end());



        boost::process::child c_tx(boost::process::search_path("wfb_tx"), tx_args, m_io_service);

        c_tx.detach();

        return std::move(c_tx);
    } else {
        std::vector<std::string> rx_args { 
            "-r", std::to_string(stream.rf_rx_port), 
            "-u", std::to_string(stream.local_rx_port), 
            "-K", stream.rx_keypair, 
            //"-k", std::to_string(stream.data_blocks), 
            //"-n", std::to_string(stream.total_blocks),
        };

        rx_args.insert(rx_args.end(), broadcast_interfaces.begin(), broadcast_interfaces.end());


        boost::process::child c_rx(boost::process::search_path("wfb_rx"), rx_args, m_io_service);

        c_rx.detach();

        return std::move(c_rx);
    }
}


stream_pair Streams::start_telemetry_stream(Stream stream) {
    std::cout << "Streams::start_telemetry_stream()" << std::endl;

    m_streams.push_back(stream);

    auto broadcast_interfaces = broadcast_card_names();
    
    if (broadcast_interfaces.size() == 0) {
        status_message(STATUS_LEVEL_EMERGENCY, "No wifibroadcast interfaces available");
        throw std::runtime_error("no wifibroadcast interfaces available");
    }



    std::vector<std::string> rx_args { 
        "-r", std::to_string(m_is_air ? stream.rf_rx_port : stream.rf_tx_port), 
        "-u", std::to_string(stream.local_rx_port), 
        "-K", stream.rx_keypair,
       // "-k", std::to_string(stream.data_blocks), 
       // "-n", std::to_string(stream.total_blocks),
    };
    rx_args.insert(rx_args.end(), broadcast_interfaces.begin(), broadcast_interfaces.end());


    std::vector<std::string> tx_args { 
        "-r", std::to_string(m_is_air ? stream.rf_tx_port : stream.rf_rx_port),
        "-u", std::to_string(stream.local_tx_port), 
        "-K", stream.tx_keypair,
        "-B", std::to_string(stream.bandwidth), 
        "-G", stream.short_gi ? "short" : "long", 
        "-S", stream.stbc ? "1" : "0", 
        "-L", stream.ldpc ? "1" : "0", 
        "-M", std::to_string(stream.mcs), 
        //"-k", std::to_string(stream.data_blocks), 
        //"-n", std::to_string(stream.total_blocks),
    };
    tx_args.insert(tx_args.end(), broadcast_interfaces.begin(), broadcast_interfaces.end());


    boost::process::child c_tx(boost::process::search_path("wfb_tx"), tx_args, m_io_service);

    c_tx.detach();

    boost::process::child c_rx(boost::process::search_path("wfb_rx"), rx_args, m_io_service);

    c_rx.detach();

    return std::make_pair(std::move(c_tx), std::move(c_rx));
}
