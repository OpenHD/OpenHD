// characters osdicons.ttf
// round  triangle   
// satellite    
// cpu  
// bars ▁▂▃▄▅▆▇█
// arrows ↥ ↦ ↤    
// warning              
// down/up stream  
// RSSI     
// cam  
// double caret    
// please wait  
// wind      
// thermometer  
// time   
// pressure  
// speed  
// windhose    
// cog    

#include <stdint.h>
#include <stdio.h>
#include "render.h"
#include "telemetry.h"
#include "flightmode.h"

#include "settings.h"

#define TO_FEET 3.28084
#define TO_MPH 0.621371
#define CELL_WARNING_PCT1 (CELL_WARNING1-CELL_MIN)/(CELL_MAX-CELL_MIN)*100
#define CELL_WARNING_PCT2 (CELL_WARNING2-CELL_MIN)/(CELL_MAX-CELL_MIN)*100

int width;
int height;

long long amps_ts; 
long long dist_ts; 
long long time_ts;
float total_amps; 
double total_dist; 
float total_time;

float scale_factor_font;
bool setting_home;
bool home_set;
float home_lat;
float home_lon;
int home_counter;

#define TEXT_BUFFER_SIZE 40
char buffer[TEXT_BUFFER_SIZE];
Fontinfo myfont, osdicons;

int packetslost_last[6];
int lpb_array[20]; //lost per block
int lpb_counter=1;

int fecs_skipped_last;
int injection_failed_last;
int tx_restart_count_last;

int last_speed;

bool no_signal = false;
FILE *fptr;


long long current_ts() {
    struct timeval te;

    gettimeofday(&te, NULL);

    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    return milliseconds;
}


int getWidth(float pos_x_percent) {
    return (width * 0.01f * pos_x_percent);
}


int getHeight(float pos_y_percent) {
    return (height * 0.01f * pos_y_percent);
}


float getOpacity(int r, int g, int b, float o) {
    if (o<0.5) {
        o = o * 2;
    }
    
    return o;
}


void setfillstroke() {
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    StrokeWidth(OUTLINEWIDTH);
}


void render_init() {
    char filename[100] = "/boot/osdfonts/";

    InitShapes(&width, &height);

    strcat(filename, FONT.c_str());

    myfont = LoadTTFFile(filename);
    if (!myfont) {
        fputs("ERROR: Failed to load font!", stderr);
        myfont = LoadTTFFile("/boot/osdfonts/Archivo-Bold.ttf");
    }

    osdicons = LoadTTFFile("/boot/osdfonts/osdicons.ttf");
    if (!osdicons) {
        fputs("ERROR: Failed to load osdicons.ttf font!", stderr);
        exit(1);
    }
   
    home_counter = 0;
    //vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);

    //wowi
    amps_ts = dist_ts = time_ts = current_ts();
}



void loopUpdate(telemetry_data_t_osd *td) {
    /*
     * Update fly time. Get time passed since last update
     */
    long time_diff = current_ts() - time_ts;

    time_ts = current_ts();

    if (COPTER == true) {
        if ((td->armed == 1) && (td->ampere > 3)) {
            total_time += (float)time_diff / 60000;
        }
    } else {
        if (td->speed>0) {
            total_time += (float)time_diff / 60000;
        }
    }


    /*
     * Update total amps used. 
     * 
     * Get time passed since last rendering
     */
    time_diff = current_ts() - amps_ts;
    
    amps_ts = current_ts();

    total_amps = total_amps + td->ampere * (float)time_diff / 3600;
}



