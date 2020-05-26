
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


#include "settings.h"


std::map<std::string, std::string> osd_settings;

bool MAVLINK;
bool LTM;
bool SMARTPORT;
bool FRSKY;
bool VOT;

bool IMPERIAL;
bool COPTER;

bool HIDE_LATLON;

bool REVERSE_ALTITUDES;

int REL_ALT_SOURCE;

bool CHINESE;

double COLOR_R, COLOR_G, COLOR_B, COLOR_A;
double OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A;

double OUTLINEWIDTH;

std::string FONT;

double GLOBAL_SCALE;

double COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A;
double COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A;
double COLOR_GOOD_R, COLOR_GOOD_G, COLOR_GOOD_B, COLOR_GOOD_A;
double COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A;


bool DOWNLINK_RSSI;
double DOWNLINK_RSSI_POS_X;
double DOWNLINK_RSSI_POS_Y;
double DOWNLINK_RSSI_SCALE;
bool DOWNLINK_RSSI_FEC_BAR;


bool DOWNLINK_RSSI_DETAILED;
double DOWNLINK_RSSI_DETAILED_POS_X;
double DOWNLINK_RSSI_DETAILED_POS_Y;
double DOWNLINK_RSSI_DETAILED_SCALE;


bool UPLINK_RSSI;
double UPLINK_RSSI_POS_X;
double UPLINK_RSSI_POS_Y;
double UPLINK_RSSI_SCALE;



bool RSSI;
double RSSI_POS_X;
double RSSI_POS_Y;
double RSSI_SCALE;
int RSSI_WARN;
int RSSI_CAUTION;
int RSSI_DECLUTTER;


bool KBITRATE;
double KBITRATE_POS_X;
double KBITRATE_POS_Y;
double KBITRATE_SCALE;
int KBITRATE_WARN;
int KBITRATE_CAUTION;
int KBITRATE_DECLUTTER;



bool SYS;
double SYS_POS_X;
double SYS_POS_Y;
double SYS_SCALE;
int CPU_LOAD_WARN;
int CPU_LOAD_CAUTION;
int CPU_TEMP_WARN;
int CPU_TEMP_CAUTION;
int SYS_DECLUTTER;


bool HOME_ARROW;
double HOME_ARROW_POS_X;
double HOME_ARROW_POS_Y;
double HOME_ARROW_SCALE;
bool HOME_ARROW_USECOG;
bool HOME_ARROW_INVERT;



bool BATT_STATUS;
double BATT_STATUS_POS_X;
double BATT_STATUS_POS_Y;
double BATT_STATUS_SCALE;
bool BATT_STATUS_CURRENT;



bool BATT_GAUGE;
double BATT_GAUGE_POS_X;
double BATT_GAUGE_POS_Y;
double BATT_GAUGE_SCALE;
int CELLS;
double CELL_MAX;
double CELL_MIN;
double CELL_WARNING1;
double CELL_WARNING2;


bool COMPASS;
double COMPASS_POS_Y;
double COMPASS_SCALE;
bool COMPASS_USECOG;
bool COMPASS_INAV;
bool COMPASS_COMPLEX;
int COMPASS_LEN;
bool COMPASS_BEARING;



bool ALTLADDER;
double ALTLADDER_POS_X;
double ALTLADDER_SCALE;
int ALTLADDER_WARN;
int ALTLADDER_CAUTION;
int ALTLADDER_VSI_TIME;


bool MSLALT;
int MSLALT_POS_X;
int MSLALT_POS_Y;
double MSLALT_SCALE;


bool SPEEDLADDER;
double SPEEDLADDER_POS_X;
double SPEEDLADDER_SCALE;
bool SPEEDLADDER_USEAIRSPEED;
int SPEEDLADDER_TREND_TIME;
int SPEEDLADDER_LOW_LIMIT;


bool YAWDISPLAY;
double YAWDISPLAY_POS_X;
double YAWDISPLAY_POS_Y;
double YAWDISPLAY_SCALE;
int YAWDISPLAY_TREND_TIME;




bool AHI;
double AHI_SCALE;
bool AHI_LADDER;
int AHI_INVERT_ROLL;
int AHI_INVERT_PITCH;
bool AHI_SWAP_ROLL_AND_PITCH;
bool AHI_ROLLANGLE;
int AHI_ROLLANGLE_INVERT;


bool POSITION;
double POSITION_POS_X;
double POSITION_POS_Y;
double POSITION_SCALE;



bool SAT;
double SAT_POS_X;
double SAT_POS_Y;
double SAT_SCALE;
int SAT_HDOP_WARN;
int SAT_HDOP_CAUTION;
int SAT_DECLUTTER;


