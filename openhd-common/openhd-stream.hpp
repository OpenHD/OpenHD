#ifndef OPENHD_STREAM_H
#define OPENHD_STREAM_H


#include <string>
#include <vector>

#include "openhd-util.hpp"


typedef enum StreamType {
    StreamTypeWBC,
    StreamTypeIP,
    StreamTypeSDR,
    StreamTypeUnknown
} StreamType;


typedef enum DataType {
    DataTypeVideo,
    DataTypeAudio,
    DataTypeTelemetry,
    DataTypeUnknown
} DataType;


struct Stream {
    StreamType stream_type;
    DataType data_type;
    /*
     * Used internally to differentiate multiple streams of the same type.
     *
     * For example if there are 3 cameras (which have indexes in the current manifest-oriented system), 
     * the stream indexes should match the camera indexes so the system can keep track of which is which, without
     * depending on the details of how the stream transmission is implemented.
     *
     * For telemetry, the microservice channel is always index 0 and uses Mavlink, while the flight controller
     * telemetry should be index 1 and could be carrying anything.
     */
    uint16_t index;

    // used for WBC and possibly IP (depends on radio type)
    std::string tx_keypair;
    std::string rx_keypair;

    int total_blocks = 12;
    int data_blocks = 8;
    int mcs = 3;
    bool stbc = false;
    bool ldpc = false;
    int bandwidth = 20;
    bool short_gi = false;

    // used for WBC
    uint16_t rf_tx_port;
    uint16_t rf_rx_port; 

    // the UDP port that received packets will be sent to
    uint16_t local_tx_port;

    // the UDP port that will accept packets for transmission
    uint16_t local_rx_port;
};


inline std::string stream_type_to_string(StreamType stream_type) {
    switch (stream_type) {
        case StreamTypeWBC: {
            return "wbc";
        }
        case StreamTypeIP: {
            return "ip";
        }
        case StreamTypeSDR: {
            return "sdr";
        }
        default: {
            return "unknown";
        }
    }
}


inline std::string data_type_to_string(DataType data_type) {
    switch (data_type) {
        case DataTypeVideo: {
            return "video";
        }
        case DataTypeAudio: {
            return "audio";
        }
        case DataTypeTelemetry: {
            return "telemetry";
        }
        default: {
            return "unknown";
        }
    }
}

#endif