void render(telemetry_data_t_osd *td, uint8_t cpuload_gnd, uint8_t temp_gnd, uint8_t undervolt, int osdfps) {

    /* 
     * Call loopUpdate to update stuff that should be updated even when particular elements are off (like total curent);
     */
    loopUpdate(td);

    /*
     * Render start
     */
    Start(width,height);

    setfillstroke();

    if (td->rx_status_sysair->undervolt == 1) {
        draw_message(0, "Undervoltage on TX", "Check wiring/power-supply", "Bitrate limited to 1 Mbit", WARNING_POS_X, WARNING_POS_Y, GLOBAL_SCALE);
    }
    
    if (undervolt == 1) {
        draw_message(0, "Undervoltage on RX", "Check wiring/power-supply", " ", WARNING_POS_X, WARNING_POS_Y, GLOBAL_SCALE);
    }


    if (FRSKY) {
        /* 
        * We assume that we have a fix if we get the NS and EW values from FrSky protocol
        */

        if (((td->ew == 'E' || td->ew == 'W') && (td->ns == 'N' || td->ns == 'S')) && !home_set) {
            setting_home = true;
        } else { 
            setting_home = false;
            home_counter = 0;
        }

        if (setting_home && !home_set) {
            /*
            * If 20 packets after each other have a fix, set home
            */
            if (++home_counter == 20) {
                home_set = true;
                home_lat = (td->ns == 'N'? 1:-1) * td->latitude;
                home_lon = (td->ew == 'E'? 1:-1) * td->longitude;
            }
        }
    }

    if (MAVLINK || SMARTPORT) {
        /*
        * If atleast 2D satfix is reported by flightcontrol
        */
        if (td->fix > 2 && !home_set) {
            setting_home = true;
        } else {
            setting_home = false;
            home_counter = 0;
        }

        if (setting_home && !home_set){
            /* 
            * If 20 packets after each other have a fix set home
            */
            if (++home_counter == 20) {
                home_set = true;
                home_lat = td->latitude;
                home_lon = td->longitude;
            
                /* 
                * Save GPS home
                */
                float lonlat[2];
                lonlat[0] = 0.0;
                lonlat[1] = 0.0;

                /*
                * Read saved home position from shm, used in case the OSD is restarted by something
                *
                */
                fptr = fopen("/dev/shm/homepos", "rb");

                if (fptr == NULL) {
                    printf("No GPS home file found. Load home from flight controller\n");

                    home_lat = td->latitude;
                    home_lon = td->longitude;

                    lonlat[0] = home_lon;
                    lonlat[1] = home_lat;
    
                    fptr = fopen("/dev/shm/homepos", "wb");

                    if (fptr == NULL) {
                        printf("Cannot create a file to store GPS home position \n" );

                        return;
                    } else {
                        printf("Saving GPS home position... \n");

                        fwrite(&lonlat, sizeof(lonlat), 1, fptr);

                        fclose(fptr);
                    }
                } else {
                    printf("GPS home file exist. Load Home position from it \n");
                    fread(&lonlat, sizeof(lonlat), 1, fptr);
                    fclose(fptr);

                    home_lat = lonlat[1];
                    home_lon = lonlat[0];

                    printf("Lat:%f \n",  home_lat);
                    printf("Lon:%f \n",  home_lon);
                }
            }
        }
    }

    if (LTM) {
        /*
        * LTM makes it easy, if O-frame reports home fix, set home position and use home 
        * lat/long from LTM O-frame
        */
        if (td->home_fix == 1) {
            home_set = true;
            home_lat = td->ltm_home_latitude;
            home_lon = td->ltm_home_longitude;
        }
    }


    if (UPLINK_RSSI) {
        draw_uplink_signal(td->rx_status_uplink->adapter[0].current_signal_dbm, td->rx_status_uplink->lost_packet_cnt, td->rx_status_rc->adapter[0].current_signal_dbm, td->rx_status_rc->lost_packet_cnt, UPLINK_RSSI_POS_X, UPLINK_RSSI_POS_Y, UPLINK_RSSI_SCALE * GLOBAL_SCALE);
    }


    if (KBITRATE) {
        draw_kbitrate(td->rx_status_sysair->cts, td->rx_status->kbitrate, td->rx_status_sysair->bitrate_kbit, td->rx_status->current_air_datarate_kbit, td->rx_status_sysair->bitrate_measured_kbit, td->datarate, td->rx_status_sysair->skipped_fec_cnt, td->rx_status_sysair->injection_fail_cnt,td->rx_status_sysair->injection_time_block, td->armed, KBITRATE_POS_X, KBITRATE_POS_Y, KBITRATE_SCALE * GLOBAL_SCALE, KBITRATE_WARN, KBITRATE_CAUTION, KBITRATE_DECLUTTER);
    }


    if (SYS) {
        draw_sys(td->rx_status_sysair->cpuload, td->rx_status_sysair->temp, cpuload_gnd, temp_gnd, td->armed, SYS_POS_X, SYS_POS_Y, SYS_SCALE * GLOBAL_SCALE, CPU_LOAD_WARN, CPU_LOAD_CAUTION, CPU_TEMP_WARN, CPU_TEMP_CAUTION, SYS_DECLUTTER);
    }



    if (FLIGHTMODE) {
        if (MAVLINK) {
            draw_mavlink_mode(td->mav_flightmode, td->armed, FLIGHTMODE_POS_X, FLIGHTMODE_POS_Y, FLIGHTMODE_SCALE * GLOBAL_SCALE);
        }

        if (VOT) {
            draw_vot_mode(td->flightmode, td->armed, FLIGHTMODE_POS_X, FLIGHTMODE_POS_Y, FLIGHTMODE_SCALE * GLOBAL_SCALE);
        }

        if (LTM) {
            draw_ltm_mode(td->ltm_flightmode, td->armed, td->ltm_failsafe, FLIGHTMODE_POS_X, FLIGHTMODE_POS_Y, FLIGHTMODE_SCALE * GLOBAL_SCALE);
        }
    }



    if (RSSI) {
        draw_rssi(td->rssi, td->armed, RSSI_POS_X, RSSI_POS_Y, RSSI_SCALE * GLOBAL_SCALE, RSSI_WARN, RSSI_CAUTION, RSSI_DECLUTTER);
    }


    if (CLIMB && MAVLINK) {
        draw_climb(td->mav_climb, CLIMB_POS_X, CLIMB_POS_Y, CLIMB_SCALE * GLOBAL_SCALE);
    }


    if (AIRSPEED) {
        draw_airspeed((int)td->airspeed, AIRSPEED_POS_X, AIRSPEED_POS_Y, AIRSPEED_SCALE * GLOBAL_SCALE);
    }


    if (GPSSPEED) {
        draw_gpsspeed((int)td->speed, GPSSPEED_POS_X, GPSSPEED_POS_Y, GPSSPEED_SCALE * GLOBAL_SCALE);
    }


    if (COURSE_OVER_GROUND) {
        draw_cog((int)td->cog, COURSE_OVER_GROUND_POS_X, COURSE_OVER_GROUND_POS_Y, COURSE_OVER_GROUND_SCALE * GLOBAL_SCALE);
    }



    if (ALTLADDER) {
        /*
        * By default in osdconfig uses mslalt = false (relative alt should be shown)
        *
        */
        if (REVERSE_ALTITUDES) {
            draw_alt_ladder((int)td->msl_altitude, ALTLADDER_POS_X, 50, ALTLADDER_SCALE * GLOBAL_SCALE, ALTLADDER_WARN, ALTLADDER_CAUTION, ALTLADDER_VSI_TIME, td->mav_climb);
        } else {
            draw_alt_ladder((int)td->rel_altitude, ALTLADDER_POS_X, 50, ALTLADDER_SCALE * GLOBAL_SCALE, ALTLADDER_WARN, ALTLADDER_CAUTION, ALTLADDER_VSI_TIME, td->mav_climb);
        }
    }



    if (MSLALT) { 
        if (REVERSE_ALTITUDES) {
            draw_mslalt(td->rel_altitude, MSLALT_POS_X, MSLALT_POS_Y, MSLALT_SCALE * GLOBAL_SCALE);
        } else {
            draw_mslalt(td->msl_altitude, MSLALT_POS_X, MSLALT_POS_Y, MSLALT_SCALE * GLOBAL_SCALE);
        }
    }



    if (SPEEDLADDER) {
        if (IMPERIAL) {
            if (SPEEDLADDER_USEAIRSPEED) {
                draw_speed_ladder((int)td->airspeed * TO_MPH, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE, SPEEDLADDER_TREND_TIME, SPEEDLADDER_LOW_LIMIT, td->vx);
            } else {
                draw_speed_ladder((int)td->speed * TO_MPH, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE, SPEEDLADDER_TREND_TIME, SPEEDLADDER_LOW_LIMIT, td->vx);
            }

        } else {
            if (SPEEDLADDER_USEAIRSPEED) {
                draw_speed_ladder((int)td->airspeed, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE, SPEEDLADDER_TREND_TIME, SPEEDLADDER_LOW_LIMIT, td->vx);
            } else {
                draw_speed_ladder((int)td->speed, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE, SPEEDLADDER_TREND_TIME, SPEEDLADDER_LOW_LIMIT, td->vx);
            }
        }
    }



    if (YAWDISPLAY && MAVLINK) {
        draw_yaw_display(td->vy, YAWDISPLAY_POS_X, YAWDISPLAY_POS_Y, YAWDISPLAY_SCALE * GLOBAL_SCALE, YAWDISPLAY_TREND_TIME);
    }



    if (HOME_ARROW) {
        if (FRSKY) {
            //draw_home_arrow((int)course_to((td->ns == 'N' ? 1 : -1) * td->latitude, (td->ns == 'E' ? 1 : -1) * td->longitude, home_lat, home_lon), HOME_ARROW_POS_X, HOME_ARROW_POS_Y, HOME_ARROW_SCALE * GLOBAL_SCALE);
        } else {
            if (HOME_ARROW_USECOG) {
                draw_home_arrow(course_to(home_lat, home_lon, td->latitude, td->longitude), td->cog, HOME_ARROW_POS_X, HOME_ARROW_POS_Y, HOME_ARROW_SCALE * GLOBAL_SCALE);
            } else {
                draw_home_arrow(course_to(home_lat, home_lon, td->latitude, td->longitude), td->heading, HOME_ARROW_POS_X, HOME_ARROW_POS_Y, HOME_ARROW_SCALE * GLOBAL_SCALE);
            }

            if (td->heading>=360) { 
                td->heading=td->heading-360;
            }
        }
    }



    if (COMPASS) {
        if (COMPASS_USECOG) {
            draw_compass(td->cog, course_to(home_lat, home_lon, td->latitude, td->longitude), 50, COMPASS_POS_Y, COMPASS_SCALE * GLOBAL_SCALE);
        } else {
            draw_compass(td->heading, course_to(home_lat, home_lon, td->latitude, td->longitude), 50, COMPASS_POS_Y, COMPASS_SCALE * GLOBAL_SCALE);
        }
    }



    if (BATT_STATUS) {
        draw_batt_status(td->voltage, td->ampere, BATT_STATUS_POS_X, BATT_STATUS_POS_Y, BATT_STATUS_SCALE * GLOBAL_SCALE);
    }



    if (TOTAL_AMPS) { 
        draw_TOTAL_AMPS(total_amps, TOTAL_AMPS_POS_X, TOTAL_AMPS_POS_Y, TOTAL_AMPS_SCALE * GLOBAL_SCALE);
    }



    if (TOTAL_DIST) {
        draw_TOTAL_DIST((int)td->speed, TOTAL_DIST_POS_X, TOTAL_DIST_POS_Y, TOTAL_DIST_SCALE * GLOBAL_SCALE);
    }



    if (TOTAL_TIME) {
        draw_TOTAL_TIME((float)total_time, TOTAL_TIME_POS_X, TOTAL_TIME_POS_Y, TOTAL_TIME_SCALE * GLOBAL_SCALE);
    }



    if (POSITION) {
        if (FRSKY) {
            draw_position((td->ns == 'N' ? 1 : -1) * td->latitude, (td->ew == 'E' ? 1 : -1) * td->longitude, POSITION_POS_X, POSITION_POS_Y, POSITION_SCALE * GLOBAL_SCALE);
        }

        draw_position(td->latitude, td->longitude, POSITION_POS_X, POSITION_POS_Y, POSITION_SCALE * GLOBAL_SCALE);
    }



    if (DISTANCE) {
        if (FRSKY) {
            draw_home_distance((int)distance_between(home_lat, home_lon, (td->ns == 'N' ? 1 : -1) * td->latitude, (td->ns == 'E' ? 1 : -1) * td->longitude), home_set, DISTANCE_POS_X, DISTANCE_POS_Y, DISTANCE_SCALE * GLOBAL_SCALE);
        } else if (LTM || MAVLINK || SMARTPORT) {
            draw_home_distance((int)distance_between(home_lat, home_lon, td->latitude, td->longitude), home_set, DISTANCE_POS_X, DISTANCE_POS_Y, DISTANCE_SCALE * GLOBAL_SCALE);
        } else if (VOT) {
            draw_home_distance((int)td->distance, home_set, DISTANCE_POS_X, DISTANCE_POS_Y, DISTANCE_SCALE * GLOBAL_SCALE);
        }
    }


    if (DOWNLINK_RSSI) {
        int i;
        
        int best_dbm = -1000;

        int ac = td->rx_status->wifi_adapter_cnt;

        no_signal = true;


        for (i = 0; i < ac; ++i) { 
            /*
            * Find out which card has best signal (and if at least one card has a signal)
            */

            if (td->rx_status->adapter[i].signal_good == 1) {
                if (best_dbm < td->rx_status->adapter[i].current_signal_dbm) {
                    best_dbm = td->rx_status->adapter[i].current_signal_dbm;
                }
            }

            if (td->rx_status->adapter[i].signal_good == 1) {
                no_signal = false;
            }
        }

        draw_total_signal(best_dbm, td->rx_status->received_block_cnt, td->rx_status->damaged_block_cnt, td->rx_status->lost_packet_cnt, td->rx_status->received_packet_cnt, td->rx_status->lost_per_block_cnt, DOWNLINK_RSSI_POS_X, DOWNLINK_RSSI_POS_Y, DOWNLINK_RSSI_SCALE * GLOBAL_SCALE);

        if (DOWNLINK_RSSI_DETAILED) {
            for (i = 0; i < ac; ++i) {
                draw_card_signal(td->rx_status->adapter[i].current_signal_dbm, td->rx_status->adapter[i].signal_good, i, ac, td->rx_status->tx_restart_cnt, td->rx_status->adapter[i].received_packet_cnt, td->rx_status->adapter[i].wrong_crc_cnt, td->rx_status->adapter[i].type, td->rx_status->received_packet_cnt, td->rx_status->lost_packet_cnt, DOWNLINK_RSSI_DETAILED_POS_X, DOWNLINK_RSSI_DETAILED_POS_Y, DOWNLINK_RSSI_DETAILED_SCALE * GLOBAL_SCALE);
            }
        }
    }



    if (SAT) {
        if (FRSKY) {
            /*
            * We assume that we have a fix if we get the NS and EW values from frsky protocol
            *
            */

            if ((td->ew == 'E' || td->ew == 'W') && (td->ns == 'N' || td->ns == 'S')) {
                draw_sat(0, 2, td->hdop, (int)td->armed, SAT_POS_X, SAT_POS_Y, SAT_SCALE * GLOBAL_SCALE, SAT_HDOP_WARN, SAT_HDOP_CAUTION, SAT_DECLUTTER);
            } else {
                draw_sat(0, 0, td->hdop, (int)td->armed, SAT_POS_X, SAT_POS_Y, SAT_SCALE * GLOBAL_SCALE, SAT_HDOP_WARN, SAT_HDOP_CAUTION, SAT_DECLUTTER);
            }
        } else if (MAVLINK || SMARTPORT || LTM || VOT) {
                draw_sat(td->sats, td->fix,  td->hdop, (int)td->armed, SAT_POS_X, SAT_POS_Y, SAT_SCALE * GLOBAL_SCALE, SAT_HDOP_WARN, SAT_HDOP_CAUTION, SAT_DECLUTTER);
        }
    }



    if (BATT_GAUGE) {
        draw_batt_gauge(((td->voltage / CELLS) - CELL_MIN) / (CELL_MAX - CELL_MIN) * 100, BATT_GAUGE_POS_X, BATT_GAUGE_POS_Y, BATT_GAUGE_SCALE * GLOBAL_SCALE);
    }



    if (HOME_RADAR) { 
        if (HOME_RADAR_USECOG) {
            draw_home_radar(course_to(home_lat, home_lon, td->latitude, td->longitude), td->cog, (int)distance_between(home_lat, home_lon, td->latitude, td->longitude), HOME_RADAR_POS_X, HOME_RADAR_POS_Y, HOME_RADAR_SCALE * GLOBAL_SCALE);
        } else {
            draw_home_radar(course_to(home_lat, home_lon, td->latitude, td->longitude), td->heading, (int)distance_between(home_lat, home_lon, td->latitude, td->longitude), HOME_RADAR_POS_X, HOME_RADAR_POS_Y, HOME_RADAR_SCALE * GLOBAL_SCALE);  
        }
    }



    if (HDOP && MAVLINK) {
        //draw_Hdop(td->hdop, HDOP_POS_X, HDOP_POS_Y, HDOP_SCALE * GLOBAL_SCALE);
    }



    if (THROTTLE_V2 && MAVLINK) { 
        draw_throttle_V2(td->throttle, THROTTLE_V2_POS_X, THROTTLE_V2_POS_Y, THROTTLE_V2_SCALE * GLOBAL_SCALE);
    }



    if (THROTTLE && MAVLINK) {
        draw_throttle((int)td->throttle, THROTTLE_TARGET, (int)td->armed, THROTTLE_POS_X, THROTTLE_POS_Y, THROTTLE_SCALE * GLOBAL_SCALE);
    }



    if (MISSION && MAVLINK) {
        draw_Mission(td->mission_current_seq, MISSION_POS_X, MISSION_POS_Y, MISSION_SCALE * GLOBAL_SCALE);
    }



    if (ANGLE2) {
        draw_Angle2(ANGLE2_POS_X, ANGLE2_POS_Y, ANGLE2_SCALE * GLOBAL_SCALE);
    }



    if (ALARM && MAVLINK) {
        draw_Alarm(td->SP, td->SE, td->SH, ALARM_POS_X, ALARM_POS_Y, ALARM_SCALE * GLOBAL_SCALE);
    }



    if (RPA) {
        /*
        * Roll and pitch angle
        */
        draw_RPA(RPA_INVERT_ROLL * td->roll, RPA_INVERT_PITCH * td->pitch, RPA_POS_X, RPA_POS_Y, RPA_SCALE * GLOBAL_SCALE);
    }




    if (AHI) {
        if (FRSKY || SMARTPORT) {
            float x_val, y_val, z_val;
            x_val = td->x;
            y_val = td->y;
            z_val = td->z;

            if (AHI_SWAP_ROLL_AND_PITCH) {
                draw_ahi(AHI_INVERT_ROLL * TO_DEG * (atan(y_val / sqrt((x_val * x_val) + (z_val * z_val)))), 
                AHI_INVERT_PITCH * TO_DEG * (atan(x_val / sqrt((y_val * y_val) + (z_val * z_val)))), AHI_SCALE * GLOBAL_SCALE);
            } else {
                draw_ahi(AHI_INVERT_ROLL * TO_DEG * (atan(x_val / sqrt((y_val * y_val) + (z_val * z_val)))), 
                AHI_INVERT_PITCH * TO_DEG * (atan(y_val / sqrt((x_val * x_val) + (z_val * z_val)))), AHI_SCALE * GLOBAL_SCALE);
            }
        } else if (LTM || VOT) {
            draw_ahi(AHI_INVERT_ROLL * td->roll, AHI_INVERT_PITCH * td->pitch, AHI_SCALE * GLOBAL_SCALE);
         } else if (MAVLINK) {	
            if (REVERSE_ALTITUDES) {
                draw_ahi_mav(AHI_INVERT_ROLL * td->roll, AHI_INVERT_PITCH * td->pitch, td->mav_climb, td->vz, td->vx, td->vy, (int)td->speed, (int)td->msl_altitude, AHI_SCALE * GLOBAL_SCALE);
            } else {
                draw_ahi_mav(AHI_INVERT_ROLL * td->roll, AHI_INVERT_PITCH * td->pitch, td->mav_climb, td->vz, td->vx, td->vy, (int)td->speed, (int)td->rel_altitude, AHI_SCALE * GLOBAL_SCALE);
            }
         }
    }




    if (ANGLE) { 
        /* 
         * Bank angle indicator. 
         * 
         * Must follow AHI 
         */
        draw_Angle(ANGLE_POS_X, ANGLE_POS_Y, ANGLE_SCALE * GLOBAL_SCALE);
    }


    /*
     * Render end (causes everything to be drawn on next vsync)
     */
    End();
}



