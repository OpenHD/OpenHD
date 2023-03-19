//
// Created by consti10 on 27.04.22.
//

#ifndef OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H
#define OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H

// This file should only contain const values that are needed by more than one module.

namespace openhd{
// The radio_ports for telemetry in wifibroadcast. These variables are from the perspective of the air pi
// aka for the ground pi, tx and rx radio ports are the opposite.
static constexpr auto TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT = 3;
static constexpr auto TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT = 4;
static_assert(TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT != TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT,
			  "Must be different");

// Where any service can send log messages to, both on air and ground pi.
// Picked up by telemetry service.
static constexpr auto LOCAL_LOG_MESSAGES_UDP_PORT = 50001;

// Video is unidirectional from air to ground
static constexpr auto VIDEO_PRIMARY_RADIO_PORT = 10;
static constexpr auto VIDEO_SECONDARY_RADIO_PORT = 11;
static_assert(VIDEO_PRIMARY_RADIO_PORT != VIDEO_SECONDARY_RADIO_PORT, "Must be different");


// Where the video stream transmitted via wifibroadcast is made available to QOpenHD to be picked up.
static constexpr auto VIDEO_GROUND_VIDEO_STREAM_1_UDP = 5600;
static constexpr auto VIDEO_GROUND_VIDEO_STREAM_2_UDP = 5601;
static_assert(VIDEO_GROUND_VIDEO_STREAM_1_UDP != VIDEO_GROUND_VIDEO_STREAM_2_UDP, "Must be different");

static constexpr auto VERSION_NUMBER_STRING="2.3.3-beta-dirty";

}
#endif //OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H