bool DISTANCE;
double DISTANCE_POS_X;
double DISTANCE_POS_Y;
double DISTANCE_SCALE;


bool FLIGHTMODE;
double FLIGHTMODE_POS_X;
double FLIGHTMODE_POS_Y;
double FLIGHTMODE_SCALE;


bool CLIMB;
double CLIMB_POS_X;
double CLIMB_POS_Y;
double CLIMB_SCALE;


bool COURSE_OVER_GROUND;
double COURSE_OVER_GROUND_POS_X;
double COURSE_OVER_GROUND_POS_Y;
double COURSE_OVER_GROUND_SCALE;


bool GPSSPEED;
double GPSSPEED_POS_X;
double GPSSPEED_POS_Y;
double GPSSPEED_SCALE;



bool AIRSPEED;
double AIRSPEED_POS_X;
double AIRSPEED_POS_Y;
double AIRSPEED_SCALE;


double WARNING_POS_X;
double WARNING_POS_Y;


bool TOTAL_AMPS;
double TOTAL_AMPS_POS_X;
double TOTAL_AMPS_POS_Y;
double TOTAL_AMPS_SCALE;


bool TOTAL_DIST;
double TOTAL_DIST_POS_X;
double TOTAL_DIST_POS_Y;
double TOTAL_DIST_SCALE;


bool TOTAL_TIME;
double TOTAL_TIME_POS_X;
double TOTAL_TIME_POS_Y;
double TOTAL_TIME_SCALE;


bool HOME_RADAR;
double HOME_RADAR_POS_X;
double HOME_RADAR_POS_Y;
double HOME_RADAR_SCALE;
bool HOME_RADAR_USECOG;



bool RPA;
double RPA_POS_X;
double RPA_POS_Y;
double RPA_SCALE;
int RPA_INVERT_ROLL;
int RPA_INVERT_PITCH;


bool THROTTLE;
double THROTTLE_POS_X;
double THROTTLE_POS_Y;
double THROTTLE_SCALE;
bool THROTTLE_GAUGE;
int THROTTLE_TARGET;



bool THROTTLE_V2;
double THROTTLE_V2_POS_X;
double THROTTLE_V2_POS_Y;
double THROTTLE_V2_SCALE;
bool THROTTLE_V2_COMPLEX;


bool HDOP;
double HDOP_POS_X;
double HDOP_POS_Y;
double HDOP_SCALE;


bool MISSION;
double MISSION_POS_X;
double MISSION_POS_Y;
double MISSION_SCALE;


bool ANGLE;
double ANGLE_POS_X;
double ANGLE_POS_Y;
double ANGLE_SCALE;


bool ANGLE2;
double ANGLE2_POS_X;
double ANGLE2_POS_Y;
double ANGLE2_SCALE;


bool ALARM;
double ALARM_POS_X;
double ALARM_POS_Y;
double ALARM_SCALE;

bool ALARM_1;
bool ALARM_2;
bool ALARM_3;
bool ALARM_4;
bool ALARM_5;
bool ALARM_6;
bool ALARM_7;
bool ALARM_8;
bool ALARM_9;
bool ALARM_10;
bool ALARM_11;
bool ALARM_12;
bool ALARM_13;
bool ALARM_14;
bool ALARM_15;
bool ALARM_16;
bool ALARM_17;
bool ALARM_18;
bool ALARM_19;
bool ALARM_20;
bool ALARM_21;
bool ALARM_22;
bool ALARM_23;
bool ALARM_24;
bool ALARM_25;
bool ALARM_26;




/*
 * This is mostly duplicated from the same code in wifibroadcast-rc-Ath9k, but it should not
 * be centralized as it is slightly modified to account for the way osdconfig.txt is written.
 * Instead we should just wait until the old OSD is no longer being used and leave it at that.
 * 
 * The reusable settings code in the rc directory should be centralized instead.
 */ 
std::pair<std::string, std::string> parse_osd_define(std::string kv) {
    boost::smatch result;

    boost::regex r(R"(^#define\s+([\w]+)\s*([\w\.\"\-\,]+)?.*\r?)");
    if (!boost::regex_match(kv, result, r)) {
        throw std::runtime_error("Ignoring invalid setting, check file for errors");
    }

    if (result.size() == 2) {
        /*
         * Special case for defines that have no value, which means the value is true. 
        
         * They can also explicitly say false, which is handled by the normal case at 
         * the end of this function.
         */
        return std::make_pair<std::string, std::string>(result[1], "true");
    }

    if (result.size() != 3) {
        throw std::runtime_error("Ignoring invalid setting, check file for errors");
    }

    return std::make_pair<std::string, std::string>(result[1], result[2]);
}