void draw_vot_mode(int mode, int armed, float pos_x, float pos_y, float scale) {
    /* 
     * Flight modes Eagletree Vector
     */
    float text_scale = getWidth(2) * scale;

    const char *fmode = vot_mode_from_telemetry(mode);

    if (armed == 1) {
        sprintf(buffer, "[%s]", fmode);
    } else {
        sprintf(buffer, "%s", fmode);
    }

    TextMid(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}


void draw_ltm_mode(int mode, int armed, int failsafe, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    const char *fmode;

    if (CHINESE) {
        fmode = chinese_ltm_mode_from_telem((COPTER_MODE)mode);
    } else {
        fmode = ltm_mode_from_telem((COPTER_MODE)mode);
    }


    if (failsafe == 1) {
        sprintf(buffer, "失 控 保 护");
    } else {
        if (armed == 1) {
            sprintf(buffer, "[%s]", fmode);
        } else {
            sprintf(buffer, "%s", fmode);
        }
    }

    TextMid(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}


void draw_mavlink_mode(int mode, int armed, float pos_x, float pos_y, float scale) {
    /*
     * Autopilot mode, mavlink specific, could be used if mode is in telemetry data of other protocols as well
     */
    float text_scale = getWidth(2) * scale;

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    const char *fmode;

    if (COPTER) {
        if (CHINESE) {
            fmode = chinese_copter_mode_from_enum((COPTER_MODE)mode);
        } else {
            fmode = copter_mode_from_enum((COPTER_MODE)mode);
        }
    } else {
        if (CHINESE) {
            fmode = chinese_plane_mode_from_enum((PLANE_MODE)mode);
        } else {
            fmode = plane_mode_from_enum((PLANE_MODE)mode);
        }
    }

    if (armed == 1) {
        sprintf(buffer, "[%s]", fmode);
    } else {
        sprintf(buffer, "%s", fmode);
    }

    TextMid(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}


void draw_rssi(int rssi, int armed, float pos_x, float pos_y, float scale, float warn, float caution, float declutter) {
    float text_scale = getWidth(2) * scale;

    VGfloat width_value = TextWidth("00", myfont, text_scale);   

    if (rssi < warn) {
        // red

        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A); 
    } else if (rssi < caution) {
        // yellow

        Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
        Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A); 
    } else {    
         //normal

        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        if ((armed == 1) && (declutter == 1)) {
             //opaque

            Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
            Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
        }
    } 

    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "", osdicons, text_scale * 0.6);
    sprintf(buffer, "%02d", rssi);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    Text(getWidth(pos_x), getHeight(pos_y), "%", myfont, text_scale * 0.6);
}



void draw_cog(int cog, float pos_x, float pos_y, float scale){

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("000°", myfont, text_scale);

    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "", osdicons, text_scale * 0.7);

    sprintf(buffer, "%d°", cog);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}



void draw_climb(float climb, float pos_x, float pos_y, float scale){

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("-00.0", myfont, text_scale);

    if (CHINESE) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "", osdicons, text_scale * 0.6);
    } else {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "ﵻ", osdicons, text_scale * 0.6);
    }

    if (climb > 0.0f) {
        sprintf(buffer, "+%.1f", climb);
    } else {
        sprintf(buffer, "%.1f", climb);
    }
    
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "m/s", myfont, text_scale * 0.6);
}


void draw_airspeed(int airspeed, float pos_x, float pos_y, float scale){

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("100", myfont, text_scale);

    if (CHINESE) {
        VGfloat width_speedo = TextWidth("", osdicons, text_scale * 0.65) + getWidth(0.5) * scale;

        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "", osdicons, text_scale * 0.65);
        TextEnd(getWidth(pos_x) - width_value-width_speedo, getHeight(pos_y), "", osdicons, text_scale * 0.65);
    } else {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "ﵷ", osdicons, text_scale * 0.65);
    }

    if (IMPERIAL) {
        sprintf(buffer, "%d", airspeed * TO_MPH);
    } else {
        sprintf(buffer, "%d", airspeed);
    }
    
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    if (IMPERIAL) {
        Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "mph", myfont, text_scale * 0.6);
    } else {
        Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "km/h", myfont, text_scale * 0.6);
    }
}



void draw_gpsspeed(int gpsspeed, float pos_x, float pos_y, float scale){
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("100", myfont, text_scale);

    if (CHINESE) {
        VGfloat width_speedo = TextWidth("", osdicons, text_scale * 0.65);
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "", osdicons, text_scale * 0.65);
        TextEnd(getWidth(pos_x) - width_value-width_speedo, getHeight(pos_y), "", osdicons, text_scale * 0.7);
    } else {
        VGfloat width_speedo = TextWidth("ﵵ", osdicons, text_scale * 0.65);
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "ﵷ", osdicons, text_scale * 0.7);
        TextEnd(getWidth(pos_x) - width_value - width_speedo, getHeight(pos_y), "", osdicons, text_scale * 0.65);
    }

    if (IMPERIAL) {
        sprintf(buffer, "%d", gpsspeed*TO_MPH);
    } else {
        sprintf(buffer, "%d", gpsspeed);
    }
    
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    if (IMPERIAL) {
        Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "mph", myfont, text_scale * 0.6);
    } else {
        Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "km/h", myfont, text_scale * 0.6);
    }
}


void draw_uplink_signal(int8_t uplink_signal, int uplink_lostpackets, int8_t rc_signal, int rc_lostpackets, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;
    
    VGfloat height_text = TextHeight(myfont, text_scale * 0.6) + getHeight(0.3) * scale;
    VGfloat width_value = TextWidth("-00", myfont, text_scale);
    VGfloat width_symbol = TextWidth(" ", osdicons, text_scale * 0.7);

    StrokeWidth(OUTLINEWIDTH);

    if ((uplink_signal < -125) && (rc_signal < -125)) {
        /*
         * Both no signal, display red dashes
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);

        sprintf(buffer, "-- ");
    } else if (rc_signal < -125) {
        /*
         * Only r/c no signal, so display uplink signal
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        sprintf(buffer, "%02d", uplink_signal);
    } else {
        /*
         * If both have signal, display r/c signal
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        sprintf(buffer, "%02d", rc_signal);
    }

    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    Text(getWidth(pos_x), getHeight(pos_y), "dBm", myfont, text_scale * 0.6);

    sprintf(buffer, "%d/%d", rc_lostpackets, uplink_lostpackets);
    Text(getWidth(pos_x) - width_value - width_symbol, getHeight(pos_y) - height_text, buffer, myfont, text_scale * 0.6);

    TextEnd(getWidth(pos_x) - width_value - getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.7);
}



void draw_kbitrate(int cts, int kbitrate, uint16_t kbitrate_tx, uint16_t current_air_datarate_kbit, uint16_t kbitrate_measured_tx, double hw_datarate_mbit, uint32_t fecs_skipped, uint32_t injection_failed, long long injection_time,int armed, float pos_x, float pos_y, float scale, float mbit_warn, float mbit_caution, float declutter) {
    
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    float text_scale = getWidth(2) * scale;
    VGfloat height_text_small = TextHeight(myfont, text_scale * 0.6) + getHeight(0.3) * scale;
    VGfloat width_value = TextWidth("10.0", myfont, text_scale);


    VGfloat width_symbol;
    if (CHINESE) {
        width_symbol = TextWidth("", osdicons, text_scale * 0.8);
    } else {
        width_symbol = TextWidth("ﵴ", osdicons, text_scale * 0.8);
    }

    float mbit = (float)kbitrate / 1000;
    float mbit_measured = (float)kbitrate_measured_tx / 1000;
    float mbit_tx = (float)kbitrate_tx / 1000;
    float ms = (float)injection_time / 1000;
    float air_rx_mbit = (float)current_air_datarate_kbit / 1000;
    

    if (air_rx_mbit / hw_datarate_mbit >= 0.75) {
        /*
         * Red text on radio line if FEC data rate > 75% of hardware max rate
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
    } else if (mbit_measured != 0 && air_rx_mbit > mbit_measured) {
        /* 
         * Yellow text on radio line if FEC data rate > measured available bandwidth, but 
         * only if measured bandwidth is not zero (zero means the user set a fixed video 
         * bitrate, measurement is not currently done in that case).
         */
        Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
        Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
    } else {
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    }

    if (cts == 0) {
        if (mbit_measured == 0) {
            sprintf(buffer, "(%.1f/-/%.1f)", air_rx_mbit, hw_datarate_mbit);
        } else {
            sprintf(buffer, "(%.1f/%.1f/%.1f)", air_rx_mbit, mbit_measured, hw_datarate_mbit);
        }
    } else {
        if (mbit_measured == 0) {
            sprintf(buffer, "(%.1f/-/%.1f) CTS", air_rx_mbit, hw_datarate_mbit);
        } else {
            sprintf(buffer, "(%.1f/%.1f/%.1f) CTS", air_rx_mbit, mbit_measured, hw_datarate_mbit);
        }
    }

    Text(getWidth(pos_x) - width_value - width_symbol, getHeight(pos_y) - height_text_small, buffer, myfont, text_scale * 0.6);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    
    /*
     * This is the reason for constant blinking of the cam icon
     */
    if (fecs_skipped > fecs_skipped_last) {
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
    } else {
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
        
        if ((armed == 1) && (declutter == 1)) {
            Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
            Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
        }
    }

    fecs_skipped_last = fecs_skipped;

    if (CHINESE) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "", osdicons, text_scale * 0.8);
    } else {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "ﵴ", osdicons, text_scale * 0.8);
    }


    if (mbit_measured != 0 && mbit > mbit_measured*mbit_warn) {
        /* 
         * Red
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A); 
    } else if (mbit_measured != 0 && mbit > mbit_measured*mbit_caution) {
        /* 
         * Yellow
         */
        Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
        Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A); 
    } else {    
        /* 
         * Normal
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        if ((armed == 1) && (declutter == 1)) {
            Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
            Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
        }
    } 

    sprintf(buffer, "%.1f/", mbit);
    TextEnd(getWidth(pos_x) + 20, getHeight(pos_y), buffer, myfont, text_scale);

    sprintf(buffer, "%.1f", mbit_tx);
    Text(getWidth(pos_x) + 20, getHeight(pos_y), buffer, myfont, text_scale * 0.6);

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    sprintf(buffer, "%d/%d",injection_failed,fecs_skipped);
    Text(getWidth(pos_x) - width_value - width_symbol, getHeight(pos_y) - height_text_small - height_text_small, buffer, myfont, text_scale * 0.6);
}



void draw_sys(uint8_t cpuload_air, uint8_t temp_air, uint8_t cpuload_gnd, uint8_t temp_gnd, int armed, float pos_x, float pos_y, float scale, float load_warn, float load_caution, float temp_warn, float temp_caution, float declutter) {

    float text_scale = getWidth(2) * scale;

    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale) + getWidth(0.5) * scale;
    VGfloat width_label = TextWidth("%", myfont, text_scale * 0.6) + getWidth(0.5) * scale;
    VGfloat width_ag = TextWidth("A", osdicons, text_scale * 0.4) - getWidth(0.3) * scale;

    

    if (cpuload_air > load_warn) {
        /* 
         * Red
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A); 
    } else if (cpuload_air > load_caution) {
        /* 
         * Yellow
         */
        Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
        Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A); 
    } else {    
        /* 
         * Normal
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        if ((armed == 1) && (declutter == 1)) {
            Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A); 
            Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
        }
    }

    TextEnd(getWidth(pos_x) - width_value - width_ag, getHeight(pos_y), "", osdicons, text_scale * 0.7);
    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "A", myfont, text_scale * 0.4); 

    sprintf(buffer, "%d", cpuload_air);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    sprintf(buffer, "%%");
    Text(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale * 0.6);

    if (temp_air > temp_warn) {
        /* 
         * Red
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A); 
    } else if (temp_air > temp_caution) {
        /* 
         * Yellow
         */
        Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
        Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A); 
    } else {    
        /* 
         * Normal color
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        if ((armed == 1) && (declutter == 1)) {
            Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
            Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
        }
    }
    sprintf(buffer, "%d°", temp_air);
    TextEnd(getWidth(pos_x) + width_value + width_label + getWidth(0.7), getHeight(pos_y), buffer, myfont, text_scale);
   
    TextEnd(getWidth(pos_x) - width_value - width_ag, getHeight(pos_y), "", osdicons, text_scale * 0.7);
    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "A", myfont, text_scale * 0.4);

    if (cpuload_gnd > load_warn) {
        /* 
         * Red
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A); 
    } else if (cpuload_gnd > load_caution) {
        /* 
         * Yellow
         */
        Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
        Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A); 
    } else {    
        /* 
         * Normal color
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        if ((armed == 1) && (declutter == 1)) {
            Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
            Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
        }
    }

    TextEnd(getWidth(pos_x) - width_value - width_ag, getHeight(pos_y) - height_text, "", osdicons, text_scale * 0.7);
    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) - height_text, "G", myfont, text_scale * 0.4);

    sprintf(buffer, "%d", cpuload_gnd);
    TextEnd(getWidth(pos_x), getHeight(pos_y) - height_text, buffer, myfont, text_scale);

    Text(getWidth(pos_x), getHeight(pos_y) - height_text, "%", myfont, text_scale * 0.6);

    if (temp_gnd > temp_warn) {
        /* 
         * Red
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A); 
    } else if (temp_gnd > temp_caution) {
        /* 
         * Yellow
         */
        Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
        Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A); 
    } else {    
        /* 
         * Normal color
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        if ((armed == 1) && (declutter == 1)) {
            Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
            Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
        }
    }
    sprintf(buffer, "%d°", temp_gnd);
    TextEnd(getWidth(pos_x) + width_value + width_label + getWidth(0.7), getHeight(pos_y) - height_text, buffer, myfont, text_scale);

    TextEnd(getWidth(pos_x) - width_value - width_ag, getHeight(pos_y) - height_text, "", osdicons, text_scale * 0.7);
    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) - height_text, "G", myfont, text_scale * 0.4);

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
}



void draw_message(int severity, const char line1[30], const char line2[30], const char line3[30], float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale * 0.7) + getHeight(0.3) * scale;
    VGfloat height_text_small = TextHeight(myfont, text_scale * 0.55) + getHeight(0.3) * scale;
    VGfloat width_text = TextWidth(line1, myfont, text_scale * 0.7);

    if (severity == 0)  {
        /* 
         * Red
         */
        Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(255, 20, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
        TextEnd(getWidth(pos_x) - width_text / 2 - getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.8);
        Text(getWidth(pos_x) + width_text / 2 + getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.8);
    } else if (severity == 1) {
        /* 
         * Yellow
         */
        Fill(229, 255, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(229, 255, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
        TextEnd(getWidth(pos_x) - width_text / 2 - getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.8);
        Text(getWidth(pos_x) + width_text / 2 + getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.8);
    } else {
        /* 
         * Normal color
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    }

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    TextMid(getWidth(pos_x), getHeight(pos_y), line1, myfont, text_scale * 0.7);
    TextMid(getWidth(pos_x), getHeight(pos_y) - height_text, line2, myfont, text_scale * 0.55);
    TextMid(getWidth(pos_x), getHeight(pos_y) - height_text-height_text_small, line3, myfont, text_scale * 0.55);
}



void draw_home_arrow(float abs_heading, float craft_heading, float pos_x, float pos_y, float scale){

    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);

    /* 
     * abs_heading is the absolute direction/bearing the arrow should point.
     * 
     * For example, bearing to home could be 45 deg because arrow is drawn relative to 
     * the osd/camera view, we need to offset by vehicles's heading
     */
    if (HOME_ARROW_INVERT) {
        abs_heading = abs_heading - 180;
    }

    /* 
     * Direction arrow needs to point relative to camera / osd / vehicle
     */
    float rel_heading = abs_heading - craft_heading;

    if (rel_heading < 0) {
        rel_heading += 360;
    }

    if (rel_heading >= 360) {
        rel_heading -=360;
    }

    pos_x = getWidth(pos_x);
    pos_y = getHeight(pos_y);

    /*
     * Offset for arrow, so middle of the arrow is at given position
     */
    pos_x -= getWidth(1.25) * scale;
    pos_y -= getWidth(1.25) * scale;

    float x[8] = {
        getWidth(0.5) * scale+pos_x, 
        getWidth(0.5) * scale+pos_x, 
        pos_x, 
        getWidth(1.25) * scale + pos_x, 
        getWidth(2.5) * scale + pos_x, 
        getWidth(2) * scale+pos_x, 
        getWidth(2) * scale + pos_x, 
        getWidth(0.5) * scale + pos_x
    };
    
    float y[8] = {
        pos_y, 
        getWidth(1.5) * scale + pos_y, 
        getWidth(1.5) * scale + pos_y, 
        getWidth(2.5) * scale + pos_y, 
        getWidth(1.5 ) *scale + pos_y, 
        getWidth(1.5) * scale + pos_y, 
        pos_y, 
        pos_y
    };

    rotatePoints(x, y, rel_heading, 8, pos_x + getWidth(1.25) * scale, pos_y + getWidth(1.25) * scale);

    Polygon(x, y, 8);
    Polyline(x, y, 8);
}



void draw_compass(float heading, float home_heading, float pos_x, float pos_y, float scale) {
    float text_scale = getHeight(1.5) * scale;
    float width_ladder = getHeight(16) * scale;
    float width_element = getWidth(0.25) * scale;
    float height_element = getWidth(0.50) * scale;
    float ratio = width_ladder / 180;

    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);

    VGfloat height_text = TextHeight(myfont, text_scale * 1.5) + getHeight(0.1) * scale;
    sprintf(buffer, "%.0f°", heading);
    TextMid(getWidth(pos_x), getHeight(pos_y) - height_element - height_text, buffer, myfont, text_scale * 1.5);

    
    int i = heading - 90;

    const char* c;

    bool draw = false;

    while (i <= heading + 90) {  
        /* 
         * Find all values from heading - 90 to heading + 90 that are % 15 == 0
         */
        
        float x = getWidth(pos_x) + (i - heading) * ratio;

        if (i % 30 == 0) {
            Rect(x-width_element / 2, getHeight(pos_y), width_element, height_element*2);
        } else if (i % 15 == 0) {
            Rect(x-width_element / 2, getHeight(pos_y), width_element, height_element);
        } else {
            i++;
            continue;
        }

        int j = i;

        if (j < 0) {
            j += 360;
        }

        if (j >= 360) {
            j -= 360;
        }

        switch (j) {
            case 0:
                draw = true;

                if (CHINESE) {
                    c = "北";
                } else {
                    c = "N";
                }
                
                break;
            case 90:
                draw = true;

                if (CHINESE) {
                    c = "东";
                } else {
                    c = "E";
                }

                break;
            case 180:
                draw = true;

                if (CHINESE) {
                    c = "南";
                } else {
                    c = "S";
                }

                break;
            case 270:
                draw = true;
                
                if (CHINESE) {
                    c = "西";
                } else {
                    c = "W";
                }

                break;
        }

        if (draw == true) {
            TextMid(x, getHeight(pos_y) + height_element * 3.5, c, myfont, text_scale * 1.5);
            draw = false;
        }

        if (j == home_heading) {
            TextMid(x, getHeight(pos_y) + height_element, "", osdicons, text_scale * 1.3);
        }

        i++;
    }

    float rel_home = home_heading - heading;

    if (rel_home<0) {
        rel_home += 360;
    }

    if ((rel_home > 90) && (rel_home <= 180)) { 
        TextMid(getWidth(pos_x) + width_ladder / 2 * 1.2, getHeight(pos_y), "", osdicons, text_scale * 0.8);
    } else if ((rel_home > 180) && (rel_home < 270)) { 
        TextMid(getWidth(pos_x) - width_ladder / 2 * 1.2, getHeight(pos_y), "", osdicons, text_scale * 0.8);
    }

    TextMid(getWidth(pos_x), getHeight(pos_y) + height_element * 2.5 + height_text, "", osdicons, text_scale * 2);
}



void draw_batt_status(float voltage, float current, float pos_x, float pos_y, float scale){
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);

    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;

    if (BATT_STATUS_CURRENT) {
        sprintf(buffer, "%.1f", current);
        TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
        Text(getWidth(pos_x), getHeight(pos_y), " A", myfont, text_scale * 0.6);
    }

    sprintf(buffer, "%.1f", voltage);
    TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text, buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y) + height_text, " V", myfont, text_scale * 0.6);
}



