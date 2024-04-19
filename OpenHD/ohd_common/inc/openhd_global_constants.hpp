//
// Created by consti10 on 27.04.22.
//

#ifndef OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H
#define OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H

#include <sstream>
// This file should only contain const values that are needed by more than one
// module.

namespace openhd {
// The radio_ports for telemetry in wifibroadcast. These variables are from the
// perspective of the air pi aka for the ground pi, tx and rx radio ports are
// the opposite.
static constexpr auto TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT = 3;
static constexpr auto TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT = 4;
static_assert(TELEMETRY_WIFIBROADCAST_RX_RADIO_PORT !=
                  TELEMETRY_WIFIBROADCAST_TX_RADIO_PORT,
              "Must be different");

// Video is unidirectional from air to ground
static constexpr auto VIDEO_PRIMARY_RADIO_PORT = 10;
static constexpr auto VIDEO_SECONDARY_RADIO_PORT = 11;
static_assert(VIDEO_PRIMARY_RADIO_PORT != VIDEO_SECONDARY_RADIO_PORT,
              "Must be different");

// management - air to ground and ground to air
static constexpr auto MANAGEMENT_RADIO_PORT_AIR_TX = 20;
static constexpr auto MANAGEMENT_RADIO_PORT_GND_TX = 21;

// Audio is unidirectional from air to ground
static constexpr auto AUDIO_WIFIBROADCAST_PORT = 30;

// Where the video stream transmitted via wifibroadcast is made available to
// QOpenHD to be picked up.
static constexpr auto VIDEO_GROUND_VIDEO_STREAM_1_UDP = 5600;
static constexpr auto VIDEO_GROUND_VIDEO_STREAM_2_UDP = 5601;
static_assert(VIDEO_GROUND_VIDEO_STREAM_1_UDP !=
                  VIDEO_GROUND_VIDEO_STREAM_2_UDP,
              "Must be different");

static constexpr uint8_t MAJOR_VERSION = 2;
static constexpr uint8_t MINOR_VERSION = 5;
static constexpr uint8_t PATCH_VERSION = 4;
static constexpr uint8_t RELEASE_TYPE = 1;
static std::string ohd_version_as_string(uint8_t major, uint8_t minor,
                                         uint8_t patch, uint8_t release_type) {
  std::stringstream ss;
  ss << (int)major << "." << (int)minor << "." << (int)patch << "-evo";
  if (release_type == 0) {
    ss << "-release";
  } else if (release_type == 1) {
    ss << "-beta";
  } else if (release_type == 2) {
    ss << "-alpha";
  } else {
    ss << "-unknown";
  }
  return ss.str();
}
static std::string get_ohd_version_as_string() {
  return ohd_version_as_string(MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION,
                               RELEASE_TYPE);
}

// This optional file contains an encryption keypair (up/down).
// It is generated at first boot if the user specifies a pw during flash.
// If this file does not exist, the default keypair from the default pw (openhd)
// is generated at run time.
static constexpr auto SECURITY_KEYPAIR_FILENAME =
    "/usr/local/share/openhd/txrx.key";

}  // namespace openhd
#endif  // OPEN_HD_OPNHD_GLOBAL_CONSTANTS_H
