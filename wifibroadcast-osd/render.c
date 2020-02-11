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
#include "osdconfig.h"

#define TO_FEET 3.28084
#define TO_MPH 0.621371
#define CELL_WARNING_PCT1 (CELL_WARNING1-CELL_MIN)/(CELL_MAX-CELL_MIN)*100
#define CELL_WARNING_PCT2 (CELL_WARNING2-CELL_MIN)/(CELL_MAX-CELL_MIN)*100

long long amps_ts; 
long long dist_ts; 
long long time_ts;
float total_amps; 
double total_dist; 
float total_time;

int width, height;
float scale_factor_font;
bool setting_home;
bool home_set;
float home_lat;
float home_lon;
int home_counter;
char buffer[40];
Fontinfo myfont,osdicons;

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
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}


int getWidth(float pos_x_percent) {
    return (width * 0.01f * pos_x_percent);
}


int getHeight(float pos_y_percent) {
    return (height * 0.01f * pos_y_percent);
}


float getOpacity(int r, int g, int b, float o) {
    if (o<0.5) o = o*2;
    return o;
}


void setfillstroke() {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    StrokeWidth(OUTLINEWIDTH);
}


void render_init() {
    char filename[100] = "/boot/osdfonts/";
    InitShapes(&width, &height);

    strcat(filename, FONT);
    myfont = LoadTTFFile(filename);
    if (!myfont) {
        fputs("ERROR: Failed to load font!", stderr);
        exit(1);
    }

    osdicons = LoadTTFFile("/boot/osdfonts/osdicons.ttf");
    if (!osdicons) {
        fputs("ERROR: Failed to load osdicons.ttf font!", stderr);
        exit(1);
    }
   
    home_counter = 0;
//  vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);

    amps_ts = dist_ts = time_ts = current_ts(); //wowi
}

void loopUpdate(telemetry_data_t_osd *td) {
    // Update fly time. Get time passed since last update
    long time_diff = current_ts() - time_ts;
    time_ts = current_ts();

#if COPTER == true
if ( (td->armed == 1) && (td->ampere > 3) )
    total_time += (float)time_diff/60000;
#else
if (td->speed>0)
   total_time += (float)time_diff/60000;
#endif


    // Update total amps used. Get time passed since last rendering
    time_diff = current_ts() - amps_ts;
    amps_ts = current_ts();
    total_amps = total_amps + td->ampere*(float)time_diff/3600;
}

void render(telemetry_data_t_osd *td, uint8_t cpuload_gnd, uint8_t temp_gnd, uint8_t undervolt, int osdfps) {

    // call loopUpdate to update stuff that should be updated even when particular elements are off (like total curent);
    loopUpdate(td);

    Start(width,height); // render start
    setfillstroke();

    if (td->rx_status_sysair->undervolt == 1) draw_message(0,"Undervoltage on TX","Check wiring/power-supply","Bitrate limited to 1 Mbit",WARNING_POS_X, WARNING_POS_Y, GLOBAL_SCALE);
    if (undervolt == 1) draw_message(0,"Undervoltage on RX","Check wiring/power-supply"," ",WARNING_POS_X, WARNING_POS_Y, GLOBAL_SCALE);

    #if defined(FRSKY)
    //we assume that we have a fix if we get the NS and EW values from frsky protocol
    if (((td->ew == 'E' || td->ew == 'W') && (td->ns == 'N' || td->ns == 'S')) && !home_set){
    setting_home = true;
    } else { //no fix
    setting_home = false;
    home_counter = 0;
    }
    if (setting_home && !home_set){
    //if 20 packages after each other have a fix set home
    if (++home_counter == 20){
        home_set = true;
        home_lat = (td->ns == 'N'? 1:-1) * td->latitude;
        home_lon = (td->ew == 'E'? 1:-1) * td->longitude;
    }
    }
    #endif

    #if defined(MAVLINK) || defined(SMARTPORT)
    // if atleast 2D satfix is reported by flightcontrol
    if (td->fix > 2 && !home_set){
    setting_home = true;
    } else { //no fix
    setting_home = false;
    home_counter = 0;
    }

    if (setting_home && !home_set){
    //if 20 packages after each other have a fix set home
    if (++home_counter == 20){
        home_set = true;
        home_lat = td->latitude;
        home_lon = td->longitude;
        
        //Save GPS home
        float lonlat[2];
            lonlat[0] = 0.0;
            lonlat[1] = 0.0;

        //read
            fptr = fopen("/dev/shm/homepos","rb");
            if(fptr == NULL)
            {
                    printf("No GPS home file found. Load home from Pixhawk\n" );

            home_lat = td->latitude;
                    home_lon = td->longitude;

                    lonlat[0] = home_lon;
                    lonlat[1] = home_lat;
                    //Save data for future
                    //write
                    fptr = fopen("/dev/shm/homepos","wb");
                    if(fptr == NULL)
                    {
                            printf("Cannot create a file to store GPS home position \n" );
                            return 1;
                    }
                    else
                    {
                            printf("Saving GPS home position... \n");
                            fwrite(&lonlat, sizeof(lonlat), 1, fptr);
                            fclose(fptr);
                    }

            }
            else
            {
                    printf("GPS home file exist. Load Home position from it \n");
                    fread(&lonlat, sizeof(lonlat), 1, fptr);
                    fclose(fptr);

                    home_lat = lonlat[1];
                    home_lon = lonlat[0];

                    printf("Lat:%f \n",  home_lat);
                    printf("Lon:%f \n",  home_lon);
            }
    //mod end       
        
    }
    }
    #endif

    #if defined(LTM)
    //LTM makes it easy: If LTM O-frame reports home fix,
    //set home position and use home lat/long from LTM O-frame
    if (td->home_fix == 1){
    home_set = true;
    home_lat = td->ltm_home_latitude;
    home_lon = td->ltm_home_longitude;
    }
    #endif


//    draw_osdinfos(osdfps, 20, 20, 1);


#ifdef UPLINK_RSSI
    draw_uplink_signal(td->rx_status_uplink->adapter[0].current_signal_dbm, td->rx_status_uplink->lost_packet_cnt, td->rx_status_rc->adapter[0].current_signal_dbm, td->rx_status_rc->lost_packet_cnt, UPLINK_RSSI_POS_X, UPLINK_RSSI_POS_Y, UPLINK_RSSI_SCALE * GLOBAL_SCALE);
#endif


#ifdef KBITRATE
    draw_kbitrate(td->rx_status_sysair->cts, td->rx_status->kbitrate, td->rx_status_sysair->bitrate_kbit, td->rx_status->current_air_datarate_kbit, td->rx_status_sysair->bitrate_measured_kbit, td->datarate, td->rx_status_sysair->skipped_fec_cnt, td->rx_status_sysair->injection_fail_cnt,td->rx_status_sysair->injection_time_block, td->armed, KBITRATE_POS_X, KBITRATE_POS_Y, KBITRATE_SCALE * GLOBAL_SCALE, KBITRATE_WARN, KBITRATE_CAUTION, KBITRATE_DECLUTTER);
#endif


#ifdef SYS
    draw_sys(td->rx_status_sysair->cpuload, td->rx_status_sysair->temp, cpuload_gnd, temp_gnd, td->armed, SYS_POS_X, SYS_POS_Y, SYS_SCALE * GLOBAL_SCALE, CPU_LOAD_WARN, CPU_LOAD_CAUTION, CPU_TEMP_WARN, CPU_TEMP_CAUTION, SYS_DECLUTTER);
#endif


#ifdef FLIGHTMODE
    #ifdef MAVLINK
    draw_mode(td->mav_flightmode, td->armed, FLIGHTMODE_POS_X, FLIGHTMODE_POS_Y, FLIGHTMODE_SCALE * GLOBAL_SCALE);
    #endif
    #ifdef VOT
    draw_mode(td->flightmode, td->armed, FLIGHTMODE_POS_X, FLIGHTMODE_POS_Y, FLIGHTMODE_SCALE * GLOBAL_SCALE);
    #endif
    #ifdef LTM
    draw_ltmmode(td->ltm_flightmode, td->armed, td->ltm_failsafe, FLIGHTMODE_POS_X, FLIGHTMODE_POS_Y, FLIGHTMODE_SCALE * GLOBAL_SCALE);
    #endif
#endif


#if defined(RSSI)
    draw_rssi(td->rssi, td->armed, RSSI_POS_X, RSSI_POS_Y, RSSI_SCALE * GLOBAL_SCALE, RSSI_WARN, RSSI_CAUTION, RSSI_DECLUTTER);
#endif


#if defined(CLIMB) && defined(MAVLINK)
    draw_climb(td->mav_climb, CLIMB_POS_X, CLIMB_POS_Y, CLIMB_SCALE * GLOBAL_SCALE);
#endif


#ifdef AIRSPEED
    draw_airspeed((int)td->airspeed, AIRSPEED_POS_X, AIRSPEED_POS_Y, AIRSPEED_SCALE * GLOBAL_SCALE);
#endif

#ifdef GPSSPEED
    draw_gpsspeed((int)td->speed, GPSSPEED_POS_X, GPSSPEED_POS_Y, GPSSPEED_SCALE * GLOBAL_SCALE);
#endif

#ifdef COURSE_OVER_GROUND
    draw_cog((int)td->cog, COURSE_OVER_GROUND_POS_X, COURSE_OVER_GROUND_POS_Y, COURSE_OVER_GROUND_SCALE * GLOBAL_SCALE);
#endif


#ifdef ALTLADDER //by default in osdconfig uses mslalt = false (relative alt should be shown)
        #if REVERSE_ALTITUDES == true
        draw_alt_ladder((int)td->msl_altitude, ALTLADDER_POS_X, 50, ALTLADDER_SCALE * GLOBAL_SCALE, ALTLADDER_WARN, ALTLADDER_CAUTION, ALTLADDER_VSI_TIME, td->mav_climb);
        #else
        draw_alt_ladder((int)td->rel_altitude, ALTLADDER_POS_X, 50, ALTLADDER_SCALE * GLOBAL_SCALE, ALTLADDER_WARN, ALTLADDER_CAUTION, ALTLADDER_VSI_TIME, td->mav_climb);
        #endif  
#endif


#ifdef MSLALT 
        #if REVERSE_ALTITUDES == true
        draw_mslalt(td->rel_altitude, MSLALT_POS_X, MSLALT_POS_Y, MSLALT_SCALE * GLOBAL_SCALE);
        #else
        draw_mslalt(td->msl_altitude, MSLALT_POS_X, MSLALT_POS_Y, MSLALT_SCALE * GLOBAL_SCALE);
        #endif
#endif


#ifdef SPEEDLADDER
    #if IMPERIAL == true
    #if SPEEDLADDER_USEAIRSPEED == true
    draw_speed_ladder((int)td->airspeed*TO_MPH, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE, SPEEDLADDER_TREND_TIME, SPEEDLADDER_LOW_LIMIT, td->vx);
    #else
    draw_speed_ladder((int)td->speed*TO_MPH, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE, SPEEDLADDER_TREND_TIME, SPEEDLADDER_LOW_LIMIT, td->vx);
    #endif
    #else
    #if SPEEDLADDER_USEAIRSPEED == true
    draw_speed_ladder((int)td->airspeed, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE, SPEEDLADDER_TREND_TIME, SPEEDLADDER_LOW_LIMIT, td->vx);
    #else
    draw_speed_ladder((int)td->speed, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE, SPEEDLADDER_TREND_TIME, SPEEDLADDER_LOW_LIMIT, td->vx);
    #endif
    #endif
#endif

#if defined(YAWDISPLAY) && defined(MAVLINK)
    draw_yaw_display(td->vy, YAWDISPLAY_POS_X, YAWDISPLAY_POS_Y, YAWDISPLAY_SCALE * GLOBAL_SCALE, YAWDISPLAY_TREND_TIME);
#endif

#ifdef HOME_ARROW
    #ifdef FRSKY
//  draw_home_arrow((int)course_to((td->ns == 'N'? 1:-1) *td->latitude, (td->ns == 'E'? 1:-1) *td->longitude, home_lat, home_lon), HOME_ARROW_POS_X, HOME_ARROW_POS_Y, HOME_ARROW_SCALE * GLOBAL_SCALE);
    #else
    #if HOME_ARROW_USECOG == true
    draw_home_arrow(course_to(home_lat, home_lon, td->latitude, td->longitude), td->cog, HOME_ARROW_POS_X, HOME_ARROW_POS_Y, HOME_ARROW_SCALE * GLOBAL_SCALE);
    #else
    draw_home_arrow(course_to(home_lat, home_lon, td->latitude, td->longitude), td->heading, HOME_ARROW_POS_X, HOME_ARROW_POS_Y, HOME_ARROW_SCALE * GLOBAL_SCALE);
    #endif
    if(td->heading>=360) td->heading=td->heading-360; // ?
    #endif
#endif


#ifdef COMPASS
    #if COMPASS_USECOG == true
    draw_compass(td->cog, course_to(home_lat, home_lon, td->latitude, td->longitude), 50, COMPASS_POS_Y, COMPASS_SCALE * GLOBAL_SCALE);
    #else
    draw_compass(td->heading, course_to(home_lat, home_lon, td->latitude, td->longitude), 50, COMPASS_POS_Y, COMPASS_SCALE * GLOBAL_SCALE);
    #endif
#endif


#ifdef BATT_STATUS
    draw_batt_status(td->voltage, td->ampere, BATT_STATUS_POS_X, BATT_STATUS_POS_Y, BATT_STATUS_SCALE * GLOBAL_SCALE);
#endif

#ifdef TOTAL_AMPS 
    draw_TOTAL_AMPS(total_amps, TOTAL_AMPS_POS_X, TOTAL_AMPS_POS_Y, TOTAL_AMPS_SCALE * GLOBAL_SCALE);
#endif

#ifdef TOTAL_DIST
    draw_TOTAL_DIST((int)td->speed, TOTAL_DIST_POS_X, TOTAL_DIST_POS_Y, TOTAL_DIST_SCALE * GLOBAL_SCALE);
 #endif

#ifdef TOTAL_TIME
    draw_TOTAL_TIME((float)total_time, TOTAL_TIME_POS_X, TOTAL_TIME_POS_Y, TOTAL_TIME_SCALE * GLOBAL_SCALE);
#endif

#ifdef POSITION
    #if defined(FRSKY)
    draw_position((td->ns == 'N'? 1:-1) * td->latitude, (td->ew == 'E'? 1:-1) * td->longitude, POSITION_POS_X, POSITION_POS_Y, POSITION_SCALE * GLOBAL_SCALE);
    #endif
    draw_position(td->latitude, td->longitude, POSITION_POS_X, POSITION_POS_Y, POSITION_SCALE * GLOBAL_SCALE);
#endif


#ifdef DISTANCE
    #ifdef FRSKY
    draw_home_distance((int)distance_between(home_lat, home_lon, (td->ns == 'N'? 1:-1) *td->latitude, (td->ns == 'E'? 1:-1) *td->longitude), home_set, DISTANCE_POS_X, DISTANCE_POS_Y, DISTANCE_SCALE * GLOBAL_SCALE);
    #elif defined(LTM) || defined(MAVLINK) || defined(SMARTPORT)
        draw_home_distance((int)distance_between(home_lat, home_lon, td->latitude, td->longitude), home_set, DISTANCE_POS_X, DISTANCE_POS_Y, DISTANCE_SCALE * GLOBAL_SCALE);
    #elif defined(VOT)
        draw_home_distance((int)td->distance, home_set, DISTANCE_POS_X, DISTANCE_POS_Y, DISTANCE_SCALE * GLOBAL_SCALE);
    #endif
#endif


#ifdef DOWNLINK_RSSI
    int i;
    int best_dbm = -1000;
    int ac = td->rx_status->wifi_adapter_cnt;

    no_signal=true;
    for(i=0; i<ac; ++i) { // find out which card has best signal (and if atleast one card has a signal)
    if (td->rx_status->adapter[i].signal_good == 1) {
            if (best_dbm < td->rx_status->adapter[i].current_signal_dbm) best_dbm = td->rx_status->adapter[i].current_signal_dbm;
    }
    if (td->rx_status->adapter[i].signal_good == 1) no_signal=false;
    }

    draw_total_signal(best_dbm, td->rx_status->received_block_cnt, td->rx_status->damaged_block_cnt, td->rx_status->lost_packet_cnt, td->rx_status->received_packet_cnt, td->rx_status->lost_per_block_cnt, DOWNLINK_RSSI_POS_X, DOWNLINK_RSSI_POS_Y, DOWNLINK_RSSI_SCALE * GLOBAL_SCALE);

    #ifdef DOWNLINK_RSSI_DETAILED
    for(i=0; i<ac; ++i) {
        draw_card_signal(td->rx_status->adapter[i].current_signal_dbm, td->rx_status->adapter[i].signal_good, i, ac, td->rx_status->tx_restart_cnt, td->rx_status->adapter[i].received_packet_cnt, td->rx_status->adapter[i].wrong_crc_cnt, td->rx_status->adapter[i].type, td->rx_status->received_packet_cnt, td->rx_status->lost_packet_cnt, DOWNLINK_RSSI_DETAILED_POS_X, DOWNLINK_RSSI_DETAILED_POS_Y, DOWNLINK_RSSI_DETAILED_SCALE * GLOBAL_SCALE);
    }
    #endif
#endif


#ifdef SAT
    #if defined(FRSKY)
    //we assume that we have a fix if we get the NS and EW values from frsky protocol
    if ((td->ew == 'E' || td->ew == 'W') && (td->ns == 'N' || td->ns == 'S')){
    draw_sat(0, 2, SAT_POS_X, SAT_POS_Y, SAT_SCALE * GLOBAL_SCALE);
    } else { //no fix
    draw_sat(0, 0, SAT_POS_X, SAT_POS_Y, SAT_SCALE * GLOBAL_SCALE);
    }
    #elif defined(MAVLINK) || defined(SMARTPORT) || defined(LTM) || defined(VOT)
    draw_sat(td->sats, td->fix, td->hdop, (int)td->armed, SAT_POS_X, SAT_POS_Y, SAT_SCALE * GLOBAL_SCALE, SAT_HDOP_WARN, SAT_HDOP_CAUTION, SAT_DECLUTTER);
    #endif
#endif


#ifdef BATT_GAUGE
    draw_batt_gauge(((td->voltage/CELLS)-CELL_MIN)/(CELL_MAX-CELL_MIN)*100, BATT_GAUGE_POS_X, BATT_GAUGE_POS_Y, BATT_GAUGE_SCALE * GLOBAL_SCALE);
#endif


#ifdef HOME_RADAR 
    #if HOME_RADAR_USECOG == true
    draw_home_radar(course_to(home_lat, home_lon, td->latitude, td->longitude), td->cog, (int)distance_between(home_lat, home_lon, td->latitude, td->longitude), HOME_RADAR_POS_X, HOME_RADAR_POS_Y, HOME_RADAR_SCALE * GLOBAL_SCALE);
    #else
    draw_home_radar(course_to(home_lat, home_lon, td->latitude, td->longitude), td->heading, (int)distance_between(home_lat, home_lon, td->latitude, td->longitude), HOME_RADAR_POS_X, HOME_RADAR_POS_Y, HOME_RADAR_SCALE * GLOBAL_SCALE);  
    #endif
#endif

#if defined(HDOP) && defined(MAVLINK)
 //   draw_Hdop(td->hdop, HDOP_POS_X, HDOP_POS_Y, HDOP_SCALE * GLOBAL_SCALE);
#endif

#if defined(THROTTLE_V2) && defined(MAVLINK) 
   draw_throttle_V2(td->throttle, THROTTLE_V2_POS_X, THROTTLE_V2_POS_Y, THROTTLE_V2_SCALE * GLOBAL_SCALE);
#endif

#if defined(THROTTLE) && defined(MAVLINK)
    draw_throttle((int)td->throttle, THROTTLE_TARGET, (int)td->armed, THROTTLE_POS_X, THROTTLE_POS_Y, THROTTLE_SCALE * GLOBAL_SCALE);
#endif

#if defined(MISSION) && defined(MAVLINK) 
  draw_Mission(td->mission_current_seq , MISSION_POS_X, MISSION_POS_Y, MISSION_SCALE * GLOBAL_SCALE);
#endif

#ifdef ANGLE2
  draw_Angle2(ANGLE2_POS_X, ANGLE2_POS_Y, ANGLE2_SCALE * GLOBAL_SCALE);
#endif

#if defined(ALARM) && defined(MAVLINK) 
    draw_Alarm(td->SP, td->SE, td->SH, ALARM_POS_X, ALARM_POS_Y, ALARM_SCALE * GLOBAL_SCALE);
#endif

#ifdef RPA  //roll and pitch angle
    draw_RPA(RPA_INVERT_ROLL * td->roll, RPA_INVERT_PITCH * td->pitch, RPA_POS_X, RPA_POS_Y, RPA_SCALE * GLOBAL_SCALE);
#endif

#ifdef AHI
    #if defined(FRSKY) || defined(SMARTPORT)
    float x_val, y_val, z_val;
    x_val = td->x;
    y_val = td->y;
    z_val = td->z;
    #if AHI_SWAP_ROLL_AND_PITCH == true
    draw_ahi(AHI_INVERT_ROLL * TO_DEG * (atan(y_val / sqrt((x_val*x_val) + (z_val*z_val)))), 
    AHI_INVERT_PITCH * TO_DEG * (atan(x_val / sqrt((y_val*y_val)+(z_val*z_val)))), AHI_SCALE * GLOBAL_SCALE);
    #else
    draw_ahi(AHI_INVERT_ROLL * TO_DEG * (atan(x_val / sqrt((y_val*y_val) + (z_val*z_val)))), 
    AHI_INVERT_PITCH * TO_DEG * (atan(y_val / sqrt((x_val*x_val)+(z_val*z_val)))), AHI_SCALE * GLOBAL_SCALE);
    #endif // AHI_SWAP_ROLL_AND_PITCH

    #elif defined(LTM) || defined(VOT)
    draw_ahi(AHI_INVERT_ROLL * td->roll, AHI_INVERT_PITCH * td->pitch, AHI_SCALE * GLOBAL_SCALE);
    #elif defined(MAVLINK)	
	#if REVERSE_ALTITUDES == true
        draw_ahi_mav(AHI_INVERT_ROLL * td->roll, AHI_INVERT_PITCH * td->pitch, td->mav_climb, td->vz, td->vx, td->vy, (int)td->speed, 
    (int)td->msl_altitude, AHI_SCALE * GLOBAL_SCALE);
        #else
        draw_ahi_mav(AHI_INVERT_ROLL * td->roll, AHI_INVERT_PITCH * td->pitch, td->mav_climb, td->vz, td->vx, td->vy, (int)td->speed, (int)td->rel_altitude, AHI_SCALE * GLOBAL_SCALE);
        #endif  
#endif //protocol
#endif //AHI

#ifdef ANGLE //bank angle indicator. Must follow AHI 
  draw_Angle(ANGLE_POS_X, ANGLE_POS_Y, ANGLE_SCALE * GLOBAL_SCALE);
#endif

    End(); // Render end (causes everything to be drawn on next vsync)
}