void draw_TOTAL_AMPS(float current, float pos_x, float pos_y, float scale) { 
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);

    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;

    sprintf(buffer, "%5.0f", current);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), " mAh", myfont, text_scale * 0.6);
}



void draw_TOTAL_DIST(double kmh, float pos_x, float pos_y, float scale) {
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
 
    /*
     * Get time passed since last rendering
     */
    long time_diff = current_ts() - dist_ts;
    dist_ts = current_ts();

    float _hours = (float)time_diff / (float)3600000;

    float added_distance = kmh * _hours;

    total_dist = total_dist + added_distance;
 
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;

    sprintf(buffer, "%3.1f", total_dist);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), " km", myfont, text_scale * 0.6);
}



void draw_TOTAL_TIME(float fly_time, float pos_x, float pos_y, float scale) {   
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A); 
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;
    sprintf(buffer, "%3.0f:%02d", fly_time, (int)(fly_time * 60) % 60);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), "", osdicons, text_scale * 0.9);
 
}



void draw_position(float lat, float lon, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;
    VGfloat width_value = TextWidth("-100.000000", myfont, text_scale);

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A); //normal
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    float mylat;
    float mylon;


    if (HIDE_LATLON) {
        mylat = lat - (int)lat;
        mylon = lon - (int)lon; 
    } else {
        mylon=lon;
        mylat=lat;
    }


    if (CHINESE) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "  ", osdicons, text_scale * 0.6);
    } else {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "ﵳ", osdicons, text_scale * 0.6);
    }

    if (HIDE_LATLON) {
        sprintf(buffer, "0%f", mylon);
    } else {
        sprintf(buffer, "%.6f", mylon);
    }


    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);


    if (CHINESE) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) + height_text, "  ", osdicons, text_scale * 0.6);
    } else {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) + height_text, "ﵲ", osdicons, text_scale * 0.6);
    }


    if (HIDE_LATLON) {
        sprintf(buffer, "0%f", mylat);
    } else {
        sprintf(buffer, "%.6f", mylat);
    }

    TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text, buffer, myfont, text_scale);
}




void draw_home_distance(int distance, bool home_fixed, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    VGfloat width_value = TextWidth("00000", myfont, text_scale);

    if (!home_fixed) {
        /* 
         * Red
         */

        Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(255, 20, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
    } else {
        /* 
         * Normal
         */

        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    }

    TextEnd(getWidth(pos_x) - width_value - getWidth(0.2), getHeight(pos_y), "", osdicons, text_scale * 0.6);

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    if (IMPERIAL) {
        Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "ft", myfont, text_scale * 0.6);
        sprintf(buffer, "%05d", (int)(distance * TO_FEET));
    } else {
        Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "m", myfont, text_scale * 0.6);
        sprintf(buffer, "%05d", distance);
    }

    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}




void draw_alt_ladder(int alt, float pos_x, float pos_y, float scale, float warn, float caution, float vsi_time, float climb) {

    if (IMPERIAL) {
        alt = alt * TO_FEET;
    }

    float text_scale = getHeight(1.3) * scale;
    float width_element = getWidth(0.50) * scale;
    float height_element = getWidth(0.25) * scale;
    float height_ladder = height_element * 21 * 4;


    /* 
     * Ladder X position
     */
    float px = getWidth(pos_x);

    /* 
     * Alt labels on ladder X position
     */
    float pxlabel = getWidth(pos_x) + width_element * 2;


    /* 
     * Alt range range of display, i.e. lowest and highest number on the ladder
     */
    float range = 100; 
    float range_half = range / 2;
    float ratio_alt = height_ladder / range;

    VGfloat offset_text_ladder = (TextHeight(myfont, text_scale) / 2) - height_element / 2 - getHeight(0.25) * scale;
    VGfloat offset_symbol = (TextHeight(osdicons, text_scale * 2) / 2) - height_element / 2 - getHeight(0.18) * scale;
    VGfloat offset_alt_value = (TextHeight(myfont, text_scale * 2) / 2) -height_element / 2 - getHeight(0.4) * scale;

    VGfloat width_symbol = TextWidth("", osdicons, text_scale * 2);
    VGfloat width_ladder_value = TextWidth("000", myfont, text_scale);

    if (alt < warn) {
        /* 
         * Red
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A); 
    } else if (alt < caution) {
        /* 
         * Yellow
         */
        Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
        Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A); 
    } else {    
        /* 
         * Normal
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    }


    /* 
     * Large ALT number
     */

    sprintf(buffer, "%d", alt);

    Text(px + width_element + 20 * scale, getHeight(pos_y) - offset_alt_value, buffer, myfont, text_scale * 1.7);
    Text(px + width_element + 20 * scale, getHeight(pos_y) - offset_symbol, "ᎆ", osdicons, text_scale * 1.7);


    /* 
     * Normal color
     */
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);


    int k;
    for (k = (int)(alt - range / 2); k <= alt + range / 2; k++) {

        int y = getHeight(pos_y) + (k - alt) * ratio_alt;

        if (k % 10 == 0) {
            if (k >= 0) {
                /* 
                * Normal color
                */

                Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
                Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

                Rect(px-width_element, y, width_element * 2, height_element);
            
            if (k > alt + 5 || k < alt - 5) {
                sprintf(buffer, "%d", k);

                Text(pxlabel, y-offset_text_ladder, buffer, myfont, text_scale);
            }
        }

        if (k < 0) {
            /* 
            * Red
            */

            Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
            Stroke(255, 20, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
            Rect(px - width_element, y + width_element * 1.3, width_element * 2, width_element * 2);
        }
    } else if ((k % 5 == 0) && (k > 0)) {
            /* 
            * Normal
            */

            Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
            Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

            Rect(px-width_element, y, width_element, height_element);
        }
    }
    
    /* 
     * VSI
     */
    if (climb < 0) {
        /* 
         * Opaque outline, yellow for descent
         */

        Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);

        Fill(245, 222, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
    } else {
        /* 
         * Opaque outline, green for climb
         */

        Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);

        Fill(43, 240, 36, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
    }

    VGfloat *LX;
    VGfloat *LY;

    VGfloat Left_X[5] = { 
        px + width_element - 4, 
        px + width_element + 5,
        px + width_element + 5, 
        px + width_element + 1.5,
        px + width_element - 4
    };

    /*
     * Really dirty var scope fix
     *
     * So arrow points up and or down with positive and negative climb
     */

    if (climb > 0) {

        VGfloat Left_Y[5] = {
            getHeight(pos_y), 
            getHeight(pos_y), 
            getHeight(pos_y) + climb * vsi_time, 
            getHeight(pos_y) + climb * vsi_time + 2.5,
            getHeight(pos_y) + climb * vsi_time
        };

        VGint npt = 5;
        LX = &Left_X[0];
        LY = &Left_Y[0];

        Polygon(LX, LY, npt);
    } else { 

        VGfloat Left_Y[5] = {
            getHeight(pos_y), 
            getHeight(pos_y), 
            getHeight(pos_y) + climb * vsi_time, 
            getHeight(pos_y) + climb * vsi_time - 2.5,
            getHeight(pos_y) + climb * vsi_time
        };

        VGint npt = 5;

        LX = &Left_X[0];
        LY = &Left_Y[0];

        Polygon(LX, LY, npt);
    }
}




