//
// Created by consti10 on 27.04.22.
//

#ifndef OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H
#define OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H

// This file should only contain const values that are needed by more than one module.
// For example, the telemetry module expects a valid wifibroadcast udp link on a specific localhost port-
// Therefore, we declare the port value here such that both modules can use it without duplicating it,
// which would lead to error-prone code.

// The ground station wifibroadcast instance(s) send data to the ground rx port
// and listen for data on the ground tx port.
// Note that the ports for the air unit are just reversed - this way, one can either run
// the telemetry service on the same system in 2 instances and they talk to each other,
// or run it on 2 different systems with wifibroadcast in between.
static constexpr auto OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX = 16550;
static constexpr auto OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX = 16551;
static_assert(
	OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX != OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX,
	"Must be different");
// The radio_port in wifibroadcast. These variables are from the perspective of the air pi
// aka for the ground pi, tx and rx radio ports are the opposite.
static constexpr auto OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT = 3;
static constexpr auto OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT = 4;
static_assert(OHD_TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT != OHD_TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT,
			  "Must be different");

// Where any service can send log messages to, both on air and ground pi.
// Picked up by telemetry service.
static constexpr auto OHD_LOCAL_LOG_MESSAGES_UDP_PORT = 50001;

static constexpr auto OHD_VIDEO_PRIMARY_RADIO_PORT = 10;
static constexpr auto OHD_VIDEO_SECONDARY_RADIO_PORT = 11;
static_assert(OHD_VIDEO_PRIMARY_RADIO_PORT != OHD_VIDEO_SECONDARY_RADIO_PORT, "Must be different");
// Where on the air pi the video stream rtp data (h264 or h265) is sent to via UDP (localhost)
static constexpr auto OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP = 5600; // first (primary) stream
static constexpr auto OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP = 5601; // secondary stream
static_assert(OHD_VIDEO_AIR_VIDEO_STREAM_1_UDP != OHD_VIDEO_AIR_VIDEO_STREAM_2_UDP, "Must be different");
// TODO: Do we really need more than 2 video stream(s) ?!! I don't think so, there is no bandwidth for more anyways.
// Also, In case someone has more than 2 cameras connected, the best would probably be to have the option to dynamically
// assign camera X to the primary video stream.

// Where the video stream transmitted via wifibroadcast is made available to QOpenHD to be picked up.
// I don't see a reason why not to use the same port the video is sent locally - it makes debugging much easier,
// aka for debugging one could theoretically just start QOpenHD on the air pi lol ;)
static constexpr auto OHD_VIDEO_GROUND_VIDEO_STREAM_1_UDP = 5600;
static constexpr auto OHD_VIDEO_GROUND_VIDEO_STREAM_2_UDP = 5601;
static_assert(OHD_VIDEO_GROUND_VIDEO_STREAM_1_UDP != OHD_VIDEO_GROUND_VIDEO_STREAM_2_UDP, "Must be different");

static constexpr auto OHD_VERSION_NUMBER_STRING="2.2.5-evo";

#endif //OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H