#ifdef VOT

void draw_mode(int mode, int armed, float pos_x, float pos_y, float scale){
    //flight modes Eagletree Vector
    float text_scale = getWidth(2) * scale;

 switch (mode) {
	case 0: sprintf(buffer, "2D"); break;
	case 1: sprintf(buffer, "2DAH"); break;
	case 2: sprintf(buffer, "2DHH"); break;
	case 3: sprintf(buffer, "2DAHH"); break;
	case 4: sprintf(buffer, "LOITER"); break;
	case 5: sprintf(buffer, "3D"); break;
	case 6: sprintf(buffer, "3DHH"); break;
	case 7: sprintf(buffer, "RTH"); break;
	case 8: sprintf(buffer, "LAND"); break;
	case 9: sprintf(buffer, "CART"); break;
	case 10: sprintf(buffer, "CARTLOI"); break;
	case 11: sprintf(buffer, "POLAR"); break;
	case 12: sprintf(buffer, "POLARLOI"); break;
	case 13: sprintf(buffer, "CENTERSTICK"); break;
	case 14: sprintf(buffer, "OFF"); break;
	case 15: sprintf(buffer, "WAYPOINT"); break;
	case 16: sprintf(buffer, "MAX"); break;
	default: sprintf(buffer, "-----"); break;
 }
 TextMid(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}

#endif

#ifdef LTM
void draw_ltmmode(int mode, int armed, int failsafe, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    
    sprintf(buffer, "[-----]");
    if (armed == 0){
      switch (mode) {
       case 0: sprintf(buffer, "[手   动]"); break;
       case 1: sprintf(buffer, "[RATE]"); break;
       case 2: sprintf(buffer, "[自   稳]"); break;
       case 3: sprintf(buffer, "[半 自 稳]"); break;
       case 4: sprintf(buffer, "[特   技]"); break;
       case 5: sprintf(buffer, "[自 稳 1]"); break;
       case 6: sprintf(buffer, "[自 稳 2]"); break;
       case 7: sprintf(buffer, "[自 稳 3]"); break;  
       case 8: sprintf(buffer, "[定   高]"); break;
       case 9: sprintf(buffer, "[GPS 定点]"); break;
       case 10: sprintf(buffer, "[自   动]"); break;
       case 11: sprintf(buffer, "[无   头]"); break;
       case 12: sprintf(buffer, "[绕   圈]"); break;
       case 13: sprintf(buffer, "[返   航]"); break;
       case 14: sprintf(buffer, "[跟   随]"); break;
       case 15: sprintf(buffer, "[降   落]"); break;
       case 16: sprintf(buffer, "[线 性 增 稳]"); break;
       case 17: sprintf(buffer, "[增 稳 定 高]"); break;
       case 18: sprintf(buffer, "[巡   航]"); break;
      }
    }
    else  {
      switch (mode) {
       case 0: sprintf(buffer, "手   动"); break;
       case 1: sprintf(buffer, "RATE"); break;
       case 2: sprintf(buffer, "自   稳"); break;
       case 3: sprintf(buffer, "半 自 稳"); break;
       case 4: sprintf(buffer, "特   技"); break;
       case 5: sprintf(buffer, "自 稳 1"); break;
       case 6: sprintf(buffer, "自 稳 2"); break;
       case 7: sprintf(buffer, "自 稳 3"); break;  
       case 8: sprintf(buffer, "定   高"); break;
       case 9: sprintf(buffer, "GPS 定点"); break;
       case 10: sprintf(buffer, "自   动"); break;
       case 11: sprintf(buffer, "无   头"); break;
       case 12: sprintf(buffer, "绕   圈"); break;
       case 13: sprintf(buffer, "返   航"); break;
       case 14: sprintf(buffer, "跟   随"); break;
       case 15: sprintf(buffer, "降   落"); break;
       case 16: sprintf(buffer, "线 性 增 稳"); break;
       case 17: sprintf(buffer, "增 稳 定 高"); break;
       case 18: sprintf(buffer, "巡   航"); break;
      }
    }  
    
    if (failsafe == 1)  {
       sprintf(buffer, "失 控 保 护");
    }
    
    TextMid(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    
} 
#endif

#ifdef MAVLINK

void draw_mode(int mode, int armed, float pos_x, float pos_y, float scale){
    //autopilot mode, mavlink specific, could be used if mode is in telemetry data of other protocols as well
    float text_scale = getWidth(2) * scale;

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    if (armed == 1){
    switch (mode) {
    #if COPTER == true
    #if CHINESE == true
    case 0: sprintf(buffer, "自   稳"); break;
    case 1: sprintf(buffer, "特   技"); break;
    case 2: sprintf(buffer, "定   高"); break;
    case 3: sprintf(buffer, "自   动"); break;
    case 4: sprintf(buffer, "指   引"); break;
    case 5: sprintf(buffer, "悬   停"); break;
    case 6: sprintf(buffer, "返   航"); break;
    case 7: sprintf(buffer, "绕   圈"); break;
    case 9: sprintf(buffer, "降   落"); break;
    case 11: sprintf(buffer, "漂   移"); break;
    case 13: sprintf(buffer, "运   动"); break;
    case 14: sprintf(buffer, "翻   滚"); break;
    case 15: sprintf(buffer, "自 动 调 参"); break;
    case 16: sprintf(buffer, "定   点"); break;
    case 17: sprintf(buffer, "制   动"); break;
    case 18: sprintf(buffer, "抛   飞"); break;
    case 19: sprintf(buffer, "避   障"); break;
    case 20: sprintf(buffer, "无 GPS 指 引"); break;
    case 255: sprintf(buffer, "-----"); break;
    #else
    case 0: sprintf(buffer, "STAB"); break;
    case 1: sprintf(buffer, "ACRO"); break;
    case 2: sprintf(buffer, "ALTHOLD"); break;
    case 3: sprintf(buffer, "AUTO"); break;
    case 4: sprintf(buffer, "GUIDED"); break;
    case 5: sprintf(buffer, "LOITER"); break;
    case 6: sprintf(buffer, "RTL"); break;
    case 7: sprintf(buffer, "CIRCLE"); break;
    case 9: sprintf(buffer, "LAND"); break;
    case 11: sprintf(buffer, "DRIFT"); break;
    case 13: sprintf(buffer, "SPORT"); break;
    case 14: sprintf(buffer, "FLIP"); break;
    case 15: sprintf(buffer, "AUTOTUNE"); break;
    case 16: sprintf(buffer, "POSHOLD"); break;
    case 17: sprintf(buffer, "BRAKE"); break;
    case 18: sprintf(buffer, "THROW"); break;
    case 19: sprintf(buffer, "AVOIDADSB"); break;
    case 20: sprintf(buffer, "GUIDEDNOGPS"); break;
    case 255: sprintf(buffer, "-----"); break;
    #endif
    #else       
    #if CHINESE == true
      case 0: sprintf(buffer, "手   动"); break;
      case 1: sprintf(buffer, "盘   旋"); break;
      case 2: sprintf(buffer, "自   稳"); break;
      case 3: sprintf(buffer, "教   练"); break;
      case 4: sprintf(buffer, "特   技"); break;
      //case 5: sprintf(buffer, "半 自 稳"); break;
      case 5: sprintf(buffer, "自   稳"); break;
      case 6: sprintf(buffer, "自 稳 定 高"); break;
      case 7: sprintf(buffer, "巡   航"); break;
      case 8: sprintf(buffer, "自 动 调 参"); break;
      case 10: sprintf(buffer, "自   动"); break;
      case 11: sprintf(buffer, "返   航"); break;
      case 12: sprintf(buffer, "定   点"); break;
      //case 15: sprintf(buffer, "抛   飞"); break;
      case 15: sprintf(buffer, "指   引"); break;
      //case 16: sprintf(buffer, "失 控 保 护"); break;
      case 16: sprintf(buffer, "INIT"); break;
      case 255: sprintf(buffer, "-----"); break;
    #else
      case 0: sprintf(buffer, "MAN"); break;
      case 1: sprintf(buffer, "CIRC"); break;
      case 2: sprintf(buffer, "STAB"); break;
      case 3: sprintf(buffer, "TRAI"); break;
      case 4: sprintf(buffer, "ACRO"); break;
      case 5: sprintf(buffer, "FBWA"); break;
      case 6: sprintf(buffer, "FBWB"); break;
      case 7: sprintf(buffer, "CRUZ"); break;
      case 8: sprintf(buffer, "TUNE"); break;
      case 10: sprintf(buffer, "AUTO"); break;
      case 11: sprintf(buffer, "RTL"); break;
      case 12: sprintf(buffer, "LOIT"); break;
      case 13: sprintf(buffer, "TAKE"); break;
      case 15: sprintf(buffer, "GUID"); break;
      case 16: sprintf(buffer, "INIT"); break;
      case 17: sprintf(buffer, "Q-STAB"); break;
      case 18: sprintf(buffer, "Q-HOVR"); break;
      case 19: sprintf(buffer, "Q-LOIT"); break;
      case 20: sprintf(buffer, "Q-LAND"); break;
      case 21: sprintf(buffer, "Q-RTL"); break;
      case 23: sprintf(buffer, "Q-ACRO"); break;
      case 255: sprintf(buffer, "-----"); break;
    #endif
    
    #endif
    default: sprintf(buffer, "-----"); break; // TODO: find out why strange numbers when using zs6bujs telemetry logs, default to something more sensible like "unknown mode"
    }
    } else {
    switch (mode) {
    #if COPTER == true
    #if CHINESE == true
    case 0: sprintf(buffer, "[自   稳]"); break;
    case 1: sprintf(buffer, "[特   技]"); break;
    case 2: sprintf(buffer, "[定   高]"); break;
    case 3: sprintf(buffer, "[自   动]"); break;
    case 4: sprintf(buffer, "[指   引]"); break;
    case 5: sprintf(buffer, "[悬   停]"); break;
    case 6: sprintf(buffer, "[返   航]"); break;
    case 7: sprintf(buffer, "[绕   圈]"); break;
    case 9: sprintf(buffer, "[降   落]"); break;
    case 11: sprintf(buffer, "[漂   移]"); break;
    case 13: sprintf(buffer, "[运   动]"); break;
    case 14: sprintf(buffer, "[翻   滚]"); break;
    case 15: sprintf(buffer, "[自 动 调 参]"); break;
    case 16: sprintf(buffer, "[定   点]"); break;
    case 17: sprintf(buffer, "[制   动]"); break;
    case 18: sprintf(buffer, "[抛   飞]"); break;
    case 19: sprintf(buffer, "[避   障]"); break;
    case 20: sprintf(buffer, "[无 GPS 指 引]"); break;
    case 255: sprintf(buffer, "[-----]"); break;
    #else
    case 0: sprintf(buffer, "[STAB]"); break;
    case 1: sprintf(buffer, "[ACRO]"); break;
    case 2: sprintf(buffer, "[ALTHOLD]"); break;
    case 3: sprintf(buffer, "[AUTO]"); break;
    case 4: sprintf(buffer, "[GUID]"); break;
    case 5: sprintf(buffer, "[LOIT]"); break;
    case 6: sprintf(buffer, "[RTL]"); break;
    case 7: sprintf(buffer, "[CIRCLE]"); break;
    case 9: sprintf(buffer, "[LAND]"); break;
    case 11: sprintf(buffer, "[DRIFT]"); break;
    case 13: sprintf(buffer, "[SPORT]"); break;
    case 14: sprintf(buffer, "[FLIP]"); break;
    case 15: sprintf(buffer, "[AUTOTUNE]"); break;
    case 16: sprintf(buffer, "[POSHOLD]"); break;
    case 17: sprintf(buffer, "[BRAKE]"); break;
    case 18: sprintf(buffer, "[THROW]"); break;
    case 19: sprintf(buffer, "[AVOIDADSB]"); break;
    case 20: sprintf(buffer, "[GUIDEDNOGPS]"); break;
    case 255: sprintf(buffer, "[-----]"); break;
    #endif
    #else
    #if CHINESE == true
      case 0: sprintf(buffer, "[手   动]"); break;
      case 1: sprintf(buffer, "[盘   旋]"); break;
      case 2: sprintf(buffer, "[自   稳]"); break;
      case 3: sprintf(buffer, "[教   练]"); break;
      case 4: sprintf(buffer, "[特   技]"); break;
      //case 5: sprintf(buffer, "[半 自 稳]"); break;
      case 5: sprintf(buffer, "[自   稳]"); break;
      case 6: sprintf(buffer, "[自 稳 定 高]"); break;
      case 7: sprintf(buffer, "[巡   航]"); break;
      case 8: sprintf(buffer, "[自 动 调 参]"); break;
      case 10: sprintf(buffer, "[自   动]"); break;
      case 11: sprintf(buffer, "[返   航]"); break;
      case 12: sprintf(buffer, "[定   点]"); break;
      //case 15: sprintf(buffer, "[抛   飞]"); break;
      case 15: sprintf(buffer, "[指   引]"); break;
      //case 16: sprintf(buffer, "[失 控 保 护]"); break;
      case 16: sprintf(buffer, "[INIT]"); break;
      case 255: sprintf(buffer, "[-----]"); break;
    #else
      case 0: sprintf(buffer, "[MAN]"); break;
      case 1: sprintf(buffer, "[CIRC]"); break;
      case 2: sprintf(buffer, "[STAB]"); break;
      case 3: sprintf(buffer, "[TRAI]"); break;
      case 4: sprintf(buffer, "[ACRO]"); break;
      case 5: sprintf(buffer, "[FBWA]"); break;
      case 6: sprintf(buffer, "[FBWB]"); break;
      case 7: sprintf(buffer, "[CRUZ]"); break;
      case 8: sprintf(buffer, "[TUNE]"); break;
      case 10: sprintf(buffer, "[AUTO]"); break;
      case 11: sprintf(buffer, "[RTL]"); break;
      case 12: sprintf(buffer, "[LOIT]"); break;
      case 13: sprintf(buffer, "[TAKE]"); break;
      case 15: sprintf(buffer, "[GUID]"); break;
      case 16: sprintf(buffer, "[INIT]"); break;
      case 17: sprintf(buffer, "Q-STAB"); break;
      case 18: sprintf(buffer, "Q-HOVR"); break;
      case 19: sprintf(buffer, "Q-LOIT"); break;
      case 20: sprintf(buffer, "Q-LAND"); break;
      case 21: sprintf(buffer, "Q-RTL"); break;
      case 23: sprintf(buffer, "Q-ACRO"); break;
      case 255: sprintf(buffer, "[-----]"); break;
    #endif
    
    #endif
    default: sprintf(buffer, "[-----]"); break; // TODO: find out why strange numbers when using zs6bujs telemetry logs, default to something more sensible like "unknown mode"
    }
    }
    TextMid(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}
#endif


void draw_rssi(int rssi, int armed, float pos_x, float pos_y, float scale, float warn, float caution, float declutter){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale);   

    if (rssi > warn) {
        Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING); 
    } else if (rssi > caution) {
        Stroke(COLOR_CAUTION); //yellow
    Fill(COLOR_CAUTION); 
    } else {    
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    } 

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale * 0.6);
    sprintf(buffer, "%02d", rssi);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    Text(getWidth(pos_x), getHeight(pos_y), "%", myfont, text_scale*0.6);
}