void draw_mslalt(float mslalt, float pos_x, float pos_y, float scale) {

    float text_scale = getWidth(2) * scale;

    /* 
     * Normal color
     */
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    VGfloat width_value;

    if (IMPERIAL) {
        width_value = TextWidth("0000", myfont, text_scale);
        sprintf(buffer, "%.0f", mslalt*TO_FEET);
    } else {
        width_value = TextWidth("000.0", myfont, text_scale);
        sprintf(buffer, "%.1f", mslalt);
    }


    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);


    if (CHINESE) {
        TextEnd(getWidth(pos_x) - width_value - getWidth(0.3)*scale, getHeight(pos_y), " ", osdicons, text_scale * 0.7);
    } else {
        TextEnd(getWidth(pos_x) - width_value - getWidth(0.3)*scale, getHeight(pos_y), "ﶁ", osdicons, text_scale * 0.7);
    }


    if (IMPERIAL) {
        Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "ft", myfont, text_scale * 0.6);
    } else {
        Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "m", myfont, text_scale * 0.6);
    }
}




void draw_speed_ladder(int speed, float pos_x, float pos_y, float scale, float trend_time, float low_limit, float vx) {

    /* 
     * Normal color
     */
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    float text_scale = getHeight(1.3) * scale;
    float width_element = getWidth(0.50) * scale;
    float height_element = getWidth(0.25) * scale;
    float height_ladder = height_element * 21 * 4;


    /* 
     * Ladder X position
     */
    float px = getWidth(pos_x);

    /* 
     * Speed labels on ladder X position
     */
    float pxlabel = getWidth(pos_x) - width_element * 2;


    /* 
     * Speed range of display, i.e. lowest and highest number on the ladder
     */
    float range = 40;
    float range_half = range / 2;
    float ratio_speed = height_ladder / range;

    VGfloat offset_text_ladder = (TextHeight(myfont, text_scale) / 2) - height_element / 2 - getHeight(0.25) * scale;
    VGfloat offset_symbol = (TextHeight(osdicons, text_scale * 2) / 2) - height_element / 2 - getHeight(0.18) * scale;
    VGfloat offset_speed_value = (TextHeight(myfont, text_scale * 2) / 2) -height_element / 2 - getHeight(0.4) * scale;

    VGfloat width_symbol = TextWidth("", osdicons, text_scale * 2);
    VGfloat width_ladder_value = TextWidth("0", myfont, text_scale);


    /* 
     * Large speed number
     */
    sprintf(buffer, "%d", speed);


    TextEnd(pxlabel - 9 * scale, getHeight(pos_y) - offset_speed_value, buffer, myfont, text_scale * 1.7);
    TextEnd(pxlabel - 9 * scale, getHeight(pos_y) - offset_symbol, "ᎄ", osdicons, text_scale * 1.7);


    int k;
    for (k = (int)(speed - range_half); k <= speed + range_half; k++) {
        int y = getHeight(pos_y) + (k - speed) * ratio_speed;

        /* 
         * Wide element plus number label every 5 'ticks' on the scale
         */
        if (k % 5 == 0) {
            
            if (k >= low_limit) {
                /* 
                 * Normal color
                 */
                Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
                Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
                Rect(px - width_element, y, width_element * 2, height_element);
            
                /* 
                 * So the box around number is not overwritten
                 */
                if (k > speed + 3 || k < speed - 3) {
                    sprintf(buffer, "%d", k);
                    TextEnd(pxlabel, y - offset_text_ladder, buffer, myfont, text_scale);
                }
            }

            if (k < low_limit) {
                /* 
                 * Red color
                 */

                Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
                Stroke(255, 20, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));

                Rect(px - width_element, y + width_element * 1.9, width_element * 2, width_element * 2);
            }
        } else if ((k % 1 == 0) && (k > low_limit)) { 
            /*
             * Narrow element every single 'tick' on the scale 
             *
             * Normal color
             */

            Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
            Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
            Rect(px, y, width_element, height_element);
        }
    }

    /*
     * Speed Trend
     */

    if (vx < 0) {
        /* 
         * Opaque, yellow for descent
         */
        Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);

        Fill(245, 222, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
    } else {
        /* 
         * Opaque, green for climb
         */
        Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);

        Fill(43, 240, 36, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
    }

    
    VGfloat *LX;
    VGfloat *LY;

    VGfloat Left_X[5] = {
        pxlabel + 3, 
        pxlabel + 12, 
        pxlabel + 12, 
        pxlabel + 7.5, 
        pxlabel + 3
    };

    /* 
     * Really dirty var scope fix
     * 
     * So arrow points up and or down with positive and negative climb
     */
    if (vx > 0) { 

        VGfloat Left_Y[5] = {
            getHeight(pos_y), 
            getHeight(pos_y), 
            getHeight(pos_y) + vx * trend_time,
            getHeight(pos_y) + vx * trend_time + 2.5,
            getHeight(pos_y) + vx * trend_time
        };
        
        VGint npt = 5;
        
        LX = &Left_X[0];
        LY = &Left_Y[0];

        Polygon(LX, LY, npt);
    } else {
        VGfloat Left_Y[5] = {
            getHeight(pos_y),
            getHeight(pos_y), 
            getHeight(pos_y) + vx * trend_time,
            getHeight(pos_y) + vx * trend_time - 2.5,
            getHeight(pos_y) + vx * trend_time
        };

        VGint npt = 5;

        LX = &Left_X[0];
        LY = &Left_Y[0];
        
        Polygon(LX, LY, npt);
    }
}




void draw_yaw_display(float vy, float pos_x, float pos_y, float scale, float trend_time) {
    /* 
     * Normal color
     */
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    float text_scale = getHeight(1.5) * scale;
    float height_display = getHeight(1.5) * scale;
    float width_element = getWidth(10) * scale;
    float height_element = getWidth(1) * scale;


    VGfloat *LX;
    VGfloat *LY;

    VGfloat Left_Y[5] = {
        getHeight(pos_y), 
        getHeight(pos_y), 
        getHeight(pos_y) + height_element / 2, 
        getHeight(pos_y) + height_element,
        getHeight(pos_y) + height_element
    };

    /*
     * Really dirty var scope fix
     */
    if (vy < 0) {

        VGfloat Left_X[5] = { 
            getWidth(pos_x), 
            getWidth(pos_x) + (vy * trend_time), 
            getWidth(pos_x) + (vy * trend_time) + height_element / 2,
            getWidth(pos_x) + (vy * trend_time), 
            getWidth(pos_x)
        };
        
        VGint npt = 5;

        LX = &Left_X[0];
        LY = &Left_Y[0];

        Polygon(LX, LY, npt);
    }

    if (vy > 0) {
        /* 
         * So tip of arrow goes correct way
         */
        VGfloat Left_X[5] = {
            getWidth(pos_x), 
            getWidth(pos_x) + (vy * trend_time), 
            getWidth(pos_x) + (vy * trend_time) - height_element / 2,
            getWidth(pos_x) + (vy * trend_time),
            getWidth(pos_x)
        };
        
        VGint npt = 5;

        LX = &Left_X[0];
        LY = &Left_Y[0];

        Polygon(LX, LY, npt);
    }
}


void draw_card_signal(int8_t signal, int signal_good, int card, int adapter_cnt, int restart_count, int packets, int wrongcrcs, int type, int totalpackets, int totalpacketslost, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.4) * scale;
    VGfloat width_value = TextWidth("-00", myfont, text_scale);
    VGfloat width_cardno = TextWidth("0", myfont, text_scale * 0.4);
    VGfloat width_unit = TextWidth("dBm", myfont, text_scale * 0.6);

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    StrokeWidth(OUTLINEWIDTH);

    sprintf(buffer, "");
    TextEnd(getWidth(pos_x) - width_value - width_cardno - getWidth(0.3) * scale, getHeight(pos_y) - card * height_text, buffer, osdicons, text_scale * 0.6);

    sprintf(buffer, "%d",card);
    TextEnd(getWidth(pos_x) - width_value - getWidth(0.3) * scale, getHeight(pos_y) - card * height_text, buffer, myfont, text_scale * 0.4);

    if ((signal_good == 0) || (signal == -127)) {
        /* 
         * Red
         */
        Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(255, 20, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));

        sprintf(buffer, "-- ");
    } else {
        /* 
         * Normal color
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    }

    sprintf(buffer, "%d", signal);
    
    TextEnd(getWidth(pos_x), getHeight(pos_y) - card * height_text, buffer, myfont, text_scale);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    sprintf(buffer, "dBm");
    Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y) - card * height_text, buffer, myfont, text_scale * 0.6);

    if (restart_count - tx_restart_count_last > 0) {
        int y;

        for (y = 0; y < adapter_cnt; y++) {
            packetslost_last[y] = 0;
        }
    }

    tx_restart_count_last = restart_count;

    int lost = totalpackets - packets + totalpacketslost;

    if (lost < packetslost_last[card]) {
        lost = packetslost_last[card];
    }

    packetslost_last[card] = lost;

    int percent_lost_card = (int)((double)lost / packets * 100);

    sprintf(buffer, "%d (%d%%)", lost, percent_lost_card);
    Text(getWidth(pos_x) + width_unit + getWidth(0.65) * scale, getHeight(pos_y) - card * height_text, buffer, myfont, text_scale * 0.7);
}



void draw_total_signal(int8_t signal, int goodblocks, int badblocks, int packets_lost, int packets_received, int lost_per_block, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    VGfloat height_text = TextHeight(myfont, text_scale * 0.6) + getHeight(0.3) * scale;
    VGfloat width_value = TextWidth("-00", myfont, text_scale);
    VGfloat width_label = TextWidth("dBm", myfont, text_scale * 0.6);
    VGfloat width_symbol = TextWidth(" ", osdicons, text_scale * 0.7);

    /* 
     * Normal color
     */
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    StrokeWidth(OUTLINEWIDTH);

    if (no_signal == true) {
        /* 
         * Red
         */
        Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(255, 20, 20,getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));

        sprintf(buffer, "-- ");
    } else {
        /* 
         * Normal color
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

        sprintf(buffer, "%02d", signal);
    }


    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    Text(getWidth(pos_x) + getWidth(0.4), getHeight(pos_y), "dBm", myfont, text_scale * 0.6);


    int percent_badblocks = (int)((double)badblocks / goodblocks * 100);
    int percent_packets_lost = (int)((double)packets_lost / packets_received * 100);

    sprintf(buffer, "%d (%d%%)/%d (%d%%)", badblocks, percent_badblocks, packets_lost, percent_packets_lost);
    
    Text(getWidth(pos_x) - width_value - width_symbol + 2, getHeight(pos_y) - height_text, buffer, myfont, text_scale * 0.6);

    TextEnd(getWidth(pos_x) - width_value - getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.7);


    /* 
     * Moving average: lost per block (LPB)
     */
    int lpb_average=0;
    int lpb_sum=0;


    /* 
     * New LPB, overwrite oldest value in array
     */
    if (lpb_counter>20) {
        lpb_counter=0;
    }

    lpb_array[lpb_counter++] = lost_per_block;


    /*
     * Calculate average
     * 
     * Total array value divided by array size
     */
    int i;
    for (i = 0; i < 20; i++) {
        lpb_sum += lpb_array[i];
    }
    

    /* 
     * 8 per block. so array length x 8
     */
    lpb_average = lpb_sum / 20;


    /* 
     * Display options
     * 
     * TODO: Graphical only, or with number 
     */
    switch (lpb_average) {
        case 0: {
            sprintf(buffer, "▁");
            Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
            
            break;
        }
        case 1: {
            sprintf(buffer, "▂");
            Fill(74, 255, 4, 0.5);

            break;
        }
        case 2: {
            sprintf(buffer, "▃");
            Fill(112, 255, 4, 0.5);
                
            break;
        }
        case 3: {
            sprintf(buffer, "▄");
            Fill(182, 255, 4, 0.5);

            break;
        }
        case 4: {
            sprintf(buffer, "▅");
            Fill(255, 208, 4, 0.5);

            break;
        }
        case 5: {
            sprintf(buffer, "▆");
            Fill(255, 93, 4, 0.5);

            break;
        }
        case 6: {
            sprintf(buffer, "▇");
            Fill(255, 50, 4, 0.5);

            break;
        }
        case 7: {
            sprintf(buffer, "█");
            Fill(255, 0, 4, 0.5);

            break;
        }
        case 8: {
            sprintf(buffer, "█");
            Fill(255, 0, 4, 0.5);
            
            break;
        }
        default: {
            sprintf(buffer, "█");
            Fill(255, 0, 4, 0.5);

            break;
        }
    }

    if (DOWNLINK_RSSI_FEC_BAR) {
        StrokeWidth(0);
        Text(getWidth(pos_x) + width_label + getWidth(0.7), getHeight(pos_y) + getHeight(0.5), buffer, osdicons, text_scale * 0.7);

        StrokeWidth(1);
        Fill(0, 0, 0, 0);
        Text(getWidth(pos_x) + width_label + getWidth(0.7), getHeight(pos_y) + getHeight(0.5), "█", osdicons, text_scale * 0.7);

        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
        sprintf(buffer, "%d/12", lpb_average);
        Text(getWidth(pos_x) + width_label + getWidth(3), getHeight(pos_y) + getHeight(0.5), buffer, myfont, text_scale * 0.5);
    }
}