std::map<std::string, std::string> read_config(std::string path) {
    std::ifstream in(path);

    std::map<std::string, std::string> settings;

    std::string str;

    while (std::getline(in, str)) {
        if (str.size() > 0) {
            try {
                auto pair = parse_osd_define(str);
                settings.insert(pair);
            } catch (std::exception &ex) {
                /* 
                 * Ignore, likely a comment or user error in which case we will use a default.
                 * 
                 * We might need to mark the ones that could not be read so we can warn the user, but
                 * not by printing messages to the console that they might never see.
                 */
            }
        }
    }

    return settings;
}


std::tuple<double, double, double, double> load_osd_define_color(std::map<std::string, std::string> settings, std::string name, double default_red, double default_green, double default_blue, double default_alpha) {
    auto search = settings.find(name);
    if (search != settings.end()) {
        boost::smatch result;

        boost::regex r{ "([\\w\\.]+),\\s*([\\w\\.]+),\\s*([\\w\\.]+),\\s*([\\w\\.]+)" };
        if (boost::regex_match(search->second, result, r)) {
            std::string r = result[1];
            std::string g = result[2];
            std::string b = result[3];
            std::string a = result[4];
            return std::tuple<double, double, double, double>(atof(r.c_str()), atof(g.c_str()), atof(b.c_str()), atof(a.c_str()));
        } else {
            std::cerr << "Ignoring invalid color setting: " << name << ", check file for errors" << std::endl;
        }
    } else {
        std::cerr << "WARNING: " << name << " not found in settings file, using default values" << std::endl;
    }

    return std::tuple<double, double, double, double>(default_red, default_green, default_blue, default_alpha);
}


int load_int_setting(std::map<std::string, std::string> settings, std::string name, int default_value) {
    auto search = settings.find(name);
    if (search != settings.end()) {
        return atoi(search->second.c_str());
    } else {
        std::cerr << "WARNING: " << name << " not found in settings file, using default value: " << default_value << std::endl;
        return default_value;
    }
}


double load_double_setting(std::map<std::string, std::string> settings, std::string name, double default_value) {
    auto search = settings.find(name);
    if (search != settings.end()) {
        return atof(search->second.c_str());
    } else {
        std::cerr << "WARNING: " << name << " not found in settings file, using default value: " << default_value << std::endl;
        return default_value;
    }
}


std::string load_string_setting(std::map<std::string, std::string> settings, std::string name, std::string default_value) {
    auto search = settings.find(name);
    if (search != settings.end()) {
        return search->second;
    } else {
        std::cerr << "WARNING: " << name << " not found in settings file, using default value: " << default_value << std::endl;
        return default_value;
    }
}


bool load_bool_setting(std::map<std::string, std::string> settings, std::string name, bool default_value) {
    auto search = settings.find(name);
    if (search != settings.end()) {
        return search->second != "false";
    } else {
        std::cerr << "WARNING: " << name << " not found in settings file, using default value: " << default_value << std::endl;
        return default_value;
    }
}


