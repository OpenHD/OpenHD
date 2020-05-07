#pragma once

#include <stdlib.h>

#include <openhd/mavlink.h>


const char * sub_mode_from_enum(SUB_MODE mode);

const char * rover_mode_from_enum(ROVER_MODE mode);

const char * chinese_copter_mode_from_enum(COPTER_MODE mode);
const char * copter_mode_from_enum(COPTER_MODE mode);

const char * chinese_plane_mode_from_enum(PLANE_MODE mode);
const char * plane_mode_from_enum(PLANE_MODE mode);

const char * tracker_mode_from_enum(TRACKER_MODE mode);

const char * vot_mode_from_telemetry(uint8_t mode);

const char * chinese_ltm_mode_from_telem(int mode);
const char * ltm_mode_from_telem(int mode);

