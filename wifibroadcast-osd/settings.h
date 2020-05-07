#pragma once

#include <tuple>
#include <string>
#include <iostream>
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>



/*
 * OSD settings
 */
extern std::map<std::string, std::string> osd_settings;


std::pair<std::string, std::string> parse_osd_define(std::string kv);
std::tuple<double, double, double, double> load_osd_define_color(std::map<std::string, std::string> settings, std::string name, double default_red, double default_green, double default_blue, double default_alpha);
std::map<std::string, std::string> read_config(std::string path);
int load_int_setting(std::map<std::string, std::string> settings, std::string name, int default_value);
double load_double_setting(std::map<std::string, std::string> settings, std::string name, double default_value);
std::string load_string_setting(std::map<std::string, std::string> settings, std::string name, std::string default_value);
bool load_bool_setting(std::map<std::string, std::string> settings, std::string name, bool default_value);


void load_settings();


extern bool MAVLINK;
extern bool LTM;
extern bool SMARTPORT;
extern bool FRSKY;
extern bool VOT;

extern bool IMPERIAL;
extern bool COPTER;

extern bool HIDE_LATLON;

extern bool REVERSE_ALTITUDES;

extern int  REL_ALT_SOURCE;

extern bool CHINESE;

extern double COLOR_R;
extern double COLOR_G;
extern double COLOR_B;
extern double COLOR_A;
extern double OUTLINECOLOR_R;
extern double OUTLINECOLOR_G;
extern double OUTLINECOLOR_B;
extern double OUTLINECOLOR_A;

extern double OUTLINEWIDTH;

extern std::string FONT;

extern double GLOBAL_SCALE;

extern double COLOR_WARNING_R;
extern double COLOR_WARNING_G;
extern double COLOR_WARNING_B;
extern double COLOR_WARNING_A;
extern double COLOR_CAUTION_R;
extern double COLOR_CAUTION_G;
extern double COLOR_CAUTION_B;
extern double COLOR_CAUTION_A;
extern double COLOR_GOOD_R;
extern double COLOR_GOOD_G;
extern double COLOR_GOOD_B;
extern double COLOR_GOOD_A;
extern double COLOR_DECLUTTER_R;
extern double COLOR_DECLUTTER_G;
extern double COLOR_DECLUTTER_B;
extern double COLOR_DECLUTTER_A;



extern bool DOWNLINK_RSSI;
extern double DOWNLINK_RSSI_POS_X;
extern double DOWNLINK_RSSI_POS_Y;
extern double DOWNLINK_RSSI_SCALE;
extern bool DOWNLINK_RSSI_FEC_BAR;


extern bool DOWNLINK_RSSI_DETAILED;
extern double DOWNLINK_RSSI_DETAILED_POS_X;
extern double DOWNLINK_RSSI_DETAILED_POS_Y;
extern double DOWNLINK_RSSI_DETAILED_SCALE;


extern bool UPLINK_RSSI;
extern double UPLINK_RSSI_POS_X;
extern double UPLINK_RSSI_POS_Y;
extern double UPLINK_RSSI_SCALE;



extern bool RSSI;
extern double RSSI_POS_X;
extern double RSSI_POS_Y;
extern double RSSI_SCALE;
extern int RSSI_WARN;
extern int RSSI_CAUTION;
extern int RSSI_DECLUTTER;


extern bool KBITRATE;
extern double KBITRATE_POS_X;
extern double KBITRATE_POS_Y;
extern double KBITRATE_SCALE;
extern int KBITRATE_WARN;
extern int KBITRATE_CAUTION;
extern int KBITRATE_DECLUTTER;



extern bool SYS;
extern double SYS_POS_X;
extern double SYS_POS_Y;
extern double SYS_SCALE;
extern int CPU_LOAD_WARN;
extern int CPU_LOAD_CAUTION;
extern int CPU_TEMP_WARN;
extern int CPU_TEMP_CAUTION;
extern int SYS_DECLUTTER;


extern bool HOME_ARROW;
extern double HOME_ARROW_POS_X;
extern double HOME_ARROW_POS_Y;
extern double HOME_ARROW_SCALE;
extern bool HOME_ARROW_USECOG;
extern bool HOME_ARROW_INVERT;



extern bool BATT_STATUS;
extern double BATT_STATUS_POS_X;
extern double BATT_STATUS_POS_Y;
extern double BATT_STATUS_SCALE;
extern bool BATT_STATUS_CURRENT;



extern bool BATT_GAUGE;
extern double BATT_GAUGE_POS_X;
extern double BATT_GAUGE_POS_Y;
extern double BATT_GAUGE_SCALE;
extern int CELLS;
extern double CELL_MAX;
extern double CELL_MIN;
extern double CELL_WARNING1;
extern double CELL_WARNING2;


extern bool COMPASS;
extern double COMPASS_POS_Y;
extern double COMPASS_SCALE;
extern bool COMPASS_USECOG;
extern bool COMPASS_INAV;
extern bool COMPASS_COMPLEX;
extern int COMPASS_LEN;
extern bool COMPASS_BEARING;



extern bool ALTLADDER;
extern double ALTLADDER_POS_X;
extern double ALTLADDER_SCALE;
extern int ALTLADDER_WARN;
extern int ALTLADDER_CAUTION;
extern int ALTLADDER_VSI_TIME;


extern bool MSLALT;
extern int MSLALT_POS_X;
extern int MSLALT_POS_Y;
extern double MSLALT_SCALE;