void draw_sat(int sats, int fixtype, int hdop, int armed, float pos_x, float pos_y, float scale, float hdop_warn, float hdop_caution, float declutter) {
    
    float text_scale = getWidth(2) * scale;

    StrokeWidth(OUTLINEWIDTH);
    

    if (fixtype < 2) {
        /* 
         * Red
         */
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
    } else {
        /* 
         * Normal color
         */
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
        
        if ((armed == 1) && (declutter == 1)) {
            /* 
             * Opaque
             */
            Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
            Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
        }
    }


    TextEnd(getWidth(pos_x), getHeight(pos_y), "", osdicons, text_scale * 0.7);
   


    if (LTM || MAVLINK || VOT) {

        float decimal_hdop = (float) hdop / 100;

        if (decimal_hdop > hdop_warn) {
            /* 
             * Red
             */

            Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
            Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A); 
        } else if (decimal_hdop > hdop_caution) {
            /* 
             * Yellow
             */

            Stroke(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A);
            Fill(COLOR_CAUTION_R, COLOR_CAUTION_G, COLOR_CAUTION_B, COLOR_CAUTION_A); 
        } else {    
            /* 
             * Normal color
             */

            Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
            Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

            if ((armed == 1) && (declutter == 1)) {
                /* 
                 * Opqaue
                 */

                Stroke(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
                Fill(COLOR_DECLUTTER_R, COLOR_DECLUTTER_G, COLOR_DECLUTTER_B, COLOR_DECLUTTER_A);
            }
        } 
        
        sprintf(buffer, "%d(%.2f)", sats, decimal_hdop);
        Text(getWidth(pos_x) + getWidth(0.2), getHeight(pos_y), buffer, myfont, text_scale);
    }

    /* 
     * Normal color
     */
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
}



void draw_batt_gauge(int remaining, float pos_x, float pos_y, float scale) {

    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);

    if (remaining < 0) {
        remaining = 0;
    } else if (remaining > 100) {
        remaining = 100;
    }

    int cell_width = getWidth(4) * scale;
    int cell_height = getWidth(1.6) * scale;

    int plus_width = cell_width * 0.09;
    int plus_height = cell_height * 0.3;

    int corner = cell_width * 0.05;
    int stroke_x = cell_width * 0.05;
    int stroke_y = cell_height * 0.1;



    if (remaining <= CELL_WARNING_PCT1 && remaining > CELL_WARNING_PCT2) {
        Fill(255, 165, 0, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(255, 165, 0, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
    } else if (remaining <= CELL_WARNING_PCT2) { 
        Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(255, 20, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
    } else {
        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    }


    StrokeWidth(OUTLINEWIDTH);


    /* 
     * Battery cell
     */
    Roundrect(getWidth(pos_x), getHeight(pos_y), cell_width, cell_height, corner, corner);


    /* 
     * Battery plus pole
     */
    Rect(getWidth(pos_x) + cell_width, getHeight(pos_y) + cell_height / 2 - plus_height / 2, plus_width, plus_height);

    Fill(0, 0, 0, 0.5);
    Rect(getWidth(pos_x) + stroke_x + remaining / 100.0f * cell_width, getHeight(pos_y) + stroke_y, cell_width - stroke_x * 2 - remaining / 100.0f * cell_width, cell_height - stroke_y * 2);
}



void draw_ahi(float roll, float pitch, float scale){
    float text_scale = getHeight(1.2) * scale;
    float height_ladder = getWidth(15) * scale;
    float width_ladder = getWidth(10) * scale;
    float height_element = getWidth(0.25) * scale;
    float range = 20;
    float space_text = getWidth(0.2) * scale;
    float ratio = height_ladder / range;
    float pos_x = getWidth(50);
    float pos_y = getHeight(50);

    VGfloat offset_text_ladder = (TextHeight(myfont, text_scale * 0.85) / 2) - height_element / 2;

   /* 
    * Left three bars
    */
    float px_l  = pos_x - width_ladder / 2 + width_ladder / 3 - width_ladder / 12;
    float px3_l = pos_x - width_ladder / 2 + 0.205f * width_ladder- width_ladder / 12;
    float px5_l = pos_x - width_ladder / 2 + 0.077f * width_ladder- width_ladder / 12;
   
    /* 
     * Right three bars
     */
    float px_r =  pos_x + width_ladder / 2 - width_ladder / 3;
    float px3_r = pos_x + width_ladder / 2 - 0.205f * width_ladder;
    float px5_r = pos_x + width_ladder / 2 - 0.077f * width_ladder;

    /* 
     * Normal color
     */
    StrokeWidth(OUTLINEWIDTH);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);


    TextEnd(getWidth(50), getHeight(50), "ᎅ", osdicons, text_scale * 2.5);


    Translate(pos_x, pos_y);
    Rotate(roll);
    Translate(-pos_x, -pos_y);


    int k = pitch - range / 2;
    int max = pitch + range / 2;

    while (k <= max) {
        float y = pos_y + (k - pitch) * ratio;

        if (k % 5 == 0 && k!= 0) {
            if (AHI_LADDER) {
                sprintf(buffer, "%d", k);

                /*
                 * Right numbers
                 */
                TextEnd(pos_x - width_ladder / 2 - space_text, y - width / height_ladder, buffer, myfont, text_scale * 0.85);

                /*
                 * Left numbers
                 */
                Text(pos_x + width_ladder / 2 + space_text, y - width / height_ladder, buffer, myfont, text_scale * 0.85);
            }
        }
        
        if ((k > 0) && (k % 5 == 0)) { 
            /*
             * Upper ladders
             */
            if (AHI_LADDER) {
                float px = pos_x - width_ladder / 2;

                Rect(px, y, width_ladder / 3, height_element);
                Rect(px + width_ladder * 2 / 3, y, width_ladder / 3, height_element);
            }
        } else if ((k < 0) && (k % 5 == 0)) { 
            /* 
             * Lower ladders
             */
            if (AHI_LADDER) {
                Rect( px_l, y, width_ladder/12, height_element);
                Rect(px3_l, y, width_ladder/12, height_element);
                Rect(px5_l, y, width_ladder/12, height_element);
                Rect( px_r, y, width_ladder/12, height_element);
                Rect(px3_r, y, width_ladder/12, height_element);
                Rect(px5_r, y, width_ladder/12, height_element);
            }
        } else if (k == 0) { 
            /*
             * Center line
             */
            if (AHI_LADDER) {
                sprintf(buffer, "%d", k);
                TextEnd(pos_x - width_ladder / 1.25f - space_text, y - width / height_ladder, buffer, myfont, text_scale * 0.85); // left number
                Text(pos_x + width_ladder / 1.25f + space_text, y - width / height_ladder, buffer, myfont, text_scale * 0.85); // right number
            }

            Rect(pos_x - width_ladder / 1.25f, y, 2 * (width_ladder /1.25f), height_element);
        }

        k++;
    }
}


void draw_ahi_mav(float roll, float pitch, float climb, float vz, float vx, float vy, float gpsspeed, float alt, float scale) {
    float text_scale = getHeight(1.2) * scale;
    float height_ladder = getWidth(15) * scale;
    float width_ladder = getWidth(30) * scale;
    float height_element = getWidth(0.25) * scale;
    float range = 20;
    float space_text = getWidth(0.2) * scale;
    float ratio = height_ladder / range;
    float pos_x = getWidth(50);
    float pos_y = getHeight(50);


    /*
     * Limit draw area
     */
    ClipRect(pos_x-width_ladder, pos_y - getHeight(30), width_ladder * 2, getHeight(60));


    VGfloat offset_text_ladder = (TextHeight(myfont, text_scale * 0.85) / 2) - height_element / 2;


    /* 
     * Left three bars
     */
    float px_l  = pos_x - width_ladder / 2 + width_ladder / 3 - width_ladder / 12;
    float px3_l = pos_x - width_ladder / 2 + 0.205f * width_ladder - width_ladder / 12;
    float px5_l = pos_x - width_ladder / 2 + 0.077f * width_ladder - width_ladder / 12;


    /* 
     * Right three bars
     */
    float px_r =  pos_x + width_ladder / 2 - width_ladder / 3;
    float px3_r = pos_x + width_ladder / 2 - 0.205f * width_ladder;
    float px5_r = pos_x + width_ladder / 2 - 0.077f * width_ladder;
  

    StrokeWidth(OUTLINEWIDTH);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    

    if (alt < ALTLADDER_CAUTION) {

        if (climb < -3.0f) {
            /* 
             * Red for warning
             */

            Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
            Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        } else if ((climb >= - 3.0f) && (climb < - 1.5f)) {
            /* 
             * Yellow for caution
             */

            Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
            Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        } else {
            /* 
             * Normal color
             */

            StrokeWidth(OUTLINEWIDTH);
            Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
            Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        }
    }


    if (gpsspeed < SPEEDLADDER_LOW_LIMIT) {
        /* 
         * Red
         */
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
    }
  

    Translate(pos_x, pos_y);
    Rotate(roll);
    Translate(-pos_x, - pos_y);


    /* 
     * FPV Y axis
     */
    double fpv_y;
    fpv_y = tan(vx / vy);
    fpv_y = fpv_y + 90;


    /* 
     * FPV Z axis
     */
    double fpv_z;
    fpv_z = tan(vx / vz) + 90; 

    

    /* 
    * Range and max are not as important, because clipping is implemented
    */
    int k = pitch - range;
    int max = pitch + range;

    while (k <= max) {
        float y = pos_y + (k - pitch) * ratio;
        
        if (k % 5 == 0 && k != 0) {
            if (AHI_LADDER) {  
                if (AHI_ROLLANGLE) {
                    sprintf(buffer, "%.1f°", roll*AHI_ROLLANGLE_INVERT);

                    /* 
                     * Right numbers
                     */
                    Text(pos_x + width_ladder / 2 + space_text, y - width / height_ladder, buffer, myfont, text_scale * 1.1);
                } else {
                    sprintf(buffer, "%d", k);

                    /* 
                     * Right numbers
                     */
                    Text(pos_x + width_ladder / 2 + space_text, y - width / height_ladder, buffer, myfont, text_scale * 1.1);
                }

                sprintf(buffer, "%d", k);

                /* 
                 * Left numbers
                 */
                TextEnd(pos_x - width_ladder / 2 - space_text, y - width / height_ladder, buffer, myfont, text_scale * 1.1);
            }
        }

        if ((k > 0) && (k % 5 == 0)) {
            /* 
             * Upper ladders
             */

            if (AHI_LADDER) {
                float px = pos_x - width_ladder / 2;
                Rect(px, y, width_ladder / 3, height_element);
                Rect(px+width_ladder * 2 / 3, y, width_ladder / 3, height_element);
            }
        } else if ((k < 0) && (k % 5 == 0)) {
            /* 
             * Lower ladders
             */

            if (AHI_LADDER) {
                Rect( px_l, y, width_ladder/12, height_element);
                Rect(px3_l, y, width_ladder/12, height_element);
                Rect(px5_l, y, width_ladder/12, height_element);
                Rect( px_r, y, width_ladder/12, height_element);
                Rect(px3_r, y, width_ladder/12, height_element);
                Rect(px5_r, y, width_ladder/12, height_element);
            }
        } else if (k == 0) {
            /* 
             * Center line
             */

            if (AHI_LADDER) {
                sprintf(buffer, "%d", k);

                /* 
                 * Left number
                 */
                TextEnd(pos_x - width_ladder / 1.25f - space_text, y - width / height_ladder, buffer, myfont, text_scale * 1.1);
                
                if (AHI_ROLLANGLE) {
                    sprintf(buffer, "%.1f°", roll*AHI_ROLLANGLE_INVERT);

                    /* 
                     * Right number
                     */
                    Text(pos_x + width_ladder / 1.25f + space_text, y - width / height_ladder, buffer, myfont, text_scale * 1.1);
                } else {
                    sprintf(buffer, "%d", k);

                    /* 
                     * Right number
                     */
                    Text(pos_x + width_ladder / 1.25f + space_text, y - width / height_ladder, buffer, myfont, text_scale * 1.1);  
                }  
            }
            
            //DRAW MAIN HORIZON BAR
            Rect(pos_x - width_ladder / 1.25f, y, 2 * (width_ladder / 1.25f) / 5 * 2, height_element * 1.5);
            Rect(pos_x - width_ladder / 1.25f + 2 * (width_ladder / 1.25f) / 5 * 3, y, 2 * (width_ladder /1.25f) / 5 * 2, height_element * 1.5);

            //Bore Sight
            TextEnd(pos_x + vy * 30, y - vz * 30, "ᎅ", osdicons, text_scale * 2.5);

            if (AHI_ROLLANGLE) {
                sprintf(buffer, "%.1f°", roll * AHI_ROLLANGLE_INVERT);

                /* 
                 * Right number
                 */
                Text(pos_x + width_ladder / 1.25f + space_text, y - width / height_ladder, buffer, myfont, text_scale * 1.1);
            }
        }

        k++;
    }

    //TODO
    ClipEnd();

    StrokeWidth(OUTLINEWIDTH);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
}



// work in progress
void draw_osdinfos(int osdfps, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale);

    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "OSDFPS:", myfont, text_scale * 0.6);

    sprintf(buffer, "%d", osdfps);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}