void draw_cog(int cog, float pos_x, float pos_y, float scale){

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("000°", myfont, text_scale);

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale*0.7);

    sprintf(buffer, "%d°", cog);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}



void draw_climb(float climb, float pos_x, float pos_y, float scale){

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("-00.0", myfont, text_scale);

    #if CHINESE == true
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale*0.6);
    #else
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "ﵻ", osdicons, text_scale*0.6);
    #endif

    if (climb > 0.0f) {
    sprintf(buffer, "+%.1f", climb);
    } else {
    sprintf(buffer, "%.1f", climb);
    }
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "m/s", myfont, text_scale*0.6);
}


void draw_airspeed(int airspeed, float pos_x, float pos_y, float scale){

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("100", myfont, text_scale);
    VGfloat width_speedo = TextWidth("", osdicons, text_scale*0.65) + getWidth(0.5)*scale;

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale*0.65);
    TextEnd(getWidth(pos_x)-width_value-width_speedo, getHeight(pos_y), "", osdicons, text_scale*0.65);

    #if IMPERIAL == true
    sprintf(buffer, "%d", airspeed*TO_MPH);
    #else
    sprintf(buffer, "%d", airspeed);
    #endif
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    #if IMPERIAL == true
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "mph", myfont, text_scale*0.6);
    #else
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "km/h", myfont, text_scale*0.6);
    #endif
}


void draw_gpsspeed(int gpsspeed, float pos_x, float pos_y, float scale){
    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("100", myfont, text_scale);
    #if CHINESE == true
    VGfloat width_speedo = TextWidth("", osdicons, text_scale*0.65);
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale*0.65);
    TextEnd(getWidth(pos_x)-width_value-width_speedo, getHeight(pos_y), "", osdicons, text_scale*0.7);
    #else
    VGfloat width_speedo = TextWidth("ﵵ", osdicons, text_scale*0.65);
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y),"ﵵ", osdicons, text_scale*0.65);
    TextEnd(getWidth(pos_x)-width_value-width_speedo, getHeight(pos_y), "ﵷ", osdicons, text_scale*0.7);
    #endif

    #if IMPERIAL == true
    sprintf(buffer, "%d", gpsspeed*TO_MPH);
    #else
    sprintf(buffer, "%d", gpsspeed);
    #endif
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    #if IMPERIAL == true
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "mph", myfont, text_scale*0.6);
    #else
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "km/h", myfont, text_scale*0.6);
    #endif
}