void load_settings() {



    osd_settings = read_config("/boot/osdconfig.txt");

    
    /*
     * Process the OSD settings we care about and store them
     */
    MAVLINK = load_bool_setting(osd_settings, "MAVLINK", false);
    LTM = load_bool_setting(osd_settings, "LTM", false);
    SMARTPORT = load_bool_setting(osd_settings, "SMARTPORT", false);
    FRSKY = load_bool_setting(osd_settings, "FRSKY", false);
    VOT = load_bool_setting(osd_settings, "VOT", false);

    IMPERIAL = load_bool_setting(osd_settings, "IMPERIAL", false);
    COPTER = load_bool_setting(osd_settings, "COPTER", true);

    HIDE_LATLON = load_bool_setting(osd_settings, "HIDE_LATLON", false);

    REVERSE_ALTITUDES = load_bool_setting(osd_settings, "REVERSE_ALTITUDES", false);

    REL_ALT_SOURCE = load_int_setting(osd_settings, "REL_ALT_SOURCE", 1);

    CHINESE = load_bool_setting(osd_settings, "CHINESE", false);


    std::tie(COLOR_R, COLOR_G, COLOR_B, COLOR_A) = load_osd_define_color(osd_settings, "COLOR", 255, 255, 255, 0.7);
    std::tie(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A) = load_osd_define_color(osd_settings, "OUTLINECOLOR", 0, 0, 0, 0.5);
    
    OUTLINEWIDTH = load_double_setting(osd_settings, "OUTLINEWIDTH", 1.2);

    FONT = load_string_setting(osd_settings, "FONT", "Archivo-Bold.ttf");
    size_t pos = 0;
    while ((pos = FONT.find('"', pos)) != std::string::npos) {
        FONT = FONT.erase(pos, 1);
    }

    GLOBAL_SCALE = load_double_setting(osd_settings, "GLOBAL_SCALE", 1.2);

    std::tie(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A) = load_osd_define_color(osd_settings, "COLOR_WARNING", 255, 20, 20, 0.9);
    std::tie(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A) = load_osd_define_color(osd_settings, "COLOR_CAUTION", 245, 222, 20, 0.8);
    std::tie(COLOR_GOOD_R, COLOR_GOOD_G, COLOR_GOOD_B, COLOR_GOOD_A) = load_osd_define_color(osd_settings, "COLOR_GOOD", 43, 240, 36, 0.7);
    std::tie(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A) = load_osd_define_color(osd_settings, "COLOR_DECLUTTER", 255, 255, 255, 0.0);


    DOWNLINK_RSSI = load_bool_setting(osd_settings, "DOWNLINK_RSSI", false);
    DOWNLINK_RSSI_POS_X = load_double_setting(osd_settings, "DOWNLINK_RSSI_POS_X", 11);
    DOWNLINK_RSSI_POS_Y = load_double_setting(osd_settings, "DOWNLINK_RSSI_POS_Y", 89);
    DOWNLINK_RSSI_SCALE = load_double_setting(osd_settings, "DOWNLINK_RSSI_SCALE", 1.0);
    DOWNLINK_RSSI_FEC_BAR = load_bool_setting(osd_settings, "DOWNLINK_RSSI_FEC_BAR", false);


    DOWNLINK_RSSI_DETAILED = load_bool_setting(osd_settings, "DOWNLINK_RSSI_DETAILED", false);
    DOWNLINK_RSSI_DETAILED_POS_X = load_double_setting(osd_settings, "DOWNLINK_RSSI_DETAILED_POS_X", 28);
    DOWNLINK_RSSI_DETAILED_POS_Y = load_double_setting(osd_settings, "DOWNLINK_RSSI_DETAILED_POS_Y", 89);
    DOWNLINK_RSSI_DETAILED_SCALE = load_double_setting(osd_settings, "DOWNLINK_RSSI_DETAILED_SCALE", 0.75);


    UPLINK_RSSI = load_bool_setting(osd_settings, "UPLINK_RSSI", false);
    UPLINK_RSSI_POS_X = load_double_setting(osd_settings, "UPLINK_RSSI_POS_X", 92);
    UPLINK_RSSI_POS_Y = load_double_setting(osd_settings, "DOWNLINK_RSSI_DETAILED_POS_Y", 89);
    UPLINK_RSSI_SCALE = load_double_setting(osd_settings, "UPLINK_RSSI_SCALE", 1);



    RSSI = load_bool_setting(osd_settings, "RSSI", false);
    RSSI_POS_X = load_double_setting(osd_settings, "RSSI_POS_X", 92);
    RSSI_POS_Y = load_double_setting(osd_settings, "RSSI_POS_Y", 77);
    RSSI_SCALE = load_double_setting(osd_settings, "RSSI_SCALE", 1);
    RSSI_WARN = load_int_setting(osd_settings, "RSSI_WARN", 25);
    RSSI_CAUTION = load_int_setting(osd_settings, "RSSI_CAUTION", 40);
    RSSI_DECLUTTER = load_int_setting(osd_settings, "RSSI_DECLUTTER", 1);


    KBITRATE = load_bool_setting(osd_settings, "KBITRATE", false);
    KBITRATE_POS_X = load_double_setting(osd_settings, "KBITRATE_POS_X", 68);
    KBITRATE_POS_Y = load_double_setting(osd_settings, "KBITRATE_POS_Y", 89);
    KBITRATE_SCALE = load_double_setting(osd_settings, "KBITRATE_SCALE", 1.0);
    KBITRATE_WARN = load_int_setting(osd_settings, "KBITRATE_WARN", 98);
    KBITRATE_CAUTION = load_int_setting(osd_settings, "KBITRATE_CAUTION", 50);
    KBITRATE_DECLUTTER = load_int_setting(osd_settings, "KBITRATE_DECLUTTER", 1);



    SYS = load_bool_setting(osd_settings, "SYS", false);
    SYS_POS_X = load_double_setting(osd_settings, "SYS_POS_X", 79);
    SYS_POS_Y = load_double_setting(osd_settings, "SYS_POS_Y", 89);
    SYS_SCALE = load_double_setting(osd_settings, "SYS_SCALE", 0.6);
    CPU_LOAD_WARN = load_int_setting(osd_settings, "CPU_LOAD_WARN", 60);
    CPU_LOAD_CAUTION = load_int_setting(osd_settings, "CPU_LOAD_CAUTION", 50);
    CPU_TEMP_WARN = load_int_setting(osd_settings, "CPU_TEMP_WARN", 70);
    CPU_TEMP_CAUTION = load_int_setting(osd_settings, "CPU_TEMP_CAUTION", 60);
    SYS_DECLUTTER = load_int_setting(osd_settings, "SYS_DECLUTTER", 1);


    HOME_ARROW = load_bool_setting(osd_settings, "HOME_ARROW", false);
    HOME_ARROW_POS_X = load_double_setting(osd_settings, "HOME_ARROW_POS_X", 50);
    HOME_ARROW_POS_Y = load_double_setting(osd_settings, "HOME_ARROW_POS_Y", 75);
    HOME_ARROW_SCALE = load_double_setting(osd_settings, "HOME_ARROW_SCALE", 1.0);
    HOME_ARROW_USECOG = load_bool_setting(osd_settings, "HOME_ARROW_USECOG", false);
    HOME_ARROW_INVERT = load_bool_setting(osd_settings, "HOME_ARROW_INVERT", false);



    BATT_STATUS = load_bool_setting(osd_settings, "BATT_STATUS", false);
    BATT_STATUS_POS_X = load_double_setting(osd_settings, "BATT_STATUS_POS_X", 9);
    BATT_STATUS_POS_Y = load_double_setting(osd_settings, "BATT_STATUS_POS_Y", 12);
    BATT_STATUS_SCALE = load_double_setting(osd_settings, "BATT_STATUS_SCALE", 1.0);
    BATT_STATUS_CURRENT = load_bool_setting(osd_settings, "BATT_STATUS_CURRENT", false);



    BATT_GAUGE = load_bool_setting(osd_settings, "BATT_GAUGE", false);
    BATT_GAUGE_POS_X = load_double_setting(osd_settings, "BATT_GAUGE_POS_X", 5);
    BATT_GAUGE_POS_Y = load_double_setting(osd_settings, "BATT_GAUGE_POS_Y", 5);
    BATT_GAUGE_SCALE = load_double_setting(osd_settings, "BATT_GAUGE_SCALE", 1.0);
    CELLS = load_int_setting(osd_settings, "CELLS", 4);
    CELL_MAX = load_double_setting(osd_settings, "CELL_MAX", 4.20);
    CELL_MIN = load_double_setting(osd_settings, "CELL_MIN", 3.20);
    CELL_WARNING1 = load_double_setting(osd_settings, "CELL_WARNING1", 3.50);
    CELL_WARNING2 = load_double_setting(osd_settings, "CELL_WARNING2", 3.40);


    COMPASS = load_bool_setting(osd_settings, "COMPASS", false);
    COMPASS_POS_Y = load_double_setting(osd_settings, "COMPASS_POS_Y", 91);
    COMPASS_SCALE = load_double_setting(osd_settings, "COMPASS_SCALE", 1.0);
    COMPASS_USECOG = load_bool_setting(osd_settings, "COMPASS_USECOG", false);
    COMPASS_INAV = load_bool_setting(osd_settings, "COMPASS_INAV", false);
    COMPASS_COMPLEX = load_bool_setting(osd_settings, "COMPASS_COMPLEX", false);
    COMPASS_LEN = load_int_setting(osd_settings, "COMPASS_LEN", 45);
    COMPASS_BEARING = load_bool_setting(osd_settings, "COMPASS_BEARING", false);



    ALTLADDER = load_bool_setting(osd_settings, "ALTLADDER", false);
    ALTLADDER_POS_X = load_double_setting(osd_settings, "ALTLADDER_POS_X", 83);
    ALTLADDER_SCALE = load_double_setting(osd_settings, "ALTLADDER_SCALE", 1.1);
    ALTLADDER_WARN = load_int_setting(osd_settings, "ALTLADDER_WARN", 5);
    ALTLADDER_CAUTION = load_int_setting(osd_settings, "ALTLADDER_CAUTION", 20);
    ALTLADDER_VSI_TIME = load_int_setting(osd_settings, "ALTLADDER_VSI_TIME", 20);


    MSLALT = load_bool_setting(osd_settings, "MSLALT", false);
    MSLALT_POS_X = load_double_setting(osd_settings, "MSLALT_POS_X", 93);
    MSLALT_POS_Y = load_double_setting(osd_settings, "MSLALT_POS_Y", 29);
    MSLALT_SCALE = load_double_setting(osd_settings, "MSLALT_SCALE", 0.6);


    SPEEDLADDER = load_bool_setting(osd_settings, "SPEEDLADDER", false);
    SPEEDLADDER_POS_X = load_double_setting(osd_settings, "SPEEDLADDER_POS_X", 16);
    SPEEDLADDER_SCALE = load_double_setting(osd_settings, "SPEEDLADDER_SCALE", 1.1);
    SPEEDLADDER_USEAIRSPEED = load_bool_setting(osd_settings, "SPEEDLADDER_USEAIRSPEED", false);
    SPEEDLADDER_TREND_TIME = load_int_setting(osd_settings, "SPEEDLADDER_TREND_TIME", 20);
    SPEEDLADDER_LOW_LIMIT = load_int_setting(osd_settings, "SPEEDLADDER_LOW_LIMIT", 0);



    YAWDISPLAY = load_bool_setting(osd_settings, "YAWDISPLAY", false);
    YAWDISPLAY_POS_X = load_double_setting(osd_settings, "YAWDISPLAY_POS_X", 50);
    YAWDISPLAY_POS_Y = load_double_setting(osd_settings, "YAWDISPLAY_POS_Y", 81);
    YAWDISPLAY_SCALE = load_double_setting(osd_settings, "YAWDISPLAY_SCALE", 1.0);
    YAWDISPLAY_TREND_TIME = load_int_setting(osd_settings, "YAWDISPLAY_TREND_TIME", 20);



    AHI = load_bool_setting(osd_settings, "AHI", false);
    AHI_SCALE = load_double_setting(osd_settings, "AHI_SCALE", 1.0);
    AHI_LADDER = load_bool_setting(osd_settings, "AHI_LADDER", false);
    AHI_INVERT_ROLL = load_int_setting(osd_settings, "AHI_INVERT_ROLL", 1);
    AHI_INVERT_PITCH = load_int_setting(osd_settings, "AHI_INVERT_PITCH", 1);
    AHI_SWAP_ROLL_AND_PITCH = load_bool_setting(osd_settings, "AHI_SWAP_ROLL_AND_PITCH", false);
    AHI_ROLLANGLE = load_bool_setting(osd_settings, "AHI_ROLLANGLE", false);
    AHI_ROLLANGLE_INVERT = load_int_setting(osd_settings, "AHI_ROLLANGLE_INVERT", -1);


    POSITION = load_bool_setting(osd_settings, "POSITION", false);
    POSITION_POS_X = load_double_setting(osd_settings, "POSITION_POS_X", 75);
    POSITION_POS_Y = load_double_setting(osd_settings, "POSITION_POS_Y", 5);
    POSITION_SCALE = load_double_setting(osd_settings, "POSITION_SCALE", 0.8);



    SAT = load_bool_setting(osd_settings, "SAT", false);
    SAT_POS_X = load_double_setting(osd_settings, "SAT_POS_X", 64);
    SAT_POS_Y = load_double_setting(osd_settings, "SAT_POS_Y", 14);
    SAT_SCALE = load_double_setting(osd_settings, "SAT_SCALE", 0.8);
    SAT_HDOP_WARN = load_int_setting(osd_settings, "SAT_HDOP_WARN", 2);
    SAT_HDOP_CAUTION = load_int_setting(osd_settings, "SAT_HDOP_CAUTION", 1);
    SAT_DECLUTTER = load_int_setting(osd_settings, "SAT_DECLUTTER", 1);


    DISTANCE = load_bool_setting(osd_settings, "DISTANCE", false);
    DISTANCE_POS_X = load_double_setting(osd_settings, "DISTANCE_POS_X", 94);
    DISTANCE_POS_Y = load_double_setting(osd_settings, "DISTANCE_POS_Y", 5);
    DISTANCE_SCALE = load_double_setting(osd_settings, "DISTANCE_SCALE", 1.0);


    FLIGHTMODE = load_bool_setting(osd_settings, "FLIGHTMODE", false);
    FLIGHTMODE_POS_X = load_double_setting(osd_settings, "FLIGHTMODE_POS_X", 50);
    FLIGHTMODE_POS_Y = load_double_setting(osd_settings, "FLIGHTMODE_POS_Y", 6);
    FLIGHTMODE_SCALE = load_double_setting(osd_settings, "FLIGHTMODE_SCALE", 1.0);


    CLIMB = load_bool_setting(osd_settings, "CLIMB", false);
    CLIMB_POS_X = load_double_setting(osd_settings, "CLIMB_POS_X", 93);
    CLIMB_POS_Y = load_double_setting(osd_settings, "CLIMB_POS_Y", 24);
    CLIMB_SCALE = load_double_setting(osd_settings, "CLIMB_SCALE", 0.6);


    COURSE_OVER_GROUND = load_bool_setting(osd_settings, "COURSE_OVER_GROUND", false);
    COURSE_OVER_GROUND_POS_X = load_double_setting(osd_settings, "COURSE_OVER_GROUND_POS_X", 92);
    COURSE_OVER_GROUND_POS_Y = load_double_setting(osd_settings, "COURSE_OVER_GROUND_POS_Y", 65);
    COURSE_OVER_GROUND_SCALE = load_double_setting(osd_settings, "COURSE_OVER_GROUND_SCALE", 0.8);


    GPSSPEED = load_bool_setting(osd_settings, "GPSSPEED", false);
    GPSSPEED_POS_X = load_double_setting(osd_settings, "GPSSPEED_POS_X", 8);
    GPSSPEED_POS_Y = load_double_setting(osd_settings, "GPSSPEED_POS_Y", 58);
    GPSSPEED_SCALE = load_double_setting(osd_settings, "GPSSPEED_SCALE", 0.8);



    AIRSPEED = load_bool_setting(osd_settings, "AIRSPEED", false);
    AIRSPEED_POS_X = load_double_setting(osd_settings, "AIRSPEED_POS_X", 8);
    AIRSPEED_POS_Y = load_double_setting(osd_settings, "AIRSPEED_POS_Y", 58);
    AIRSPEED_SCALE = load_double_setting(osd_settings, "AIRSPEED_SCALE", 0.8);


    WARNING_POS_X = load_double_setting(osd_settings, "WARNING_POS_X", 50);
    WARNING_POS_Y = load_double_setting(osd_settings, "WARNING_POS_Y", 25);



    TOTAL_AMPS = load_bool_setting(osd_settings, "TOTAL_AMPS", false);
    TOTAL_AMPS_POS_X = load_double_setting(osd_settings, "TOTAL_AMPS_POS_X", 24);
    TOTAL_AMPS_POS_Y = load_double_setting(osd_settings, "TOTAL_AMPS_POS_Y", 5);
    TOTAL_AMPS_SCALE = load_double_setting(osd_settings, "TOTAL_AMPS_SCALE", 1.0);


    TOTAL_DIST = load_bool_setting(osd_settings, "TOTAL_DIST", false);
    TOTAL_DIST_POS_X = load_double_setting(osd_settings, "TOTAL_DIST_POS_X", 50);
    TOTAL_DIST_POS_Y = load_double_setting(osd_settings, "TOTAL_DIST_POS_Y", 14);
    TOTAL_DIST_SCALE = load_double_setting(osd_settings, "TOTAL_DIST_SCALE", 0.8);


    TOTAL_TIME = load_bool_setting(osd_settings, "TOTAL_TIME", false);
    TOTAL_TIME_POS_X = load_double_setting(osd_settings, "TOTAL_TIME_POS_X", 92);
    TOTAL_TIME_POS_Y = load_double_setting(osd_settings, "TOTAL_TIME_POS_Y", 13);
    TOTAL_TIME_SCALE = load_double_setting(osd_settings, "TOTAL_TIME_SCALE", 0.8);


    HOME_RADAR = load_bool_setting(osd_settings, "HOME_RADAR", false);
    HOME_RADAR_POS_X = load_double_setting(osd_settings, "HOME_RADAR_POS_X", 60);
    HOME_RADAR_POS_Y = load_double_setting(osd_settings, "HOME_RADAR_POS_Y", 65);
    HOME_RADAR_SCALE = load_double_setting(osd_settings, "HOME_RADAR_SCALE", 0.8);
    HOME_RADAR_USECOG = load_bool_setting(osd_settings, "HOME_RADAR_USECOG", false);



    RPA = load_bool_setting(osd_settings, "RPA", false);
    RPA_POS_X = load_double_setting(osd_settings, "RPA_POS_X", 97);
    RPA_POS_Y = load_double_setting(osd_settings, "RPA_POS_Y", 66);
    RPA_SCALE = load_double_setting(osd_settings, "RPA_SCALE", 0.6);
    RPA_INVERT_ROLL = load_int_setting(osd_settings, "RPA_INVERT_ROLL", -1);
    RPA_INVERT_PITCH = load_int_setting(osd_settings, "RPA_INVERT_PITCH", 1);


    THROTTLE = load_bool_setting(osd_settings, "THROTTLE", false);
    THROTTLE_POS_X = load_double_setting(osd_settings, "THROTTLE_POS_X", 5);
    THROTTLE_POS_Y = load_double_setting(osd_settings, "THROTTLE_POS_Y", 28);
    THROTTLE_SCALE = load_double_setting(osd_settings, "THROTTLE_SCALE", 1);
    THROTTLE_GAUGE = load_bool_setting(osd_settings, "THROTTLE_GAUGE", false);
    THROTTLE_TARGET = load_int_setting(osd_settings, "THROTTLE_TARGET", 20);



    THROTTLE_V2 = load_bool_setting(osd_settings, "THROTTLE_V2", false);
    THROTTLE_V2_POS_X = load_double_setting(osd_settings, "THROTTLE_V2_POS_X", 10.5);
    THROTTLE_V2_POS_Y = load_double_setting(osd_settings, "THROTTLE_V2_POS_Y", 28);
    THROTTLE_V2_SCALE = load_double_setting(osd_settings, "THROTTLE_V2_SCALE", 0.65);
    THROTTLE_V2_COMPLEX = load_bool_setting(osd_settings, "THROTTLE_V2_COMPLEX", false);


    HDOP = load_bool_setting(osd_settings, "HDOP", false);
    HDOP_POS_X = load_double_setting(osd_settings, "HDOP_POS_X", 98);
    HDOP_POS_Y = load_double_setting(osd_settings, "HDOP_POS_Y", 95);
    HDOP_SCALE = load_double_setting(osd_settings, "HDOP_SCALE", 0.6);


    MISSION = load_bool_setting(osd_settings, "MISSION", false);
    MISSION_POS_X = load_double_setting(osd_settings, "MISSION_POS_X", 35);
    MISSION_POS_Y = load_double_setting(osd_settings, "MISSION_POS_Y", 1);
    MISSION_SCALE = load_double_setting(osd_settings, "MISSION_SCALE", 1);


    ANGLE = load_bool_setting(osd_settings, "ANGLE", false);
    ANGLE_POS_X = load_double_setting(osd_settings, "ANGLE_POS_X", 50);
    ANGLE_POS_Y = load_double_setting(osd_settings, "ANGLE_POS_Y", 77);
    ANGLE_SCALE = load_double_setting(osd_settings, "ANGLE_SCALE", 1);


    ANGLE2 = load_bool_setting(osd_settings, "ANGLE2", false);
    ANGLE2_POS_X = load_double_setting(osd_settings, "ANGLE2_POS_X", 50);
    ANGLE2_POS_Y = load_double_setting(osd_settings, "ANGLE2_POS_Y", 78);
    ANGLE2_SCALE = load_double_setting(osd_settings, "ANGLE2_SCALE", 1);


    ALARM = load_bool_setting(osd_settings, "ALARM", false);
    ALARM_POS_X = load_double_setting(osd_settings, "ALARM_POS_X", 83);
    ALARM_POS_Y = load_double_setting(osd_settings, "ALARM_POS_Y", 33);
    ALARM_SCALE = load_double_setting(osd_settings, "ALARM_SCALE", 0.5);

    ALARM_1 = load_bool_setting(osd_settings, "ALARM_1", false);
    ALARM_2 = load_bool_setting(osd_settings, "ALARM_2", false);
    ALARM_3 = load_bool_setting(osd_settings, "ALARM_3", false);
    ALARM_4 = load_bool_setting(osd_settings, "ALARM_4", false);
    ALARM_5 = load_bool_setting(osd_settings, "ALARM_5", false);
    ALARM_6 = load_bool_setting(osd_settings, "ALARM_6", false);
    ALARM_7 = load_bool_setting(osd_settings, "ALARM_7", false);
    ALARM_8 = load_bool_setting(osd_settings, "ALARM_8", false);
    ALARM_9 = load_bool_setting(osd_settings, "ALARM_9", false);
    ALARM_10 = load_bool_setting(osd_settings, "ALARM_10", false);
    ALARM_11 = load_bool_setting(osd_settings, "ALARM_11", false);
    ALARM_12 = load_bool_setting(osd_settings, "ALARM_12", false);
    ALARM_13 = load_bool_setting(osd_settings, "ALARM_13", false);
    ALARM_14 = load_bool_setting(osd_settings, "ALARM_14", false);
    ALARM_15 = load_bool_setting(osd_settings, "ALARM_15", false);
    ALARM_16 = load_bool_setting(osd_settings, "ALARM_16", false);
    ALARM_17 = load_bool_setting(osd_settings, "ALARM_17", false);
    ALARM_18 = load_bool_setting(osd_settings, "ALARM_18", false);
    ALARM_19 = load_bool_setting(osd_settings, "ALARM_19", false);
    ALARM_20 = load_bool_setting(osd_settings, "ALARM_20", false);
    ALARM_21 = load_bool_setting(osd_settings, "ALARM_21", false);
    ALARM_22 = load_bool_setting(osd_settings, "ALARM_22", false);
    ALARM_23 = load_bool_setting(osd_settings, "ALARM_23", false);
    ALARM_24 = load_bool_setting(osd_settings, "ALARM_24", false);
    ALARM_25 = load_bool_setting(osd_settings, "ALARM_25", false);
    ALARM_26 = load_bool_setting(osd_settings, "ALARM_26", false);
}
