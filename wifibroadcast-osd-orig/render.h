#pragma once

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

#define TO_DEG 180.0f / M_PI

void render_init();
void setfillstroke();
void render(telemetry_data_t *td, uint8_t cpuload_gnd, uint8_t temp_gnd, uint8_t undervolt, int fps);

void rotatePoints(float *x, float *y, float angle, int points, int center_x, int center_y); //rotate a polyline/polygon
float distance_between(float lat1, float long1, float lat2, float long2);
float course_to (float lat1, float long1, float lat2, float long2);

void draw_total_signal(int8_t signal, int goodblocks, int badblocks, int packets_lost, int packets_received, int lost_per_block, float pos_x, float pos_y, float scale);
void draw_card_signal(int8_t signal, int signal_good, int card, int adapter_cnt, int restart_count, int packets, int wrongcrcs, int type, int totalpackets, int totalpacketslost, float pos_x, float pos_y, float scale);
void draw_uplink_signal(int8_t uplink_signal, int uplink_lostpackets, int8_t rc_signal, int rc_lostpackets, float pos_x, float pos_y, float scale);

void draw_kbitrate(int cts, int kbitrate, uint16_t kbitrate_measured_tx, uint16_t kbitrate_tx, uint32_t fecs_skipped, uint32_t injection_failed, long long injection_time, float pos_x, float pos_y, float scale);
void draw_sys(uint8_t cpuload_air, uint8_t temp_air, uint8_t cpuload_gnd, uint8_t temp_gnd, float pos_x, float pos_y, float scale);
void draw_message(int severity, char line1[30], char line2[30], char line3[30], float pos_x, float pos_y, float scale);

//new stuff from fritz walter https://www.youtube.com/watch?v=EQ01b3aJ-rk
//this will only indicate how much % are left. Mavlink specific, but could be used with others as well.
void draw_batt_gauge(int remaining, float pos_x, float pos_y, float scale);
void draw_batt_status(float voltage, float current, float pos_x, float pos_y, float scale);
void draw_position(float lat, float lon, float pos_x, float pos_y, float scale);
void draw_sat(int sats, int fixtype, float pos_x, float pos_y, float scale);
void draw_home_distance(int distance, bool home_fixed, float pos_x, float pos_y, float scale);
void draw_mode(int mode, int armed, float pos_x, float pos_y, float scale);
void draw_rssi(int rssi, float pos_x, float pos_y, float scale);
void draw_cog(int cog, float pos_x, float pos_y, float scale);
void draw_climb(float climb, float pos_x, float pos_y, float scale);
void draw_baroalt(float baroalt, float pos_x, float pos_y, float scale);
void draw_gpsalt(float gpsalt, float pos_x, float pos_y, float scale);
void draw_airspeed(int airspeed, float pos_x, float pos_y, float scale);
void draw_gpsspeed(int gpsspeed, float pos_x, float pos_y, float scale);
void draw_compass(float heading, float home_heading, float pos_x, float pos_y, float scale);
void draw_alt_ladder(int alt, float pos_x, float pos_y, float scale);
void draw_speed_ladder(int speed, float pos_x, float pos_y, float scale);
void draw_ahi(float roll, float pitch, float scale);
void draw_home_arrow(float abs_heading, float craft_heading, float pos_x, float pos_y, float scale);

void draw_osdinfos(int osdfos, float pos_x, float pos_y, float scale);

int width,height;