void draw_uplink_signal(int8_t uplink_signal, int uplink_lostpackets, int8_t rc_signal, int rc_lostpackets, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale*0.6)+getHeight(0.3)*scale;
    VGfloat width_value = TextWidth("-00", myfont, text_scale);
    VGfloat width_symbol = TextWidth(" ", osdicons, text_scale*0.7);

    StrokeWidth(OUTLINEWIDTH);

    if ((uplink_signal < -125) && (rc_signal < -125)) { // both no signal, display red dashes
    Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING);
    sprintf(buffer, "-- ");
    } else if (rc_signal < -125) { // only r/c no signal, so display uplink signal
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    sprintf(buffer, "%02d", uplink_signal);
    } else { // if both have signal, display r/c signal
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    sprintf(buffer, "%02d", rc_signal);
    }

    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    Text(getWidth(pos_x), getHeight(pos_y), "dBm", myfont, text_scale*0.6);

    sprintf(buffer, "%d/%d", rc_lostpackets, uplink_lostpackets);
    Text(getWidth(pos_x)-width_value-width_symbol, getHeight(pos_y)-height_text, buffer, myfont, text_scale*0.6);

    TextEnd(getWidth(pos_x)-width_value - getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.7);
}



void draw_kbitrate(int cts, int kbitrate, uint16_t kbitrate_tx, uint16_t current_air_datarate_kbit, uint16_t kbitrate_measured_tx, double hw_datarate_mbit, uint32_t fecs_skipped, uint32_t injection_failed, long long injection_time,int armed, float pos_x, float pos_y, float scale, float mbit_warn, float mbit_caution, float declutter){
    
    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    float text_scale = getWidth(2) * scale;
    VGfloat height_text_small = TextHeight(myfont, text_scale*0.6)+getHeight(0.3)*scale;
    VGfloat width_value = TextWidth("10.0", myfont, text_scale);
    #if CHINESE == true
    VGfloat width_symbol = TextWidth("", osdicons, text_scale*0.8);
    #else
    VGfloat width_symbol = TextWidth("ﵴ", osdicons, text_scale*0.8);
    #endif

    float mbit = (float)kbitrate / 1000;
    float mbit_measured = (float)kbitrate_measured_tx / 1000;
    float mbit_tx = (float)kbitrate_tx / 1000;
    float ms = (float)injection_time / 1000;
    float air_rx_mbit = (float)current_air_datarate_kbit / 1000;
    

    if (air_rx_mbit / hw_datarate_mbit >= 0.75) {
        Stroke(COLOR_WARNING); //red
        Fill(COLOR_WARNING);
    } else if (air_rx_mbit > mbit_measured) {
        Stroke(COLOR_CAUTION); //yellow
        Fill(COLOR_CAUTION);
    } else {
        Fill(COLOR);
        Stroke(OUTLINECOLOR);
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

    Text(getWidth(pos_x)-width_value-width_symbol, getHeight(pos_y)-height_text_small, buffer, myfont, text_scale*0.6);
    Fill(COLOR);
    Stroke(OUTLINECOLOR);

//this is the reason for constant blinking of the cam icon

    if (fecs_skipped > fecs_skipped_last) {
    Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING);
    } else {
        Fill(COLOR);
        Stroke(OUTLINECOLOR);
    if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    }
    fecs_skipped_last = fecs_skipped;

    #if CHINESE == true
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale * 0.8);
    #else
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y),"ﵴ", osdicons, text_scale * 0.8);
    #endif
    if (mbit_measured != 0 && mbit > mbit_measured*mbit_warn) {
        Stroke(COLOR_WARNING); //red
        Fill(COLOR_WARNING); 
    } else if (mbit_measured != 0 && mbit > mbit_measured*mbit_caution) {
        Stroke(COLOR_CAUTION); //yellow
        Fill(COLOR_CAUTION); 
    } else {    
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    } 

    sprintf(buffer, "%.1f/", mbit);
    TextEnd(getWidth(pos_x) + 20, getHeight(pos_y), buffer, myfont, text_scale);

    sprintf(buffer, "%.1f", mbit_tx);
    Text(getWidth(pos_x) + 20, getHeight(pos_y), buffer, myfont, text_scale*0.6);

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    

//    sprintf(buffer, "%.1f", ms);
//    TextEnd(getWidth(pos_x)-width_value-width_symbol+width_value_ms, getHeight(pos_y)-height_text-height_text_small, buffer, myfont, text_scale*0.6);
//    sprintf(buffer, "ms");
//    Text(getWidth(pos_x)-width_value-width_symbol+width_value_ms, getHeight(pos_y)-height_text-height_text_small, buffer, myfont, text_scale*0.4);

    sprintf(buffer, "%d/%d",injection_failed,fecs_skipped);
    Text(getWidth(pos_x)-width_value-width_symbol, getHeight(pos_y)-height_text_small-height_text_small, buffer, myfont, text_scale*0.6);
}



void draw_sys(uint8_t cpuload_air, uint8_t temp_air, uint8_t cpuload_gnd, uint8_t temp_gnd, int armed, float pos_x, float pos_y, float scale, float load_warn, float load_caution, float temp_warn, float temp_caution, float declutter) {

    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale) + getWidth(0.5)*scale;
    VGfloat width_label = TextWidth("%", myfont, text_scale*0.6) + getWidth(0.5)*scale;
    VGfloat width_ag = TextWidth("A", osdicons, text_scale*0.4) - getWidth(0.3)*scale;

    

    if (cpuload_air > load_warn) {
        Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING); 
    } else if (cpuload_air > load_caution) {
        Stroke(COLOR_CAUTION); //yellow
    Fill(COLOR_CAUTION); 
    } else {    
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    }
    TextEnd(getWidth(pos_x)-width_value-width_ag, getHeight(pos_y), "", osdicons, text_scale*0.7);
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "A", myfont, text_scale*0.4); 

    sprintf(buffer, "%d", cpuload_air);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    sprintf(buffer, "%%");
    Text(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale*0.6);

    if (temp_air > temp_warn) {
        Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING); 
    } else if (temp_air > temp_caution) {
        Stroke(COLOR_CAUTION); //yellow
    Fill(COLOR_CAUTION); 
    } else {    
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    }
    sprintf(buffer, "%d°", temp_air);
    TextEnd(getWidth(pos_x)+width_value+width_label+getWidth(0.7), getHeight(pos_y), buffer, myfont, text_scale);
   
    TextEnd(getWidth(pos_x)-width_value-width_ag, getHeight(pos_y), "", osdicons, text_scale*0.7);
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "A", myfont, text_scale*0.4);

    if (cpuload_gnd > load_warn) {
        Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING); 
    } else if (cpuload_gnd > load_caution) {
        Stroke(COLOR_CAUTION); //yellow
    Fill(COLOR_CAUTION); 
    } else {    
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    }
    TextEnd(getWidth(pos_x)-width_value-width_ag, getHeight(pos_y)-height_text, "", osdicons, text_scale*0.7);
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y)-height_text, "G", myfont, text_scale*0.4);

    sprintf(buffer, "%d", cpuload_gnd);
    TextEnd(getWidth(pos_x), getHeight(pos_y)-height_text, buffer, myfont, text_scale);

    Text(getWidth(pos_x), getHeight(pos_y)-height_text, "%", myfont, text_scale*0.6);

    if (temp_gnd > temp_warn) {
        Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING); 
    } else if (temp_gnd > temp_caution) {
        Stroke(COLOR_CAUTION); //yellow
    Fill(COLOR_CAUTION); 
    } else {    
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    }
    sprintf(buffer, "%d°", temp_gnd);
    TextEnd(getWidth(pos_x)+width_value+width_label+getWidth(0.7), getHeight(pos_y)-height_text, buffer, myfont, text_scale);

    TextEnd(getWidth(pos_x)-width_value-width_ag, getHeight(pos_y)-height_text, "", osdicons, text_scale*0.7);
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y)-height_text, "G", myfont, text_scale*0.4);

    Fill(COLOR);
    Stroke(OUTLINECOLOR);
}



void draw_message(int severity, char line1[30], char line2[30], char line3[30], float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale*0.7)+getHeight(0.3)*scale;
    VGfloat height_text_small = TextHeight(myfont, text_scale*0.55)+getHeight(0.3)*scale;
    VGfloat width_text = TextWidth(line1, myfont, text_scale*0.7);

    if (severity == 0)  { // high severity
    Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    TextEnd(getWidth(pos_x)-width_text/2 - getWidth(0.3)*scale, getHeight(pos_y), "", osdicons, text_scale*0.8);
    Text(getWidth(pos_x)+width_text/2 + getWidth(0.3)*scale, getHeight(pos_y), "", osdicons, text_scale*0.8);
    } else if (severity == 1) { // medium
        Fill(229,255,20,getOpacity(COLOR)); // yellow
    Stroke(229,255,20,getOpacity(OUTLINECOLOR));
    TextEnd(getWidth(pos_x)-width_text/2 - getWidth(0.3)*scale, getHeight(pos_y), "", osdicons, text_scale*0.8);
    Text(getWidth(pos_x)+width_text/2 + getWidth(0.3)*scale, getHeight(pos_y), "", osdicons, text_scale*0.8);
    } else { // low
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    TextMid(getWidth(pos_x), getHeight(pos_y), line1, myfont, text_scale*0.7);
    TextMid(getWidth(pos_x), getHeight(pos_y) - height_text, line2, myfont, text_scale*0.55);
    TextMid(getWidth(pos_x), getHeight(pos_y) - height_text-height_text_small, line3, myfont, text_scale*0.55);
}



void draw_home_arrow(float abs_heading, float craft_heading, float pos_x, float pos_y, float scale){

    Stroke(OUTLINECOLOR);
    Fill(COLOR);
    //abs_heading is the absolute direction/bearing the arrow should point eg bearing to home could be 45 deg
    //because arrow is drawn relative to the osd/camera view we need to offset by craft's heading
    #if HOME_ARROW_INVERT == true
    abs_heading = abs_heading - 180;
    #endif
    float rel_heading = abs_heading - craft_heading; //direction arrow needs to point relative to camera/osd/craft
    if (rel_heading < 0) rel_heading += 360;
    if (rel_heading >= 360) rel_heading -=360;
    pos_x = getWidth(pos_x);
    pos_y = getHeight(pos_y);
    //offset for arrow, so middle of the arrow is at given position
    pos_x -= getWidth(1.25) * scale;
    pos_y -= getWidth(1.25) * scale;

    float x[8] = {getWidth(0.5)*scale+pos_x, getWidth(0.5)*scale+pos_x, pos_x, getWidth(1.25)*scale+pos_x, getWidth(2.5)*scale+pos_x, getWidth(2)*scale+pos_x, getWidth(2)*scale+pos_x, getWidth(0.5)*scale+pos_x};
    float y[8] = {pos_y, getWidth(1.5)*scale+pos_y, getWidth(1.5)*scale+pos_y, getWidth(2.5)*scale+pos_y, getWidth(1.5)*scale+pos_y, getWidth(1.5)*scale+pos_y, pos_y, pos_y};
    rotatePoints(x, y, rel_heading, 8, pos_x+getWidth(1.25)*scale,pos_y+getWidth(1.25)*scale);
    Polygon(x, y, 8);
    Polyline(x, y, 8);
}



void draw_compass(float heading, float home_heading, float pos_x, float pos_y, float scale){
    float text_scale = getHeight(1.5) * scale;
    float width_ladder = getHeight(16) * scale;
    float width_element = getWidth(0.25) * scale;
    float height_element = getWidth(0.50) * scale;
    float ratio = width_ladder / 180;

    Stroke(OUTLINECOLOR);
    Fill(COLOR);

    VGfloat height_text = TextHeight(myfont, text_scale*1.5)+getHeight(0.1)*scale;
    sprintf(buffer, "%.0f°", heading);
    TextMid(getWidth(pos_x), getHeight(pos_y) - height_element - height_text, buffer, myfont, text_scale*1.5);

    int i = heading - 90;
    char* c;
    bool draw = false;
    while (i <= heading + 90) {  //find all values from heading - 90 to heading + 90 that are % 15 == 0
    float x = getWidth(pos_x) + (i - heading) * ratio;
    if (i % 30 == 0) {
        Rect(x-width_element/2, getHeight(pos_y), width_element, height_element*2);
    }else if (i % 15 == 0) {
        Rect(x-width_element/2, getHeight(pos_y), width_element, height_element);
    }else{
        i++;
        continue;
    }

    int j = i;
    if (j < 0) j += 360;
    if (j >= 360) j -= 360;

    switch (j) {
        case 0:
            draw = true;
            #if CHINESE == true
            c = "北";
            #else
            c = "N";
	    #endif
            break;
        case 90:
            draw = true;
            #if CHINESE == true
            c = "东";
            #else
            c = "E";
            #endif
            break;
        case 180:
            draw = true;
            #if CHINESE == true
            c = "南";
            #else
            c = "S";
            #endif
            break;
        case 270:
            draw = true;
            #if CHINESE == true
            c = "西";
            #else
            c = "W";
            #endif
            break;
    }
    if (draw == true) {
        TextMid(x, getHeight(pos_y) + height_element*3.5, c, myfont, text_scale*1.5);
        draw = false;
    }
    if (j == home_heading) {
         TextMid(x, getHeight(pos_y) + height_element, "", osdicons, text_scale*1.3);
     }
    i++;
    }

    float rel_home = home_heading-heading;
    if (rel_home<0) rel_home+= 360;
    if ((rel_home > 90) && (rel_home <= 180)) { TextMid(getWidth(pos_x)+width_ladder/2 * 1.2, getHeight(pos_y), "", osdicons, text_scale * 0.8); }
    else if ((rel_home > 180) && (rel_home < 270)) { TextMid(getWidth(pos_x)-width_ladder/2 * 1.2, getHeight(pos_y), "", osdicons, text_scale * 0.8); }

    TextMid(getWidth(pos_x), getHeight(pos_y) + height_element*2.5+height_text, "", osdicons, text_scale*2);
}