extern bool SPEEDLADDER;
extern double SPEEDLADDER_POS_X;
extern double SPEEDLADDER_SCALE;
extern bool SPEEDLADDER_USEAIRSPEED;
extern int SPEEDLADDER_TREND_TIME;
extern int SPEEDLADDER_LOW_LIMIT;


extern bool YAWDISPLAY;
extern double YAWDISPLAY_POS_X;
extern double YAWDISPLAY_POS_Y;
extern double YAWDISPLAY_SCALE;
extern int YAWDISPLAY_TREND_TIME;




extern bool AHI;
extern double AHI_SCALE;
extern bool AHI_LADDER;
extern int AHI_INVERT_ROLL;
extern int AHI_INVERT_PITCH;
extern bool AHI_SWAP_ROLL_AND_PITCH;
extern bool AHI_ROLLANGLE;
extern int AHI_ROLLANGLE_INVERT;


extern bool POSITION;
extern double POSITION_POS_X;
extern double POSITION_POS_Y;
extern double POSITION_SCALE;



extern bool SAT;
extern double SAT_POS_X;
extern double SAT_POS_Y;
extern double SAT_SCALE;
extern int SAT_HDOP_WARN;
extern int SAT_HDOP_CAUTION;
extern int SAT_DECLUTTER;


extern bool DISTANCE;
extern double DISTANCE_POS_X;
extern double DISTANCE_POS_Y;
extern double DISTANCE_SCALE;


extern bool FLIGHTMODE;
extern double FLIGHTMODE_POS_X;
extern double FLIGHTMODE_POS_Y;
extern double FLIGHTMODE_SCALE;


extern bool CLIMB;
extern double CLIMB_POS_X;
extern double CLIMB_POS_Y;
extern double CLIMB_SCALE;


extern bool COURSE_OVER_GROUND;
extern double COURSE_OVER_GROUND_POS_X;
extern double COURSE_OVER_GROUND_POS_Y;
extern double COURSE_OVER_GROUND_SCALE;


extern bool GPSSPEED;
extern double GPSSPEED_POS_X;
extern double GPSSPEED_POS_Y;
extern double GPSSPEED_SCALE;



extern bool AIRSPEED;
extern double AIRSPEED_POS_X;
extern double AIRSPEED_POS_Y;
extern double AIRSPEED_SCALE;


extern double WARNING_POS_X;
extern double WARNING_POS_Y;


extern bool TOTAL_AMPS;
extern double TOTAL_AMPS_POS_X;
extern double TOTAL_AMPS_POS_Y;
extern double TOTAL_AMPS_SCALE;


extern bool TOTAL_DIST;
extern double TOTAL_DIST_POS_X;
extern double TOTAL_DIST_POS_Y;
extern double TOTAL_DIST_SCALE;


extern bool TOTAL_TIME;
extern double TOTAL_TIME_POS_X;
extern double TOTAL_TIME_POS_Y;
extern double TOTAL_TIME_SCALE;


extern bool HOME_RADAR;
extern double HOME_RADAR_POS_X;
extern double HOME_RADAR_POS_Y;
extern double HOME_RADAR_SCALE;
extern bool HOME_RADAR_USECOG;



extern bool RPA;
extern double RPA_POS_X;
extern double RPA_POS_Y;
extern double RPA_SCALE;
extern int RPA_INVERT_ROLL;
extern int RPA_INVERT_PITCH;


extern bool THROTTLE;
extern double THROTTLE_POS_X;
extern double THROTTLE_POS_Y;
extern double THROTTLE_SCALE;
extern bool THROTTLE_GAUGE;
extern int THROTTLE_TARGET;



extern bool THROTTLE_V2;
extern double THROTTLE_V2_POS_X;
extern double THROTTLE_V2_POS_Y;
extern double THROTTLE_V2_SCALE;
extern bool THROTTLE_V2_COMPLEX;


extern bool HDOP;
extern double HDOP_POS_X;
extern double HDOP_POS_Y;
extern double HDOP_SCALE;


extern bool MISSION;
extern double MISSION_POS_X;
extern double MISSION_POS_Y;
extern double MISSION_SCALE;


extern bool ANGLE;
extern double ANGLE_POS_X;
extern double ANGLE_POS_Y;
extern double ANGLE_SCALE;


extern bool ANGLE2;
extern double ANGLE2_POS_X;
extern double ANGLE2_POS_Y;
extern double ANGLE2_SCALE;


extern bool ALARM;
extern double ALARM_POS_X;
extern double ALARM_POS_Y;
extern double ALARM_SCALE;

extern bool ALARM_1;
extern bool ALARM_2;
extern bool ALARM_3;
extern bool ALARM_4;
extern bool ALARM_5;
extern bool ALARM_6;
extern bool ALARM_7;
extern bool ALARM_8;
extern bool ALARM_9;
extern bool ALARM_10;
extern bool ALARM_11;
extern bool ALARM_12;
extern bool ALARM_13;
extern bool ALARM_14;
extern bool ALARM_15;
extern bool ALARM_16;
extern bool ALARM_17;
extern bool ALARM_18;
extern bool ALARM_19;
extern bool ALARM_20;
extern bool ALARM_21;
extern bool ALARM_22;
extern bool ALARM_23;
extern bool ALARM_24;
extern bool ALARM_25;
extern bool ALARM_26;
