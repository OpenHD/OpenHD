//#pragma once

#include "bcm_host.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"
#include <math.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <utime.h>
#include <unistd.h>
#include <getopt.h>
#include <endian.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <fontinfo.h>
#include <time.h>
#include "telemetry.h"
#include <fontinfo.h>
#include <time.h>

#define TO_DEG 180.0f / M_PI

void render_init();

void setfillstroke();
void render(telemetry_data_t_osd *td, uint8_t cpuload_gnd, uint8_t temp_gnd, uint8_t undervolt, int fps);

/*
 * Rotate a polyline/polygon
 * 
 */
void rotatePoints(float *x, float *y, float angle, int points, int center_x, int center_y);

float distance_between(float lat1, float long1, float lat2, float long2);
float course_to (float lat1, float long1, float lat2, float long2);


void draw_total_signal(int8_t signal, int goodblocks, int badblocks, int packets_lost, int packets_received, int lost_per_block, float pos_x, float pos_y, float scale);
void draw_card_signal(int8_t signal, int signal_good, int card, int adapter_cnt, int restart_count, int packets, int wrongcrcs, int type, int totalpackets, int totalpacketslost, float pos_x, float pos_y, float scale);
void draw_uplink_signal(int8_t uplink_signal, int uplink_lostpackets, int8_t rc_signal, int rc_lostpackets, float pos_x, float pos_y, float scale);

void draw_kbitrate(int cts, int kbitrate, uint16_t kbitrate_tx, uint16_t current_air_datarate_kbit, uint16_t kbitrate_measured_tx, double hw_datarate_mbit, uint32_t fecs_skipped, uint32_t injection_failed, long long injection_time, int armed, float pos_x, float pos_y, float scale, float mbit_warn, float mbit_caution, float declutter);
void draw_sys(uint8_t cpuload_air, uint8_t temp_air, uint8_t cpuload_gnd, uint8_t temp_gnd, int armed, float pos_x, float pos_y, float scale, float load_warn, float load_caution, float temp_warn, float temp_caution, float declutter);
void draw_message(int severity, const char line1[30], const char line2[30], const char line3[30], float pos_x, float pos_y, float scale);

/* 
 * New stuff from fritz walter https://www.youtube.com/watch?v=EQ01b3aJ-rk
 * 
 * This will only indicate how much % are left. Mavlink specific, but could be used with others as well.
 * 
 */
void draw_batt_gauge(int remaining, float pos_x, float pos_y, float scale);
void draw_batt_status(float voltage, float current, float pos_x, float pos_y, float scale);



void draw_TOTAL_AMPS(float current, float pos_x, float pos_y, float scale);
void draw_TOTAL_DIST(double kmh, float pos_x, float pos_y, float scale);
void draw_TOTAL_TIME(float fly_time, float pos_x, float pos_y, float scale); 


void draw_position(float lat, float lon, float pos_x, float pos_y, float scale);


void draw_home_distance(int distance, bool home_fixed, float pos_x, float pos_y, float scale);
void draw_mavlink_mode(int mode, int armed, float pos_x, float pos_y, float scale);
void draw_vot_mode(int mode, int armed, float pos_x, float pos_y, float scale);
void draw_ltm_mode(int mode, int armed, int failsafe, float pos_x, float pos_y, float scale);
void draw_rssi(int rssi, int armed, float pos_x, float pos_y, float scale, float mbit_warn, float mbit_caution, float declutter);
void draw_cog(int cog, float pos_x, float pos_y, float scale);
void draw_climb(float climb, float pos_x, float pos_y, float scale);
void draw_mslalt(float mslalt, float pos_x, float pos_y, float scale);
void draw_airspeed(int airspeed, float pos_x, float pos_y, float scale);
void draw_gpsspeed(int gpsspeed, float pos_x, float pos_y, float scale);

void draw_compass(float heading, float home_heading, float pos_x, float pos_y, float scale);
void draw_alt_ladder(int alt, float pos_x, float pos_y, float scale, float warn, float caution, float vsi_time, float climb);
void draw_speed_ladder(int speed, float pos_x, float pos_y, float scale, float trend_time, float low_limit, float vx);
void draw_yaw_display(float vy, float pos_x, float pos_y, float scale, float trend_time);
void draw_ahi(float roll, float pitch, float scale);
void draw_ahi_mav(float roll, float pitch, float mav_climb, float vz, float vx, float vy, float gpsspeed, float gpsalt, float scale);
void draw_home_arrow(float abs_heading, float craft_heading, float pos_x, float pos_y, float scale);

void draw_osdinfos(int osdfos, float pos_x, float pos_y, float scale);

void draw_home_radar(float abs_heading, float craft_heading, int homedst, float pos_x, float pos_y, float scale);
void draw_osdinfos(int osdfos, float pos_x, float pos_y, float scale);
void draw_RPA(float roll, float pitch, float pos_x, float pos_y, float scale);
void draw_throttle(uint16_t throttle, uint16_t throttle_target, int armed, float pos_x, float pos_y, float scale);
void draw_throttle_V2(uint16_t throttle, float pos_x, float pos_y, float scale);
void draw_ltmmode(int mode, int armed, int failsafe, float pos_x, float pos_y, float scale);
void draw_amp_status(float current, float pos_x, float pos_y, float scale);
void draw_osdinfos(int osdfps, float pos_x, float pos_y, float scale);
void draw_sat(int sats, int fixtype, int hdop, int armed, float pos_x, float pos_y, float scale, float hdop_warn, float hdop_caution, float declutter);


//void draw_Hdop(float hdop, float pos_x, float pos_y, float scale);
void draw_Mission(int Seq,float pos_x, float pos_y, float scale);
void draw_Angle(float pos_x, float pos_y, float scale);
void draw_Angle2(float pos_x, float pos_y, float scale);
void draw_Alarm(int SenorsPresent, int SenorsEnabled, int SenorsHealth, float pos_x, float pos_y, float scale);

extern int width;
extern int height;