void draw_batt_status(float voltage, float current, float pos_x, float pos_y, float scale){
    Stroke(OUTLINECOLOR);
    Fill(COLOR);
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;

    #if BATT_STATUS_CURRENT == true
    sprintf(buffer, "%.1f", current);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), " A", myfont, text_scale*0.6);
    #endif

    sprintf(buffer, "%.1f", voltage);
    TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text, buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y)+height_text, " V", myfont, text_scale*0.6);
}

// display totals mAh used, distance flown (km), airborne time (mins) - wowi
void draw_TOTAL_AMPS(float current, float pos_x, float pos_y, float scale){ 
    Stroke(OUTLINECOLOR);
    Fill(COLOR);
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    sprintf(buffer, "%5.0f", current);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), " mAh", myfont, text_scale*0.6);
 
}
void draw_TOTAL_DIST(int gpsspeed, float pos_x, float pos_y, float scale){
    Stroke(OUTLINECOLOR);
    Fill(COLOR);
 
    // get time passed since last rendering
    long time_diff = current_ts() - dist_ts;
    dist_ts = current_ts();

    float _kmh = (float)gpsspeed * 3.6;

    float _hours = (float)time_diff / (float)3600000;

    float added_distance = _kmh * _hours;

    total_dist = total_dist + added_distance;
 
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    sprintf(buffer, "%3.1f", total_dist);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), " km", myfont, text_scale*0.6);
 
}
void draw_TOTAL_TIME(float fly_time, float pos_x, float pos_y, float scale){   
    Stroke(OUTLINECOLOR);
    Fill(COLOR); 
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    sprintf(buffer, "%3.0f:%02d", fly_time, (int)(fly_time*60) % 60);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), "", osdicons, text_scale*0.9);
 
}

void draw_position(float lat, float lon, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    VGfloat width_value = TextWidth("-100.000000", myfont, text_scale);

    Fill(COLOR); //normal
    Stroke(OUTLINECOLOR);

    float mylat;
    float mylon;

    #if HIDE_LATLON == true
    mylat = lat - (int)lat;
    mylon = lon - (int)lon; 
    #else
    mylon=lon;
        mylat=lat;
    #endif

    #if CHINESE == true
    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "  ", osdicons, text_scale*0.6);
    #else
    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "ﵳ", osdicons, text_scale*0.6);
    #endif

    #if HIDE_LATLON == true
    sprintf(buffer, "0%f", mylon);
    #else
        sprintf(buffer, "%.6f", mylon);
    #endif

    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    #if CHINESE == true
    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y)+height_text, "  ", osdicons, text_scale*0.6);
    #else
    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y)+height_text, "ﵲ", osdicons, text_scale*0.6);
    #endif

    #if HIDE_LATLON == true
    sprintf(buffer, "0%f", mylat);
    #else
        sprintf(buffer, "%.6f", mylat);
    #endif

    TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text, buffer, myfont, text_scale);
}



void draw_home_distance(int distance, bool home_fixed, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00000", myfont, text_scale);

    if (!home_fixed){
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    }else{
        Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }
    TextEnd(getWidth(pos_x)-width_value-getWidth(0.2), getHeight(pos_y), "", osdicons, text_scale * 0.6);

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    #if IMPERIAL == true
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "ft", myfont, text_scale*0.6);
    sprintf(buffer, "%05d", (int)(distance*TO_FEET));
    #else
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "m", myfont, text_scale*0.6);
    sprintf(buffer, "%05d", distance);
    #endif
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}



void draw_alt_ladder(int alt, float pos_x, float pos_y, float scale, float warn, float caution, float vsi_time, float climb){

    #if IMPERIAL == true
        alt=alt *TO_FEET;
    #endif

    float text_scale = getHeight(1.3) * scale;
    float width_element = getWidth(0.50) * scale;
    float height_element = getWidth(0.25) * scale;
    float height_ladder = height_element * 21 * 4;

    float px = getWidth(pos_x); // ladder x position
    float pxlabel = getWidth(pos_x) + width_element*2; // alt labels on ladder x position

    float range = 100; // alt range range of display, i.e. lowest and highest number on the ladder
    float range_half = range / 2;
    float ratio_alt = height_ladder / range;

    VGfloat offset_text_ladder = (TextHeight(myfont, text_scale) / 2) - height_element/2 - getHeight(0.25)*scale;
    VGfloat offset_symbol = (TextHeight(osdicons, text_scale*2) / 2) - height_element/2 - getHeight(0.18)*scale;
    VGfloat offset_alt_value = (TextHeight(myfont, text_scale*2) / 2) -height_element/2 - getHeight(0.4)*scale;

    VGfloat width_symbol = TextWidth("", osdicons, text_scale*2);
    VGfloat width_ladder_value = TextWidth("000", myfont, text_scale);

    if (alt < warn) {
        Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING); 
    } else if (alt < caution) {
        Stroke(COLOR_CAUTION); //yellow
    Fill(COLOR_CAUTION); 
    } else {    
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
    }

    sprintf(buffer, "%d", alt); // large alt number

 //   Text(pxlabel+width_ladder_value+width_symbol, getHeight(pos_y)-offset_alt_value, buffer, myfont, text_scale*2);
 //   Text(pxlabel+width_ladder_value, getHeight(pos_y)-offset_symbol, "", osdicons, text_scale*2);

    Text(px+ width_element+20* scale, getHeight(pos_y)-offset_alt_value, buffer, myfont, text_scale*1.7);
    Text(px+ width_element+20* scale, getHeight(pos_y)-offset_symbol, "ᎆ", osdicons, text_scale*1.7);

    Fill(COLOR); //normal
    Stroke(OUTLINECOLOR);

    int k;
    for (k = (int) (alt - range / 2); k <= alt + range / 2; k++) {
    int y = getHeight(pos_y) + (k - alt) * ratio_alt;
    if (k % 10 == 0) {

        if (k >= 0) {
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        Rect(px-width_element, y, width_element*2, height_element);
        
		if (k>alt+5 || k<alt-5){
		sprintf(buffer, "%d", k);
        	Text(pxlabel, y-offset_text_ladder, buffer, myfont, text_scale);
		}
        }
        if (k < 0) {
        Fill(255,20,20,getOpacity(COLOR)); // red
        Stroke(255,20,20,getOpacity(OUTLINECOLOR));
        Rect(px-width_element, y+width_element*1.3, width_element*2, width_element*2);
        }
    } else if ((k % 5 == 0) && (k > 0)){
            Fill(COLOR); //normal
            Stroke(OUTLINECOLOR);
            Rect(px-width_element, y, width_element, height_element);
        }
    }
    
    //ALTITUDE TREND - VSI
    if (climb<0){
    Stroke(COLOR_DECLUTTER); //make outline opaque
    Fill(245,222,20,getOpacity(COLOR));} //yellow for decent
        else{
            Stroke(COLOR_DECLUTTER); //make outline opaque
            Fill(43,240,36,getOpacity(COLOR));} //green for climb

	VGfloat *LX,*LY;
	VGfloat Left_X[5] = {px+ width_element-4, px+ width_element+5,
	px+ width_element+5, px+ width_element+1.5, px+ width_element-4};

//VGfloat Left_Y[5];
//REALLY DIRTY VAR SCOPE FIX
//so arrow points up and or down with positive and negative climb
if (climb>0){
	VGfloat Left_Y[5] ={getHeight(pos_y), getHeight(pos_y), getHeight(pos_y)+climb*vsi_time, 
	getHeight(pos_y)+climb*vsi_time+2.5,
   	getHeight(pos_y)+climb*vsi_time};
	VGint npt = 5;
	LX = &Left_X[0];
	LY = &Left_Y[0];
	Polygon(LX,LY,npt);}
	else { 
	VGfloat Left_Y[5] ={getHeight(pos_y), getHeight(pos_y), getHeight(pos_y)+climb*vsi_time, 
	getHeight(pos_y)+climb*vsi_time-2.5,
    	getHeight(pos_y)+climb*vsi_time};
	VGint npt = 5;
	LX = &Left_X[0];
	LY = &Left_Y[0];
	Polygon(LX,LY,npt);}

}


void draw_mslalt(float mslalt, float pos_x, float pos_y, float scale){

    float text_scale = getWidth(2) * scale;

    Fill(COLOR); //normal
    Stroke(OUTLINECOLOR);

    #if IMPERIAL == true
    VGfloat width_value = TextWidth("0000", myfont, text_scale);
    sprintf(buffer, "%.0f", mslalt*TO_FEET);
    #else
    VGfloat width_value = TextWidth("000.0", myfont, text_scale);
    sprintf(buffer, "%.1f", mslalt);
    #endif
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    #if CHINESE == true
    TextEnd(getWidth(pos_x)-width_value-getWidth(0.3)*scale, getHeight(pos_y), " ", osdicons, text_scale*0.7);
    #else
    TextEnd(getWidth(pos_x)-width_value-getWidth(0.3)*scale, getHeight(pos_y), "ﶁ", osdicons, text_scale*0.7);
    #endif

    #if IMPERIAL == true
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "ft", myfont, text_scale*0.6);
    #else
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "m", myfont, text_scale*0.6);
    #endif
}


void draw_speed_ladder(int speed, float pos_x, float pos_y, float scale, float trend_time, float low_limit, float vx){

    Fill(COLOR); //normal
    Stroke(OUTLINECOLOR);

    float text_scale = getHeight(1.3) * scale;
    float width_element = getWidth(0.50) * scale;
    float height_element = getWidth(0.25) * scale;
    float height_ladder = height_element * 21 * 4;

    float px = getWidth(pos_x); // ladder x position
    float pxlabel = getWidth(pos_x) - width_element*2; // speed labels on ladder x position

    float range = 40; // speed range of display, i.e. lowest and highest number on the ladder
    float range_half = range / 2;
    float ratio_speed = height_ladder / range;

    VGfloat offset_text_ladder = (TextHeight(myfont, text_scale) / 2) - height_element/2 - getHeight(0.25)*scale;
    VGfloat offset_symbol = (TextHeight(osdicons, text_scale*2) / 2) - height_element/2 - getHeight(0.18)*scale;
    VGfloat offset_speed_value = (TextHeight(myfont, text_scale*2) / 2) -height_element/2 - getHeight(0.4)*scale;

    VGfloat width_symbol = TextWidth("", osdicons, text_scale*2);
    VGfloat width_ladder_value = TextWidth("0", myfont, text_scale);

    sprintf(buffer, "%d", speed); // large speed number
  //  TextEnd(pxlabel-width_ladder_value-width_symbol, getHeight(pos_y)-offset_speed_value, buffer, myfont, text_scale*2);
 //   TextEnd(pxlabel-width_ladder_value, getHeight(pos_y)-offset_symbol, "", osdicons, text_scale*2);

    TextEnd(pxlabel-9*scale, getHeight(pos_y)-offset_speed_value, buffer, myfont, text_scale*1.7);
    TextEnd(pxlabel-9*scale, getHeight(pos_y)-offset_symbol, "ᎄ", osdicons, text_scale*1.7);


    int k;
    for (k = (int) (speed - range_half); k <= speed + range_half; k++) {
    int y = getHeight(pos_y) + (k - speed) * ratio_speed;
    if (k % 5 == 0) { // wide element plus number label every 5 'ticks' on the scale
        
        if (k >= low_limit) {
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        Rect(px-width_element, y, width_element*2, height_element);
		
		//so the box around number is not overwritten
		if (k>speed+3 || k<speed-3){
        	sprintf(buffer, "%d", k);
        	TextEnd(pxlabel, y-offset_text_ladder, buffer, myfont, text_scale);
		}

        }
        if (k < low_limit) {
        Fill(255,20,20,getOpacity(COLOR)); // red
        Stroke(255,20,20,getOpacity(OUTLINECOLOR));
        Rect(px-width_element, y+width_element*1.9, width_element*2, width_element*2);

        }
    } else if ((k % 1 == 0) && (k > low_limit)){ // narrow element every single 'tick' on the scale 
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        Rect(px, y, width_element, height_element);}
    }

// Speed Trend

    if (vx<0){
    Stroke(COLOR_DECLUTTER); //make outline opaque
    Fill(245,222,20,getOpacity(COLOR));} //yellow for decent
        else{
            Stroke(COLOR_DECLUTTER); //make outline opaque
            Fill(43,240,36,getOpacity(COLOR));} //green for climb

    VGfloat *LX,*LY;

VGfloat Left_X[5] = {pxlabel+3, pxlabel+12, pxlabel+12, 
    pxlabel+7.5, pxlabel+3};

//VGfloat Left_Y[5];
//REALLY DIRTY VAR SCOPE FIX
//so arrow points up and or down with positive and negative climb
if (vx>0){ 
    VGfloat Left_Y[5] ={getHeight(pos_y), getHeight(pos_y), getHeight(pos_y)+vx*trend_time, getHeight(pos_y)+vx*trend_time+2.5,
    getHeight(pos_y)+vx*trend_time};
VGint npt = 5;
LX = &Left_X[0];
LY = &Left_Y[0];
Polygon(LX,LY,npt);}
else {
        VGfloat Left_Y[5] ={getHeight(pos_y), getHeight(pos_y), getHeight(pos_y)+vx*trend_time, getHeight(pos_y)+vx*trend_time-2.5,
    getHeight(pos_y)+vx*trend_time};
VGint npt = 5;
LX = &Left_X[0];
LY = &Left_Y[0];
Polygon(LX,LY,npt);}



}