float distance_between(float lat1, float long1, float lat2, float long2) {
    /*
     * Taken from tinygps: https://github.com/mikalhart/TinyGPS/blob/master/TinyGPS.cpp#L296
     * 
     * Returns the distance in meters between two positions, both specified
     * as signed decimal-degrees, latitude and longitude. Uses great-circle
     * distance computation for the hypothetical sphere of radius 6372795 meters.
     * 
     * Because Earth is not an exact sphere, rounding errors may be up to 0.5%.
     * 
     * Courtesy of Maarten Lamers
     */

    float delta = (long1 - long2) * 0.017453292519;
    float sdlong = sin(delta);
    float cdlong = cos(delta);

    lat1 = (lat1) * 0.017453292519;
    lat2 = (lat2) * 0.017453292519;

    float slat1 = sin(lat1);
    float clat1 = cos(lat1);
    float slat2 = sin(lat2);
    float clat2 = cos(lat2);

    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
    delta = delta * delta;
    delta += (clat2 * sdlong) * (clat2 * sdlong);
    delta = sqrt(delta);

    float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
    delta = atan2(delta, denom);

    return delta * 6372795;
}



float course_to (float lat1, float long1, float lat2, float long2) {
    /*
     * Taken from tinygps: https://github.com/mikalhart/TinyGPS/blob/master/TinyGPS.cpp#L321
     * 
     * Returns the course in degrees (North = 0, West = 270) from position 1 to position 2,
     * both specified as signed decimal-degrees, latitude and longitude.
     * 
     * Because Earth is not an exact sphere, calculated course may be off by a tiny fraction.
     * 
     * Courtesy of Maarten Lamers
     *
     */

    float dlon = (long2 - long1) * 0.017453292519;

    lat1 = (lat1) * 0.017453292519;
    lat2 = (lat2) * 0.017453292519;
    
    float a1 = sin(dlon) * cos(lat2);
    float a2 = sin(lat1) * cos(lat2) * cos(dlon);
    
    a2 = cos(lat1) * sin(lat2) - a2;
    a2 = atan2(a1, a2);
    
    if (a2 < 0.0) {
        a2 += M_PI * 2;
    }
    
    return TO_DEG*(a2);
}



void rotatePoints(float *x, float *y, float angle, int points, int center_x, int center_y) {
    double cosAngle = cos(-angle * 0.017453292519);
    double sinAngle = sin(-angle * 0.017453292519);

    int i = 0;

    float tmp_x = 0;
    float tmp_y = 0;

    while (i < points) {
        tmp_x = center_x + (x[i] - center_x) * cosAngle - (y[i] - center_y) * sinAngle;
        tmp_y = center_y + (x[i] - center_x) * sinAngle + (y[i] - center_y) * cosAngle;

        x[i] = tmp_x;
        y[i] = tmp_y;

        i++;
    }
}



void draw_RPA(float roll, float pitch, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    VGfloat width_value = TextWidth("00000.0", myfont, text_scale) * 1.1;
    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;

    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
    
    sprintf(buffer, "%.1f°", roll);
    TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * 1.3, buffer, myfont, text_scale);
    
    if (roll - 5.01 > 0) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) + height_text * 1.3, "ᎇ", osdicons, text_scale * 1.2);
    }
    else if (roll + 5.01 < 0) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) + height_text * 1.3, "ᎈ", osdicons, text_scale * 1.2);
    }
    else {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) + height_text * 1.3, "᎔", osdicons, text_scale * 1.2);
    }


    
    sprintf(buffer, "%.1f°", pitch);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    


    if (pitch - 3.01 > 0) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "ᎉ", osdicons, text_scale * 1.2);
    } else if (pitch + 3.01 < 0) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "ᎊ", osdicons, text_scale * 1.2);
    } else {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "᎕", osdicons, text_scale * 1.2);
    }
}



void draw_Mission(int Seq,float pos_x, float pos_y, float scale){
    
    float text_scale = getWidth(2) * scale;

    VGfloat width_value = TextWidth("00", myfont, text_scale);
    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;


    if (CHINESE) {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) + getHeight(0.3) * scale, "航 点:", myfont, text_scale * 0.9);
    } else {
        TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y) + getHeight(0.3) * scale, "Mission:", myfont, text_scale * 0.9);
    }


    sprintf(buffer, "%d", Seq);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}



void draw_Angle(float pos_x, float pos_y, float scale) {
    
    float text_scale = getWidth(2) * scale;

    VGfloat width_value = TextWidth("00", myfont, text_scale);
    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(0.3) * scale;

    //角度指示器1
    TextEnd(getWidth(pos_x), getHeight(pos_y), "ᐕ", osdicons, text_scale * 5);  
}



void draw_Angle2(float pos_x, float pos_y, float scale) {
    
    float text_scale = getWidth(2) * scale;

    //角度指示器2
    TextEnd(getWidth(pos_x), getHeight(pos_y), "ᐖ", osdicons, text_scale);   
}



