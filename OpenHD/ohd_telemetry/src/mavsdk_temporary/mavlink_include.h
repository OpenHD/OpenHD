//
// Created by consti10 on 08.07.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_MAVLINK_INCLUDE_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_MAVLINK_INCLUDE_H_

extern "C" {
// NOTE: Make sure to include the openhd mavlink flavour, otherwise the custom
// messages won't bw parsed.
#include <openhd/mavlink.h>
}

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_MAV_PARAM_MAVLINK_INCLUDE_H_