void draw_yaw_display(float vy, float pos_x, float pos_y, float scale, float trend_time){
    Fill(COLOR); //normal
    Stroke(OUTLINECOLOR);

    float text_scale = getHeight(1.5) * scale;
    float height_display = getHeight(1.5) * scale;
    float width_element = getWidth(10) * scale;
    float height_element = getWidth(1) * scale;

 //  TextMid(getWidth(pos_x), getHeight(pos_y) - height_element - height_text, buffer, myfont, text_scale*1.5);

    VGfloat *LX,*LY;

   VGfloat Left_Y[5] ={getHeight(pos_y), getHeight(pos_y), getHeight(pos_y)+height_element/2, getHeight(pos_y)+height_element, getHeight(pos_y)+height_element};

//REALLY DIRTY VAR SCOPE FIX
if (vy<0){
    VGfloat Left_X[5] = { getWidth(pos_x), getWidth(pos_x)+(vy*trend_time), getWidth(pos_x)+(vy*trend_time)+height_element/2, getWidth(pos_x)+(vy*trend_time),getWidth(pos_x)};
    
    VGint npt = 5;
    LX = &Left_X[0];
    LY = &Left_Y[0];
    Polygon(LX,LY,npt);
}
if (vy>0) {
VGfloat Left_X[5] = { getWidth(pos_x), getWidth(pos_x)+(vy*trend_time), getWidth(pos_x)+(vy*trend_time)-height_element/2, getWidth(pos_x)+(vy*trend_time),getWidth(pos_x)}; // so tip of arrow goes correct way
    
    VGint npt = 5;
    LX = &Left_X[0];
    LY = &Left_Y[0];
    Polygon(LX,LY,npt);

}

}


void draw_card_signal(int8_t signal, int signal_good, int card, int adapter_cnt, int restart_count, int packets, int wrongcrcs, int type, int totalpackets, int totalpacketslost, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.4)*scale;
    VGfloat width_value = TextWidth("-00", myfont, text_scale);
    VGfloat width_cardno = TextWidth("0", myfont, text_scale*0.4);
    VGfloat width_unit = TextWidth("dBm", myfont, text_scale*0.6);

    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    StrokeWidth(OUTLINEWIDTH);

    sprintf(buffer, "");
    TextEnd(getWidth(pos_x) - width_value - width_cardno - getWidth(0.3)*scale, getHeight(pos_y) - card * height_text, buffer, osdicons, text_scale*0.6);

    sprintf(buffer, "%d",card);
    TextEnd(getWidth(pos_x) - width_value - getWidth(0.3)*scale, getHeight(pos_y) - card * height_text, buffer, myfont, text_scale*0.4);

    if (( signal_good == 0) || (signal == -127)) {
    Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    sprintf(buffer, "-- ");
    } else {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }

    sprintf(buffer, "%d", signal);
    
    TextEnd(getWidth(pos_x), getHeight(pos_y) - card * height_text, buffer, myfont, text_scale);
    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    sprintf(buffer, "dBm");
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y) - card * height_text, buffer, myfont, text_scale*0.6);

    if (restart_count - tx_restart_count_last > 0) {
    int y;
    for (y=0; y<adapter_cnt; y++) {
        packetslost_last[y] = 0;
    }
    }
    tx_restart_count_last = restart_count;

    int lost=totalpackets-packets+totalpacketslost;
    if (lost < packetslost_last[card]) lost = packetslost_last[card];
    packetslost_last[card] = lost;
    int percent_lost_card=(int)((double)lost/packets*100);
    sprintf(buffer, "%d (%d%%)", lost, percent_lost_card);
    Text(getWidth(pos_x)+width_unit+getWidth(0.65)*scale, getHeight(pos_y) - card * height_text, buffer, myfont, text_scale*0.7);
}



void draw_total_signal(int8_t signal, int goodblocks, int badblocks, int packets_lost, int packets_received, int lost_per_block, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale*0.6)+getHeight(0.3)*scale;
    VGfloat width_value = TextWidth("-00", myfont, text_scale);
    VGfloat width_label = TextWidth("dBm", myfont, text_scale*0.6);
    VGfloat width_symbol = TextWidth(" ", osdicons, text_scale*0.7);

    Fill(COLOR); //normal
    Stroke(OUTLINECOLOR);
    StrokeWidth(OUTLINEWIDTH);

    if (no_signal == true) {
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    sprintf(buffer, "-- ");
    } else {
        Fill(COLOR);
    Stroke(OUTLINECOLOR);
    sprintf(buffer, "%02d", signal);
    }
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "dBm", myfont, text_scale*0.6);

    int percent_badblocks=(int)((double)badblocks/goodblocks*100);
    int percent_packets_lost=(int)((double)packets_lost/packets_received*100);

    sprintf(buffer, "%d (%d%%)/%d (%d%%)", badblocks, percent_badblocks, packets_lost, percent_packets_lost);
    
    Text(getWidth(pos_x)-width_value-width_symbol+2, getHeight(pos_y)-height_text, buffer, myfont, text_scale*0.6);

    TextEnd(getWidth(pos_x)-width_value - getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.7);

	//MOVING AVERAGE: LOST PER BLOCK (LPB)
	int lpb_average=0;
	int lpb_sum=0;

	//new lpb.. overwrite oldest value in array
		if (lpb_counter>20){
			lpb_counter=0;
		}

		lpb_array[lpb_counter++]=lost_per_block;

	//calculate average
		//total array value divided by array size
	int i;
		for (i = 0; i < 20; i++) {
		    lpb_sum += lpb_array[i];
		}
		//8 per block. so array length x 8
		lpb_average=lpb_sum/20;

		//display options- graphical only or with number TODO

    switch (lpb_average) {
    case 0:
        sprintf(buffer, "▁");
        Fill(COLOR);
        break;
    case 1:
        sprintf(buffer, "▂");
        Fill(74,255,4,0.5); // green
            break;
    case 2:
        sprintf(buffer, "▃");
        Fill(112,255,4,0.5); //
            break;
    case 3:
        sprintf(buffer, "▄");
        Fill(182,255,4,0.5); //
            break;
    case 4:
        sprintf(buffer, "▅");
        Fill(255,208,4,0.5); //
            break;
    case 5:
        sprintf(buffer, "▆");
        Fill(255,93,4,0.5); //
            break;
    case 6:
        sprintf(buffer, "▇");
        Fill(255,50,4,0.5); //
            break;
    case 7:
        sprintf(buffer, "█");
        Fill(255,0,4,0.5); // red
            break;
    case 8:
        sprintf(buffer, "█");
        Fill(255,0,4,0.5); // red
            break;
    default:
        sprintf(buffer, "█");
        Fill(255,0,4,0.5); // red
    }

    #if DOWNLINK_RSSI_FEC_BAR == true
    StrokeWidth(0);
    Text(getWidth(pos_x)+width_label+getWidth(0.7), getHeight(pos_y)+getHeight(0.5), buffer, osdicons, text_scale*0.7);

    StrokeWidth(1);
    Fill(0,0,0,0); // transparent
    Text(getWidth(pos_x)+width_label+getWidth(0.7), getHeight(pos_y)+getHeight(0.5), "█", osdicons, text_scale*0.7);

Fill(COLOR);
Stroke(OUTLINECOLOR);
sprintf(buffer, "%d/12", lpb_average);
Text(getWidth(pos_x)+width_label+getWidth(3), getHeight(pos_y)+getHeight(0.5), buffer, myfont, text_scale*0.5);


    #endif
}



void draw_sat(int sats, int fixtype, int hdop, int armed, float pos_x, float pos_y, float scale, float hdop_warn, float hdop_caution, float declutter){
    float text_scale = getWidth(2) * scale;

    StrokeWidth(OUTLINEWIDTH);
    if (fixtype < 2){
        Fill(COLOR_WARNING); // red
        Stroke(COLOR_WARNING);
    }else{
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    }

    TextEnd(getWidth(pos_x), getHeight(pos_y), "", osdicons, text_scale*0.7);
   
    #if defined(LTM) || defined(MAVLINK) || defined(VOT)
    float decimal_hdop = (float) hdop/100;
     if (decimal_hdop > hdop_warn) {
        Stroke(COLOR_WARNING); //red
    Fill(COLOR_WARNING); 
    } else if (decimal_hdop > hdop_caution) {
        Stroke(COLOR_CAUTION); //yellow
    Fill(COLOR_CAUTION); 
    } else {    
        Fill(COLOR); //normal
        Stroke(OUTLINECOLOR);
        if ((armed==1)&&(declutter==1)){
        Stroke(COLOR_DECLUTTER); //opaque
        Fill(COLOR_DECLUTTER);}
    } 
    
    sprintf(buffer, "%d(%.2f)", sats, decimal_hdop);
    Text(getWidth(pos_x)+getWidth(0.2), getHeight(pos_y), buffer, myfont, text_scale);
    #endif

    Fill(COLOR); //normal
    Stroke(OUTLINECOLOR);
}



void draw_batt_gauge(int remaining, float pos_x, float pos_y, float scale){

    Stroke(OUTLINECOLOR);
    Fill(COLOR);

    if (remaining < 0) remaining = 0;
    else if (remaining > 100) remaining = 100;

    int cell_width = getWidth(4) * scale;
    int cell_height = getWidth(1.6) * scale;
    int plus_width = cell_width * 0.09;
    int plus_height = cell_height * 0.3;

    int corner = cell_width * 0.05;
    int stroke_x = cell_width * 0.05;
    int stroke_y = cell_height * 0.1;

    if (remaining <= CELL_WARNING_PCT1 && remaining > CELL_WARNING_PCT2) {
    Fill(255,165,0,getOpacity(COLOR));
    Stroke(255,165,0,getOpacity(OUTLINECOLOR));
    } else if (remaining <= CELL_WARNING_PCT2) { 
    Fill(255,20,20,getOpacity(COLOR));
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    } else {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }

    StrokeWidth(OUTLINEWIDTH);

    Roundrect(getWidth(pos_x), getHeight(pos_y), cell_width, cell_height, corner, corner); // battery cell
    Rect(getWidth(pos_x)+cell_width, getHeight(pos_y)+cell_height/2 - plus_height/2, plus_width, plus_height); // battery plus pole

    Fill(0,0,0,0.5);
    Rect(getWidth(pos_x) + stroke_x + remaining / 100.0f * cell_width, getHeight(pos_y) + stroke_y, cell_width - stroke_x*2 - remaining / 100.0f * cell_width, cell_height - stroke_y*2);
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

    VGfloat offset_text_ladder = (TextHeight(myfont, text_scale*0.85) / 2) - height_element/2;

    float px_l  = pos_x - width_ladder / 2 + width_ladder / 3 - width_ladder / 12; // left three bars
    float px3_l = pos_x - width_ladder / 2 + 0.205f * width_ladder- width_ladder / 12; // left three bars
    float px5_l = pos_x - width_ladder / 2 + 0.077f * width_ladder- width_ladder / 12; // left three bars
    float px_r =  pos_x + width_ladder / 2 - width_ladder / 3; // right three bars
    float px3_r = pos_x + width_ladder / 2 - 0.205f * width_ladder; // right three bars
    float px5_r = pos_x + width_ladder / 2 - 0.077f * width_ladder; // right three bars

    StrokeWidth(OUTLINEWIDTH);
    Stroke(OUTLINECOLOR);
    Fill(COLOR);

    TextEnd(getWidth(50), getHeight(50), "ᎅ", osdicons, text_scale*2.5);

    Translate(pos_x, pos_y);
    Rotate(roll);
    Translate(-pos_x, -pos_y);

    int k = pitch - range/2;
    int max = pitch + range/2;
    while (k <= max){
    float y = pos_y + (k - pitch) * ratio;
    if (k % 5 == 0 && k!= 0) {
        #if AHI_LADDER == true
        sprintf(buffer, "%d", k);
        TextEnd(pos_x - width_ladder / 2 - space_text, y - width / height_ladder, buffer, myfont, text_scale*0.85); // right numbers
        Text(pos_x + width_ladder / 2 + space_text, y - width / height_ladder, buffer, myfont, text_scale*0.85); // left numbers
        #endif
    }
    if ((k > 0) && (k % 5 == 0)) { // upper ladders
        #if AHI_LADDER == true
        float px = pos_x - width_ladder / 2;
        Rect(px, y, width_ladder/3, height_element);
        Rect(px+width_ladder*2/3, y, width_ladder/3, height_element);
        #endif
    } else if ((k < 0) && (k % 5 == 0)) { // lower ladders
        #if AHI_LADDER == true
        Rect( px_l, y, width_ladder/12, height_element);
        Rect(px3_l, y, width_ladder/12, height_element);
        Rect(px5_l, y, width_ladder/12, height_element);
        Rect( px_r, y, width_ladder/12, height_element);
        Rect(px3_r, y, width_ladder/12, height_element);
        Rect(px5_r, y, width_ladder/12, height_element);
        #endif
    } else if (k == 0) { // center line
        #if AHI_LADDER == true
        sprintf(buffer, "%d", k);
        TextEnd(pos_x - width_ladder / 1.25f - space_text, y - width / height_ladder, buffer, myfont, text_scale*0.85); // left number
        Text(pos_x + width_ladder / 1.25f + space_text, y - width / height_ladder, buffer, myfont, text_scale*0.85); // right number
        #endif
        Rect(pos_x - width_ladder / 1.25f, y, 2*(width_ladder /1.25f), height_element);
    }
    k++;
    }
}


