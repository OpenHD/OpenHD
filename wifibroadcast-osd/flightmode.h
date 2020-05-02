#include <stdlib.h>

#include <openhd/mavlink.h>


char * sub_mode_from_enum(SUB_MODE mode);

char * rover_mode_from_enum(ROVER_MODE mode);

char * copter_mode_from_enum(COPTER_MODE mode);

char * plane_mode_from_enum(PLANE_MODE mode);

char * tracker_mode_from_enum(TRACKER_MODE mode);

char * vot_mode_from_telemetry(uint8_t mode);

char * ltm_mode_from_telem(int mode);