void draw_Alarm(int SenorsPresent, int SenorsEnabled, int SenorsHealth, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    VGfloat width_value = TextWidth("00", myfont, text_scale);
    VGfloat height_text = TextHeight(myfont, text_scale) + getHeight(1) * scale;
    
    int row = 0;
    
    /* 
     * Red
     */
    Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A)); 
    Stroke(255, 20, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));

    
    /*
    if (SenorsHealth & 0b00000000000000000000000001) == 0) {
        sprintf(buffer, "%d", SenorsHealth);
        TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);  
    */



    if (ALARM_1) {
        if (((SenorsEnabled & 0b00000000000000000000000001) == 1) && ((SenorsHealth & 0b00000000000000000000000001) == 0)) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "陀 螺 仪", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "3D GYRO", myfont, text_scale);
            }
            
            row += 1;
        }
    }
    

    if (ALARM_2) {
        if (((SenorsEnabled & 0b00000000000000000000000010) >> 1 == 1) && ((SenorsHealth & 0b00000000000000000000000010)) >> 1 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "加 速 度 计", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "3D ACCEL", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_3) {
        if (((SenorsEnabled & 0b00000000000000000000000100) >> 2 == 1) && ((SenorsHealth & 0b00000000000000000000000100)) >> 2 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "磁 罗 盘", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "3D MAG", myfont, text_scale);
            }
            
            row += 1;
        }   
    }


    if (ALARM_4) {
        if (((SenorsEnabled & 0b00000000000000000000001000) >> 3 == 1) && ((SenorsHealth & 0b00000000000000000000001000)) >> 3 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "空 速 计 静 压", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "ABSOLUTE PRESSURE", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_5) {
        if (((SenorsEnabled & 0b00000000000000000000010000) >> 4 == 1) && ((SenorsHealth & 0b00000000000000000000010000)) >> 4 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "空 速 计 动 压", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "DIFFERENTIAL PRESSURE", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_6) {
        if (((SenorsEnabled & 0b00000000000000000000100000) >> 5 == 1) && ((SenorsHealth & 0b00000000000000000000100000)) >> 5 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "全 球 定 位 系 统 GPS", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "GPS", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_7) {
        if (((SenorsEnabled & 0b00000000000000000001000000) >> 6 == 1) && ((SenorsHealth & 0b00000000000000000001000000)) >> 6 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "光 流 传 感 器", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "OPTICAL FLOW", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_8) {
        if (((SenorsEnabled & 0b00000000000000000010000000) >> 7 == 1) && ((SenorsHealth & 0b00000000000000000010000000)) >> 7 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "视 觉 传 感 器", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "VISION POSITION", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_9) {
        if (((SenorsEnabled & 0b00000000000000000100000000) >> 8 == 1) && ((SenorsHealth & 0b00000000000000000100000000)) >> 8 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "激 光 传 感 器", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "LASER POSITION", myfont, text_scale);
            }
        
            row += 1;
        }   
    }


    if (ALARM_10) {
        if (((SenorsEnabled & 0b00000000000000001000000000) >> 9 == 1) && ((SenorsHealth & 0b00000000000000001000000000)) >> 9 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "External Ground Truth", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "External Ground Truth", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_11) {
        if (((SenorsEnabled & 0b00000000000000010000000000) >> 10 == 1) && ((SenorsHealth & 0b00000000000000010000000000)) >> 10 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "角 速 率 控 制", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "ANGULAR RATE CONTROL", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_12) {
        if (((SenorsEnabled & 0b00000000000000100000000000) >> 11 == 1) && ((SenorsHealth & 0b00000000000000100000000000)) >> 11 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "姿 态 稳 定", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "ATTITUDE STABILIZATION", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_13) {
        if (((SenorsEnabled & 0b00000000000001000000000000) >> 12 == 1) && ((SenorsHealth & 0b00000000000001000000000000)) >> 12 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text*row, "偏 航 位 置", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text*row, "YAW POSITION", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_14) {
        if (((SenorsEnabled & 0b00000000000010000000000000) >> 13 == 1) && ((SenorsHealth & 0b00000000000010000000000000)) >> 13 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "Z 轴 高 度 控 制", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "Z ALTITUDE CONTROL", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_15) {
        if (((SenorsEnabled & 0b00000000000100000000000000) >> 14 == 1) && ((SenorsHealth & 0b00000000000100000000000000)) >> 14 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "X/Y 轴 位 置 控 制", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "XY POSITION CONTROL", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_16) {
        if (((SenorsEnabled & 0b00000000001000000000000000) >> 15 == 1) && ((SenorsHealth & 0b00000000001000000000000000)) >> 15 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "电 机 输 出", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "MOTOR OUTPUTS", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_17) {
        if (((SenorsEnabled & 0b00000000010000000000000000) >> 16 == 1) && ((SenorsHealth & 0b00000000010000000000000000)) >> 16 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "接 收 机", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "RC RECEIVER ", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_18) {
        if (((SenorsEnabled & 0b00000000100000000000000000) >> 17 == 1) && ((SenorsHealth & 0b00000000100000000000000000)) >> 17 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "2 号 陀 螺 仪", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "3D GYRO2", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_19) {
        if (((SenorsEnabled & 0b00000001000000000000000000) >> 18 == 1) && ((SenorsHealth & 0b00000001000000000000000000)) >> 18 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "2 号 加 速 度 计", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "3D ACCEL2", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_20) {
        if (((SenorsEnabled & 0b00000010000000000000000000) >> 19 == 1) && ((SenorsHealth & 0b00000010000000000000000000)) >> 19 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "2 号 磁 罗 盘", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "3D MAG2", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_21) {
        if (((SenorsEnabled & 0b00000100000000000000000000) >> 20 == 1) && ((SenorsHealth & 0b00000100000000000000000000)) >> 20 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "地 理 围 栏", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "GEOFENCE", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_22) {
        if (((SenorsEnabled & 0b00001000000000000000000000) >> 21 == 1) && ((SenorsHealth & 0b00001000000000000000000000)) >> 21 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "航 姿 参 考 系 统 AHRS", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "AHRS", myfont, text_scale);
            }

            row += 1;
        }   
    }


    if (ALARM_23) {
        if (((SenorsEnabled & 0b00010000000000000000000000) >> 22 == 1) && ((SenorsHealth & 0b00010000000000000000000000)) >> 22 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "地 形", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "TERRAIN", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_24) {
        if (((SenorsEnabled & 0b00100000000000000000000000) >> 23 == 1) && ((SenorsHealth & 0b00100000000000000000000000)) >> 23 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "电 机 反 转", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "REVERSE MOTOR", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_25) {
        if (((SenorsEnabled & 0b01000000000000000000000000) >> 24 == 1) && ((SenorsHealth & 0b01000000000000000000000000)) >> 24 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "日 志 记 录", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "LOGGING", myfont, text_scale);
            }

            row += 1;
        }
    }


    if (ALARM_26) {
        if (((SenorsEnabled & 0b10000000000000000000000000) >> 25 == 1) && ((SenorsHealth & 0b10000000000000000000000000)) >> 25 == 0) {
            if (CHINESE) {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "电 池 传 感 器", myfont, text_scale);
            } else {
                TextEnd(getWidth(pos_x), getHeight(pos_y) + height_text * row, "BATTERY", myfont, text_scale);
            }

            row += 1;
        }
    }


    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
}



void draw_home_radar(float abs_heading, float craft_heading, int homedst, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    int dstrng;
    
    /*
    int bearing;
    int position_algle;
    int dst;
    float dstlat;
    float dstlon;
    float scaleLongDown = cos(abs(homelat)*0.0174532925);
    dstlat = (homelat - lat)*111319.5;
    dstlon = (homelon - lon)*111319.5*scaleLongDown;
    bearing = atan2(dstlat, -dstlon) * 57.295775;
    position_algle = bearing+180;
    if (position_algle < 0) position_algle += 360;
    if (position_algle >= 360) position_algle -= 360;
    dst=sqrt(dstlat*dstlat+dstlon*dstlon);
    */
    
    if (homedst <= 500) {
        dstrng = 500;
    } else if (homedst > 500 && homedst<=1000) {
        dstrng = 1000;
    } else if (homedst > 1000 && homedst<=2000) {
        dstrng = 2000;
    } else if (homedst > 1000 && homedst<=4000) {
        dstrng = 4000;
    } else if (homedst > 4000 && homedst<=8000) {
        dstrng = 8000;
    } else if (homedst > 8000 && homedst<=16000) {
        dstrng = 16000;
    } else if (homedst > 16000 && homedst<=32000) {
        dstrng = 32000;
    } else { 
        dstrng = homedst; 
    }


    pos_x = getWidth(pos_x / 2);
    pos_y = getHeight(pos_y / 2);

    pos_x = pos_x / dstrng * homedst * sin(abs_heading * 0.0174532925) + width / 2;
    pos_y = pos_y / dstrng * homedst * cos(abs_heading * 0.0174532925) + height / 2;


    float x[5] = {
         pos_x - getWidth(1.25) * scale, 
         pos_x, 
         pos_x + getWidth(1.25) * scale,
         pos_x, 
         pos_x - getWidth(1.25) * scale
    };

    float y[5] = {
        pos_y - getWidth(1) * scale,
        getWidth(1) * scale + pos_y,
        pos_y - getWidth(1) * scale,
        getWidth(0.25) * scale + pos_y,
        pos_y - getWidth(1) * scale
    };


    rotatePoints(x, y, craft_heading, 5, pos_x + getWidth(1.25) * scale, pos_y + getWidth(1.25) * scale);

    Fill(255, 255, 0, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
    Stroke(255, 255, 0, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));

    Polygon(x, y, 5);
    Polyline(x, y, 5);
    
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
}




void draw_throttle(uint16_t throttle, uint16_t throttle_target, int armed, float pos_x, float pos_y, float scale) {

    float text_scale = getHeight(2) * scale;
    float width_element = getWidth(0.15) * scale;
    float height_element = getWidth(3) * scale;
    
    Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
    Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);

    sprintf(buffer, "%d%%", throttle);
    Text(getWidth(pos_x) + getWidth(0.2), getHeight(pos_y), buffer, myfont, text_scale);

    if (THROTTLE_GAUGE) {
        /*
         * Save the current matrix so it can be reset later
         */
        VGfloat savedMatrix[9];
        vgGetMatrix(savedMatrix);


        /*
         * Move the reference point to center of gauge (now 0,0)
         */
        Translate(getWidth(pos_x), getHeight(pos_y + 3));


        Stroke(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        //Circle(getWidth(pos_x + 3), getHeight(pos_y + 3), width_gauge);
        CircleOutline(0, 0, height_element * 2.2);


        /* 
         * 0 tick
         */
        Rotate(135);
        Rect(0, height_element * 0.85, width_element, height_element * 0.2);
        vgLoadMatrix(savedMatrix);


        /*
         * Throttle target tick
         * 
         * Green
         */
        Translate(getWidth(pos_x), getHeight(pos_y + 3));
        Rotate(135);
        Rotate((throttle_target * 2.35) * -1);
        Stroke(COLOR_GOOD_R, COLOR_GOOD_G, COLOR_GOOD_B, COLOR_GOOD_A);
        Fill(COLOR_GOOD_R, COLOR_GOOD_G, COLOR_GOOD_B, COLOR_GOOD_A);
        Rect(0, height_element * 0.85, width_element, height_element * 0.2);
        vgLoadMatrix(savedMatrix);



        /*
         * 100 tick
         * 
         * Red
         */
        Translate(getWidth(pos_x), getHeight(pos_y + 3));
        Rotate(270);
        Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        Rect(0, height_element * 0.85, width_element, height_element * 0.2);
        vgLoadMatrix(savedMatrix);



        /*
         * Set initial rotation at 135 degrees for 0 throttle
         * 
         * 2.35 rotation so that 100 percent throttle = 235 degrees
         */
        Translate(getWidth(pos_x), getHeight(pos_y + 3));
        Rotate(135);
        Rotate((throttle * 2.35) * -1);


        if (armed == 1) {
            /* 
             * Red
             */
            Stroke(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
            Fill(COLOR_WARNING_R, COLOR_WARNING_G, COLOR_WARNING_B, COLOR_WARNING_A);
        } else {
            /* 
             * Normal color
             */
            Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
            Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
        }


        if ((throttle > throttle_target - 2) && (throttle < throttle_target + 2)) {
            /* 
             * Green
             */
            Stroke(COLOR_GOOD_R, COLOR_GOOD_G, COLOR_GOOD_B, COLOR_GOOD_A);
            Fill(COLOR_GOOD_R, COLOR_GOOD_G, COLOR_GOOD_B, COLOR_GOOD_A);
        }
        

        /*
         * Draw needle
         */
        Rect(0, 0, width_element, height_element*.8);

        /*
         * Reset matrix so coordinate system is good for next draw
         */
        vgLoadMatrix(savedMatrix);
    }
}



void draw_throttle_V2(uint16_t throttle, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;

    VGfloat width_value = TextWidth(".", myfont, text_scale) * 1.3;
    VGfloat width_value_1 = TextWidth("0000", myfont, text_scale) * 1.1;
    
    int j = throttle;
    
    const char* c;

    if (THROTTLE_V2_COMPLEX) {

        if (j <= 2 ) { 
            c = "᐀";
        } else if (j > 2 && j <= 5 ) {
            c = "ᐁ";
        } else if (j > 5 && j <= 10 ) {
            c = "ᐂ";
        } else if (j > 10 && j <= 15 ) {
            c = "ᐃ";
        } else if (j > 15 && j <= 20 ) {
            c = "ᐄ";
        } else if (j > 20 && j <= 25 ) {
            c = "ᐅ";
        } else if (j > 25 && j <= 30 ) {
            c = "ᐆ";
        } else if (j > 30 && j <= 35 ) {
            c = "ᐇ";
        } else if (j > 35 && j <= 40 ) {
            c = "ᐈ";
        } else if (j > 40 && j <= 45 ) {
            c = "ᐉ";
        } else if (j > 45 && j <= 50 ) {
            c = "ᐊ";
        } else if (j > 50 && j <= 55 ) {
            c = "ᐋ";
        } else if (j > 55 && j <= 60 ) {
            c = "ᐌ";
        } else if (j > 60 && j <= 65 ) {
            c = "ᐍ";
        } else if (j > 65 && j <= 70 ) {
            c = "ᐎ";
        } else if (j > 70 && j <= 75 ) {
            c = "ᐏ";
        } else if (j > 75 && j <= 80 ) {
            c = "ᐐ";
        } else if (j > 80 && j <= 85 ) {
            c = "ᐑ";
        } else if (j > 85 && j <= 90 ) {
            c = "ᐒ";
        } else if (j > 90 && j <= 95 ) {
            c = "ᐓ";
        } else if (j == 99 ) { 
            c = "ᐔ";
        } else {
            c = "ᐔ";
        }

        /* 
         * Green
         */
        Fill(0, 190, 90, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(0, 190, 90, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
        TextEnd(getWidth(pos_x) + width_value, getHeight(pos_y) - getHeight(2.4) * scale, "ᐗ", osdicons, text_scale);


        /* 
         * Orange
         */
        Fill(255, 165, 0, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(255, 165, 0, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
        TextEnd(getWidth(pos_x) + width_value, getHeight(pos_y) - getHeight(2.4) * scale, "ᐘ", osdicons, text_scale); 
        

        /* 
         * Red
         */
        Fill(255, 20, 20, getOpacity(COLOR_R, COLOR_G, COLOR_B, COLOR_A));
        Stroke(255, 20, 20, getOpacity(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A));
        TextEnd(getWidth(pos_x) + width_value, getHeight(pos_y) - getHeight(2.4) * scale, "ᐙ", osdicons, text_scale); 



        Fill(COLOR_R, COLOR_G, COLOR_B, COLOR_A);
        Stroke(OUTLINECOLOR_R, OUTLINECOLOR_G, OUTLINECOLOR_B, OUTLINECOLOR_A);
        sprintf(buffer, "%d", throttle);
        TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
        TextEnd(getWidth(pos_x) + width_value, getHeight(pos_y) - getHeight(2.4) * scale, c, osdicons, text_scale);


    } else {
        if (j <= 10 ) {
            c = "ᎋ";
        } else if (j > 10 && j <= 20 ) {
            c = "ᎌ";
        } else if (j > 20 && j <= 30 ) {
            c = "ᎍ";
        } else if (j > 30 && j <= 40 ) {
            c = "ᎎ";
        } else if (j > 40 && j <= 50 ) {
            c = "ᎏ";
        } else if (j > 50 && j <= 60 ) {
            c = "᎐";
        } else if (j > 60 && j <= 70 ) {
            c = "᎑";
        } else if (j > 70 && j <= 80 ) {
            c = "᎒";
        } else {
            c = "᎓";
        }

        sprintf(buffer, "%d", throttle);

        TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
        TextEnd(getWidth(pos_x) - width_value_1, getHeight(pos_y), c, osdicons, text_scale);
    }
}