void draw_ahi_mav(float roll, float pitch, float climb, float vz, float vx, float vy, float gpsspeed, float alt, float scale){
    float text_scale = getHeight(1.2) * scale;
    float height_ladder = getWidth(15) * scale;
    float width_ladder = getWidth(30) * scale;
    float height_element = getWidth(0.25) * scale;
    float range = 100;
    float space_text = getWidth(0.2) * scale;
    float ratio = height_ladder / range;
    float pos_x = getWidth(50);
    float pos_y = getHeight(50);


//Limit draw area
ClipRect(pos_x-width_ladder, pos_y-getHeight(30), width_ladder*2, getHeight(60));




    VGfloat offset_text_ladder = (TextHeight(myfont, text_scale*0.85) / 2) - height_element/2;

    float px_l  = pos_x - width_ladder / 2 + width_ladder / 3 - width_ladder / 12; // left three bars
    float px3_l = pos_x - width_ladder / 2 + 0.205f * width_ladder- width_ladder / 12; // left three bars
    float px5_l = pos_x - width_ladder / 2 + 0.077f * width_ladder- width_ladder / 12; // left three bars
    float px_r =  pos_x + width_ladder / 2 - width_ladder / 3; // right three bars
    float px3_r = pos_x + width_ladder / 2 - 0.205f * width_ladder; // right three bars
    float px5_r = pos_x + width_ladder / 2 - 0.077f * width_ladder; // right three bars
  

    StrokeWidth(OUTLINEWIDTH);
    Stroke(OUTLINECOLOR);
    Fill(COLOR);
	

  if (alt < ALTLADDER_CAUTION) {	
	if (climb < -3.0f) {
	  Stroke(COLOR_WARNING); //red
          Fill(COLOR_WARNING);
	}
	else if ((climb >= -3.0f) && (climb < -1.5f)) {
	  Stroke(COLOR_WARNING); //caution
          Fill(COLOR_WARNING);
	}
	//else if ((climb >= -1.5f) && (climb < 0.0f)) {
	//  Stroke(COLOR_GOOD); //good
    	//  Fill(COLOR_GOOD);
	//}
	else {
	  StrokeWidth(OUTLINEWIDTH);
    	  Stroke(OUTLINECOLOR);
          Fill(COLOR);
	}
  }

  if (gpsspeed < SPEEDLADDER_LOW_LIMIT) {
          Stroke(COLOR_WARNING); //red
          Fill(COLOR_WARNING);
  }	
    //Bore Sight
 //   TextEnd(getWidth(50), getHeight(50), "ᎅ", osdicons, text_scale*2.5); 
  
    Translate(pos_x, pos_y);
    Rotate(roll);
    Translate(-pos_x, -pos_y);

//Flight Path Vector

	//get x y z axis

	//FPV_Z calculate vector 
	// x*x+z*z= resultant/resultant=resultant

	//get angle
	//tan(Theta) = (5/10) = 0.5      (x/y) tan-1 + 90
	//Theta = tan-1 (0.5)
	//Theta = 26.6 degrees
	//Direction of R = 90 deg + 26.6 deg
	//Direction of R = 116.6 deg

	//FPV_Y
        double fpv_y;
	fpv_y=tan(vx/vy);
	fpv_y=fpv_y+90;
	//FPV_Z
        double fpv_z;
	fpv_z=tan(vx/vz) +90; 

    int k = pitch - range;
    int max = pitch + range; //range and max not as important as clipping is implimented
    while (k <= max){
	float y = pos_y + (k - pitch) * ratio;
	if (k % 5 == 0 && k!= 0) {
	    #if AHI_LADDER == true    
		  #if AHI_ROLLANGLE == true
		     sprintf(buffer, "%.1f°", roll*AHI_ROLLANGLE_INVERT);
	         Text(pos_x + width_ladder / 2 + space_text, y - width / height_ladder, buffer, myfont, text_scale*1.1); // right numbers
		  #else
			 sprintf(buffer, "%d", k);
	         Text(pos_x + width_ladder / 2 + space_text, y - width / height_ladder, buffer, myfont, text_scale*1.1); // right numbers
		  #endif
		sprintf(buffer, "%d", k);
		TextEnd(pos_x - width_ladder / 2 - space_text, y - width / height_ladder, buffer, myfont, text_scale*1.1); // left numbers
	    #endif
	}
	if ((k > 0) && (k % 5 == 0)) { // upper ladders
	    #if AHI_LADDER == true
	    float px = pos_x - width_ladder / 2;
	    Rect(px, y, width_ladder/3, height_element);
	    Rect(px+width_ladder*2/3, y, width_ladder/3, height_element);
	    #endif
	} else if ((k < 0) && (k % 5 == 0)) { // lower ladders
	    #if AHI_LADDER == true
	    Rect( px_l, y, width_ladder/12, height_element);
	    Rect(px3_l, y, width_ladder/12, height_element);
	    Rect(px5_l, y, width_ladder/12, height_element);
	    Rect( px_r, y, width_ladder/12, height_element);
	    Rect(px3_r, y, width_ladder/12, height_element);
	    Rect(px5_r, y, width_ladder/12, height_element);
	    #endif
	} else if (k == 0) { // center line
	    #if AHI_LADDER == true
	    sprintf(buffer, "%d", k);
	    TextEnd(pos_x - width_ladder / 1.25f - space_text, y - width / height_ladder, buffer, myfont, text_scale*1.1); // left number
		  #if AHI_ROLLANGLE == true
		    sprintf(buffer, "%.1f°", roll*AHI_ROLLANGLE_INVERT);
	        Text(pos_x + width_ladder / 1.25f + space_text, y - width / height_ladder, buffer, myfont, text_scale*1.1); // right number
		  #else
			sprintf(buffer, "%d", k);
	        Text(pos_x + width_ladder / 1.25f + space_text, y - width / height_ladder, buffer, myfont, text_scale*1.1); // right number  
		  #endif	  
	    #endif
//DRAW MAIN HORIZON BAR
	    Rect(pos_x - width_ladder / 1.25f, y, 2*(width_ladder /1.25f)/5*2, height_element*1.5);
	    Rect(pos_x - width_ladder / 1.25f + 2*(width_ladder /1.25f)/5*3, y, 2*(width_ladder /1.25f)/5*2,
                 height_element*1.5);

//Bore Sight
    TextEnd(pos_x+vy*30, y-vz*30, "ᎅ", osdicons, text_scale*2.5);

		  #if AHI_ROLLANGLE == true
		    sprintf(buffer, "%.1f°", roll*AHI_ROLLANGLE_INVERT);
	        Text(pos_x + width_ladder / 1.25f + space_text, y - width / height_ladder, buffer, myfont, text_scale*1.1); // right number
		  #endif	  
	}
	k++;
    }

//TODO
ClipEnd();

    StrokeWidth(OUTLINEWIDTH);
    Stroke(OUTLINECOLOR);
    Fill(COLOR);
}


// work in progress
void draw_osdinfos(int osdfps, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale);

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "OSDFPS:", myfont, text_scale*0.6);

    sprintf(buffer, "%d", osdfps);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}


float distance_between(float lat1, float long1, float lat2, float long2) {
    //taken from tinygps: https://github.com/mikalhart/TinyGPS/blob/master/TinyGPS.cpp#L296
    // returns distance in meters between two positions, both specified
    // as signed decimal-degrees latitude and longitude. Uses great-circle
    // distance computation for hypothetical sphere of radius 6372795 meters.
    // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
    // Courtesy of Maarten Lamers
    float delta = (long1-long2)*0.017453292519;
    float sdlong = sin(delta);
    float cdlong = cos(delta);
    lat1 = (lat1)*0.017453292519;
    lat2 = (lat2)*0.017453292519;
    float slat1 = sin(lat1);
    float clat1 = cos(lat1);
    float slat2 = sin(lat2);
    float clat2 = cos(lat2);
    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
    delta = delta*delta;
    delta += (clat2 * sdlong)*(clat2 * sdlong);
    delta = sqrt(delta);
    float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
    delta = atan2(delta, denom);

    return delta * 6372795;
}



float course_to (float lat1, float long1, float lat2, float long2) {
    //taken from tinygps: https://github.com/mikalhart/TinyGPS/blob/master/TinyGPS.cpp#L321
    // returns course in degrees (North=0, West=270) from position 1 to position 2,
    // both specified as signed decimal-degrees latitude and longitude.
    // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
    // Courtesy of Maarten Lamers
    float dlon = (long2-long1)*0.017453292519;
    lat1 = (lat1)*0.017453292519;
    lat2 = (lat2)*0.017453292519;
    float a1 = sin(dlon) * cos(lat2);
    float a2 = sin(lat1) * cos(lat2) * cos(dlon);
    a2 = cos(lat1) * sin(lat2) - a2;
    a2 = atan2(a1, a2);
    if (a2 < 0.0) a2 += M_PI*2;
    return TO_DEG*(a2);
}



void rotatePoints(float *x, float *y, float angle, int points, int center_x, int center_y){
    double cosAngle = cos(-angle * 0.017453292519);
    double sinAngle = sin(-angle * 0.017453292519);

    int i = 0;
    float tmp_x = 0;
    float tmp_y = 0;
    while (i < points){
    tmp_x = center_x + (x[i]-center_x)*cosAngle-(y[i]-center_y)*sinAngle;
    tmp_y = center_y + (x[i]-center_x)*sinAngle + (y[i] - center_y)*cosAngle;
    x[i] = tmp_x;
    y[i] = tmp_y;
    i++;
    }
}

void draw_RPA(float roll, float pitch, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00000.0", myfont, text_scale)*1.1;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    
    //int introll = (int)roll;
    //int intpitch = (int)pitch;
    
    sprintf(buffer, "%.1f°", roll);
    TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*1.3, buffer, myfont, text_scale);
    
    if (roll - 5.01 > 0) {
        TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y)+height_text*1.3, "ᎇ", osdicons, text_scale*1.2);
    }
    else if (roll + 5.01 < 0)  {
        TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y)+height_text*1.3, "ᎈ", osdicons, text_scale*1.2);
    }
    else  {
        TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y)+height_text*1.3, "᎔", osdicons, text_scale*1.2);
    }   
    
    sprintf(buffer, "%.1f°", pitch);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    
    if (pitch - 3.01 > 0) {
        TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "ᎉ", osdicons, text_scale*1.2);
    }
    else if (pitch + 3.01 < 0) {
        TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "ᎊ", osdicons, text_scale*1.2);
    }
   else {
        TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "᎕", osdicons, text_scale*1.2);
    }
}

void draw_Mission(int Seq,float pos_x, float pos_y, float scale){
    
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale);
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    #if CHINESE == true
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y)+getHeight(0.3)*scale, "航 点:", myfont, text_scale*0.9);
    #else
    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y)+getHeight(0.3)*scale, "Mission:", myfont, text_scale*0.9);
    #endif
    sprintf(buffer, "%d", Seq);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

}

void draw_Angle(float pos_x, float pos_y, float scale){
    
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale);
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;

   TextEnd(getWidth(pos_x), getHeight(pos_y), "ᐕ", osdicons, text_scale*5);   //角度指示器1
}

void draw_Angle2(float pos_x, float pos_y, float scale){
    
    float text_scale = getWidth(2) * scale;

    TextEnd(getWidth(pos_x), getHeight(pos_y), "ᐖ", osdicons, text_scale);   //角度指示器2
}

