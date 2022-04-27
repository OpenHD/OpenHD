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
static constexpr auto OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX=16550;
static constexpr auto OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX=16551;
static_assert(OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_RX != OHD_TELEMETRY_WIFIBROADCAST_LOCAL_UDP_PORT_GROUND_TX,"Must be different");
// The radio_port in wifibroadcast
static constexpr auto OHD_TELEMETRY_WIFIBROADCAST_RF_RX_PORT_ID=3;
static constexpr auto OHD_TELEMETRY_WIFIBROADCAST_RF_TX_PORT_ID=4;
static_assert(OHD_TELEMETRY_RF_RX_PORT_ID!= OHD_TELEMETRY_RF_TX_PORT_ID,"Must be different");

// Where any service can send log messages to
static constexpr auto OHD_LOCAL_LOG_MESSAGES_UDP_PORT=50000;

#endif //OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H
