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

enum PX4_CUSTOM_MAIN_MODE {
    PX4_CUSTOM_MAIN_MODE_MANUAL = 1,
    PX4_CUSTOM_MAIN_MODE_ALTCTL,
    PX4_CUSTOM_MAIN_MODE_POSCTL,
    PX4_CUSTOM_MAIN_MODE_AUTO,
    PX4_CUSTOM_MAIN_MODE_ACRO,
    PX4_CUSTOM_MAIN_MODE_OFFBOARD,
    PX4_CUSTOM_MAIN_MODE_STABILIZED,
    PX4_CUSTOM_MAIN_MODE_RATTITUDE,
    PX4_CUSTOM_MAIN_MODE_SIMPLE
};

enum PX4_CUSTOM_SUB_MODE_AUTO {
    PX4_CUSTOM_SUB_MODE_AUTO_READY = 1,
    PX4_CUSTOM_SUB_MODE_AUTO_TAKEOFF,
    PX4_CUSTOM_SUB_MODE_AUTO_LOITER,
    PX4_CUSTOM_SUB_MODE_AUTO_MISSION,
    PX4_CUSTOM_SUB_MODE_AUTO_RTL,
    PX4_CUSTOM_SUB_MODE_AUTO_LAND,
    PX4_CUSTOM_SUB_MODE_AUTO_RESERVED_DO_NOT_USE,
    PX4_CUSTOM_SUB_MODE_AUTO_FOLLOW_TARGET,
    PX4_CUSTOM_SUB_MODE_AUTO_PRECLAND
};

enum PX4_CUSTOM_SUB_MODE_POSCTL {
    PX4_CUSTOM_SUB_MODE_POSCTL_POSCTL = 0,
    PX4_CUSTOM_SUB_MODE_POSCTL_ORBIT
};

union px4_custom_mode {
    struct {
        uint16_t reserved;
        uint8_t main_mode;
        uint8_t sub_mode;
    };
    uint32_t data;
    float data_float;
    struct {
        uint16_t reserved_hl;
        uint16_t custom_mode_hl;
    };
};

const char * px4_mode_from_custom_mode(int custom_mode);