void draw_Alarm(int SenorsPresent, int SenorsEnabled, int SenorsHealth, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale);
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(1)*scale;
    
    int row = 0;
    
    Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    
    /*if (SenorsHealth & 0b00000000000000000000000001) == 0) { */
      /*sprintf(buffer, "%d", SenorsHealth);
      TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);  */
    #if ALARM_1 == true
    if (((SenorsEnabled & 0b00000000000000000000000001) == 1) && ((SenorsHealth & 0b00000000000000000000000001) == 0))  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "陀 螺 仪", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "3D GYRO", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_2 == true
    if (((SenorsEnabled & 0b00000000000000000000000010) >> 1 == 1) && ((SenorsHealth & 0b00000000000000000000000010)) >> 1 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "加 速 度 计", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "3D ACCEL", myfont, text_scale);
	#endif
      row += 1;
    }
    #endif
    #if ALARM_3 == true
    if (((SenorsEnabled & 0b00000000000000000000000100) >> 2 == 1) && ((SenorsHealth & 0b00000000000000000000000100)) >> 2 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "磁 罗 盘", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "3D MAG", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_4 == true
    if (((SenorsEnabled & 0b00000000000000000000001000) >> 3 == 1) && ((SenorsHealth & 0b00000000000000000000001000)) >> 3 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "空 速 计 静 压", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "ABSOLUTE PRESSURE", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_5 == true
    if (((SenorsEnabled & 0b00000000000000000000010000) >> 4 == 1) && ((SenorsHealth & 0b00000000000000000000010000)) >> 4 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "空 速 计 动 压", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "DIFFERENTIAL PRESSURE", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_6 == true
    if (((SenorsEnabled & 0b00000000000000000000100000) >> 5 == 1) && ((SenorsHealth & 0b00000000000000000000100000)) >> 5 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "全 球 定 位 系 统 GPS", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "GPS", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_7 == true
    if (((SenorsEnabled & 0b00000000000000000001000000) >> 6 == 1) && ((SenorsHealth & 0b00000000000000000001000000)) >> 6 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "光 流 传 感 器", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "OPTICAL FLOW", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_8 == true
    if (((SenorsEnabled & 0b00000000000000000010000000) >> 7 == 1) && ((SenorsHealth & 0b00000000000000000010000000)) >> 7 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "视 觉 传 感 器", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "VISION POSITION", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_9 == true
    if (((SenorsEnabled & 0b00000000000000000100000000) >> 8 == 1) && ((SenorsHealth & 0b00000000000000000100000000)) >> 8 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "激 光 传 感 器", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "LASER POSITION", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_10 == true
    if (((SenorsEnabled & 0b00000000000000001000000000) >> 9 == 1) && ((SenorsHealth & 0b00000000000000001000000000)) >> 9 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "External Ground Truth", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "External Ground Truth", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_11 == true
    if (((SenorsEnabled & 0b00000000000000010000000000) >> 10 == 1) && ((SenorsHealth & 0b00000000000000010000000000)) >> 10 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "角 速 率 控 制", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "ANGULAR RATE CONTROL", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_12 == true
    if (((SenorsEnabled & 0b00000000000000100000000000) >> 11 == 1) && ((SenorsHealth & 0b00000000000000100000000000)) >> 11 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "姿 态 稳 定", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "ATTITUDE STABILIZATION", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_13 == true
    if (((SenorsEnabled & 0b00000000000001000000000000) >> 12 == 1) && ((SenorsHealth & 0b00000000000001000000000000)) >> 12 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "偏 航 位 置", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "YAW POSITION", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_14 == true
    if (((SenorsEnabled & 0b00000000000010000000000000) >> 13 == 1) && ((SenorsHealth & 0b00000000000010000000000000)) >> 13 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "Z 轴 高 度 控 制", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "Z ALTITUDE CONTROL", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_15 == true
    if (((SenorsEnabled & 0b00000000000100000000000000) >> 14 == 1) && ((SenorsHealth & 0b00000000000100000000000000)) >> 14 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "X/Y 轴 位 置 控 制", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "XY POSITION CONTROL", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_16 == true
    if (((SenorsEnabled & 0b00000000001000000000000000) >> 15 == 1) && ((SenorsHealth & 0b00000000001000000000000000)) >> 15 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "电 机 输 出", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "MOTOR OUTPUTS", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_17 == true
    if (((SenorsEnabled & 0b00000000010000000000000000) >> 16 == 1) && ((SenorsHealth & 0b00000000010000000000000000)) >> 16 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "接 收 机", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "RC RECEIVER ", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_18 == true
    if (((SenorsEnabled & 0b00000000100000000000000000) >> 17 == 1) && ((SenorsHealth & 0b00000000100000000000000000)) >> 17 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "2 号 陀 螺 仪", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "3D GYRO2", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_19 == true
    if (((SenorsEnabled & 0b00000001000000000000000000) >> 18 == 1) && ((SenorsHealth & 0b00000001000000000000000000)) >> 18 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "2 号 加 速 度 计", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "3D ACCEL2", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_20 == true
    if (((SenorsEnabled & 0b00000010000000000000000000) >> 19 == 1) && ((SenorsHealth & 0b00000010000000000000000000)) >> 19 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "2 号 磁 罗 盘", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "3D MAG2", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_21 == true
    if (((SenorsEnabled & 0b00000100000000000000000000) >> 20 == 1) && ((SenorsHealth & 0b00000100000000000000000000)) >> 20 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "地 理 围 栏", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "GEOFENCE", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_22 == true
    if (((SenorsEnabled & 0b00001000000000000000000000) >> 21 == 1) && ((SenorsHealth & 0b00001000000000000000000000)) >> 21 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "航 姿 参 考 系 统 AHRS", myfont, text_scale);
	#else
	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "AHRS", myfont, text_scale);
	#endif
      	row += 1;
    }   
    #endif
    #if ALARM_23 == true
    if (((SenorsEnabled & 0b00010000000000000000000000) >> 22 == 1) && ((SenorsHealth & 0b00010000000000000000000000)) >> 22 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "地 形", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "TERRAIN", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_24 == true
    if (((SenorsEnabled & 0b00100000000000000000000000) >> 23 == 1) && ((SenorsHealth & 0b00100000000000000000000000)) >> 23 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "电 机 反 转", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "REVERSE MOTOR", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_25 == true
    if (((SenorsEnabled & 0b01000000000000000000000000) >> 24 == 1) && ((SenorsHealth & 0b01000000000000000000000000)) >> 24 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "日 志 记 录", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "LOGGING", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif
    #if ALARM_26 == true
    if (((SenorsEnabled & 0b10000000000000000000000000) >> 25 == 1) && ((SenorsHealth & 0b10000000000000000000000000)) >> 25 == 0)  {
	#if CHINESE == true
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "电 池 传 感 器", myfont, text_scale);
	#else
      	TextEnd(getWidth(pos_x), getHeight(pos_y)+height_text*row, "BATTERY", myfont, text_scale);
	#endif
      	row += 1;
    }
    #endif

    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    
}

void draw_home_radar(float abs_heading, float craft_heading, int homedst, float pos_x, float pos_y, float scale){
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
    
    if (homedst <= 500)   dstrng = 500;
    //else if (homedst > 250 && homedst<=500)   dstrng = 500;
    else if (homedst > 500 && homedst<=1000)   dstrng = 1000;
    else if (homedst > 1000 && homedst<=2000)   dstrng = 2000;
    else if (homedst > 1000 && homedst<=4000)   dstrng = 4000;
    else if (homedst > 4000 && homedst<=8000)   dstrng = 8000;
    else if (homedst > 8000 && homedst<=16000)  dstrng = 16000;
    else if (homedst > 16000 && homedst<=32000) dstrng = 32000;
    else dstrng = homedst;

    //sprintf(buffer, "%d", position_algle);
    //TextEnd(700, 400, buffer, myfont, text_scale);
    //sprintf(buffer, "%.2f", 500.000/dstrng*homedst);
    //TextEnd(700, 350, buffer, myfont, text_scale);
    //sprintf(buffer, "%.2f", 280.000/dstrng*homedst);
    //TextEnd(700, 300, buffer, myfont, text_scale);

    pos_x = getWidth(pos_x/2);
    pos_y = getHeight(pos_y/2);
    pos_x = pos_x/dstrng*homedst*sin(abs_heading*0.0174532925)+width/2;
    pos_y = pos_y/dstrng*homedst*cos(abs_heading*0.0174532925)+height/2;   
    //pos_x = 500*sin(position_algle*0.0174532925)+960;
    //pos_y = 500*cos(position_algle*0.0174532925)+540;  
    float x[5] = {pos_x-getWidth(1.25)*scale, pos_x, pos_x+getWidth(1.25)*scale, pos_x, pos_x-getWidth(1.25)*scale};
    float y[5] = {pos_y-getWidth(1)*scale, getWidth(1)*scale+pos_y, pos_y-getWidth(1)*scale, getWidth(0.25)*scale+pos_y, pos_y-getWidth(1)*scale};
    rotatePoints(x, y, craft_heading, 5, pos_x+getWidth(1.25)*scale,pos_y+getWidth(1.25)*scale);
    Fill(255,255,0,getOpacity(COLOR)); // yellow
    Stroke(255,255,0,getOpacity(OUTLINECOLOR)); 
    Polygon(x, y, 5);
    Polyline(x, y, 5);
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    
}

void draw_throttle(uint16_t throttle, uint16_t throttle_target, int armed, float pos_x, float pos_y, float scale){

    float text_scale = getHeight(2) * scale;
    float width_element = getWidth(0.15) * scale;
    float height_element = getWidth(3) * scale;
    
    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    sprintf(buffer, "%d%%",  throttle);
    Text(getWidth(pos_x)+getWidth(0.2), getHeight(pos_y), buffer, myfont, text_scale);

#if THROTTLE_GAUGE == true
//save the current matrix so it can be reset later
VGfloat savedMatrix[9];
vgGetMatrix(savedMatrix);

//move the reference point to center of gauge (now 0,0)
Translate(getWidth(pos_x), getHeight(pos_y+3));

Stroke(COLOR);
//Circle(getWidth(pos_x+3), getHeight(pos_y+3), width_gauge);
CircleOutline(0, 0, height_element*2.2);

// 0 tick
Rotate(135);
Rect(0, height_element*.85, width_element, height_element*.2);
vgLoadMatrix(savedMatrix); //reload matrix

// Throttle target tick
Translate(getWidth(pos_x), getHeight(pos_y+3));
Rotate(135);
Rotate((throttle_target*2.35)*-1);
Stroke(COLOR_GOOD); //green
Fill(COLOR_GOOD); //green
Rect(0, height_element*.85, width_element, height_element*.2);
vgLoadMatrix(savedMatrix); //reload matrix

// 100 tick
Translate(getWidth(pos_x), getHeight(pos_y+3));
Rotate(270);
Stroke(COLOR_WARNING); //red
Fill(COLOR_WARNING); //red
Rect(0, height_element*.85, width_element, height_element*.2);
vgLoadMatrix(savedMatrix);//reload matrix

//set initial rotation at 135 degrees for 0 throttle
Translate(getWidth(pos_x), getHeight(pos_y+3));
Rotate(135);
//2.35 so that 100 percent throttle=235 degrees
Rotate((throttle*2.35)*-1);

if (armed == 1){
Stroke(COLOR_WARNING); //red
Fill(COLOR_WARNING);} //if armed needle is red
else {
Fill(COLOR);
Stroke(OUTLINECOLOR);}


if ( (throttle >throttle_target-2) && (throttle<throttle_target+2) ){
    Stroke(COLOR_GOOD);
    Fill(COLOR_GOOD);} //green
   

//draw needle
Rect(0, 0, width_element, height_element*.8);

//reset matrix so coordinate system is good for next draw
vgLoadMatrix(savedMatrix);

#endif
}

void draw_throttle_V2(uint16_t throttle, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth(".", myfont, text_scale)*1.3;
    VGfloat width_value_1 = TextWidth("0000", myfont, text_scale)*1.1;
    //Fill(COLOR);
    //Stroke(OUTLINECOLOR);
    
    int j = throttle;
    char* c;
    #if THROTTLE_V2_COMPLEX == true
    if (j <= 2 )   c = "᐀";
    else if (j > 2 && j <= 5 )   c = "ᐁ";
    else if (j > 5 && j <= 10 )   c = "ᐂ";
    else if (j > 10 && j <= 15 )   c = "ᐃ";
    else if (j > 15 && j <= 20 )   c = "ᐄ";
    else if (j > 20 && j <= 25 )   c = "ᐅ";
    else if (j > 25 && j <= 30 )   c = "ᐆ";
    else if (j > 30 && j <= 35 )   c = "ᐇ";
    else if (j > 35 && j <= 40 )   c = "ᐈ";
    else if (j > 40 && j <= 45 )   c = "ᐉ";
    else if (j > 45 && j <= 50 )   c = "ᐊ";
    else if (j > 50 && j <= 55 )   c = "ᐋ";
    else if (j > 55 && j <= 60 )   c = "ᐌ";
    else if (j > 60 && j <= 65 )   c = "ᐍ";
    else if (j > 65 && j <= 70 )   c = "ᐎ";
    else if (j > 70 && j <= 75 )   c = "ᐏ";
    else if (j > 75 && j <= 80 )   c = "ᐐ";
    else if (j > 80 && j <= 85 )   c = "ᐑ";
    else if (j > 85 && j <= 90 )   c = "ᐒ";
    else if (j > 90 && j <= 95 )   c = "ᐓ";
    else if (j == 99 )   c = "ᐔ";
    else  c = "ᐔ";

    
    Fill(0,190,90,getOpacity(COLOR)); // green
    Stroke(0,190,90,getOpacity(OUTLINECOLOR));
    TextEnd(getWidth(pos_x)+width_value, getHeight(pos_y)-getHeight(2.4)*scale, "ᐗ", osdicons, text_scale);
    
    Fill(255,165,0,getOpacity(COLOR)); // orange
    Stroke(255,165,0,getOpacity(OUTLINECOLOR));
    TextEnd(getWidth(pos_x)+width_value, getHeight(pos_y)-getHeight(2.4)*scale, "ᐘ", osdicons, text_scale); 
    
    Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    TextEnd(getWidth(pos_x)+width_value, getHeight(pos_y)-getHeight(2.4)*scale, "ᐙ", osdicons, text_scale); 
    
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    sprintf(buffer, "%d", throttle);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    TextEnd(getWidth(pos_x)+width_value, getHeight(pos_y)-getHeight(2.4)*scale, c, osdicons, text_scale);   
    #else
    if (j <= 10 )   c = "ᎋ";
    else if (j > 10 && j <= 20 )   c = "ᎌ";
    else if (j > 20 && j <= 30 )   c = "ᎍ";
    else if (j > 30 && j <= 40 )   c = "ᎎ";
    else if (j > 40 && j <= 50 )   c = "ᎏ";
    else if (j > 50 && j <= 60 )   c = "᎐";
    else if (j > 60 && j <= 70 )   c = "᎑";
    else if (j > 70 && j <= 80 )   c = "᎒";
    else  c = "᎓";
    sprintf(buffer, "%d", throttle);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    TextEnd(getWidth(pos_x)-width_value_1, getHeight(pos_y), c, osdicons, text_scale);
    

    #endif
    
}
