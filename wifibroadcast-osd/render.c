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
float total_dist; 
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

int fecs_skipped_last;
int injection_failed_last;
int tx_restart_count_last;

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


void render(telemetry_data_t *td, uint8_t cpuload_gnd, uint8_t temp_gnd, uint8_t undervolt, int osdfps) {
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
    draw_kbitrate(td->rx_status_sysair->cts, td->rx_status->kbitrate, td->rx_status_sysair->bitrate_measured_kbit, td->rx_status_sysair->bitrate_kbit, td->rx_status_sysair->skipped_fec_cnt, td->rx_status_sysair->injection_fail_cnt,td->rx_status_sysair->injection_time_block,KBITRATE_POS_X, KBITRATE_POS_Y, KBITRATE_SCALE * GLOBAL_SCALE);
#endif


#ifdef SYS
    draw_sys(td->rx_status_sysair->cpuload, td->rx_status_sysair->temp, cpuload_gnd, temp_gnd, SYS_POS_X, SYS_POS_Y, SYS_SCALE * GLOBAL_SCALE);
#endif


#ifdef FLIGHTMODE
    #ifdef MAVLINK
    draw_mode(td->mav_flightmode, td->armed, FLIGHTMODE_POS_X, FLIGHTMODE_POS_Y, FLIGHTMODE_SCALE * GLOBAL_SCALE);
    #endif
#endif


#if defined(RSSI)
    draw_rssi(td->rssi, RSSI_POS_X, RSSI_POS_Y, RSSI_SCALE * GLOBAL_SCALE);
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


#ifdef BAROALT
    draw_baroalt(td->baro_altitude, BAROALT_POS_X, BAROALT_POS_Y, BAROALT_SCALE * GLOBAL_SCALE);
#endif

#ifdef GPSALT
    draw_gpsalt(td->altitude, GPSALT_POS_X, GPSALT_POS_Y, GPSALT_SCALE * GLOBAL_SCALE);
#endif


#ifdef COURSE_OVER_GROUND
    draw_cog((int)td->cog, COURSE_OVER_GROUND_POS_X, COURSE_OVER_GROUND_POS_Y, COURSE_OVER_GROUND_SCALE * GLOBAL_SCALE);
#endif


#ifdef ALTLADDER
    #if IMPERIAL == true
    #if ALTLADDER_USEBAROALT == true
    draw_alt_ladder((int)(td->baro_altitude * TO_FEET), ALTLADDER_POS_X, 50, ALTLADDER_SCALE * GLOBAL_SCALE);
    #else
    draw_alt_ladder((int)(td->altitude * TO_FEET), ALTLADDER_POS_X, 50, ALTLADDER_SCALE * GLOBAL_SCALE);
    #endif
    #else
    #if ALTLADDER_USEBAROALT == true
    draw_alt_ladder((int)td->baro_altitude, ALTLADDER_POS_X, 50, ALTLADDER_SCALE * GLOBAL_SCALE);
    #else
    draw_alt_ladder((int)td->altitude, ALTLADDER_POS_X, 50, ALTLADDER_SCALE * GLOBAL_SCALE);
    #endif
    #endif
#endif


#ifdef SPEEDLADDER
    #if IMPERIAL == true
    #if SPEEDLADDER_USEAIRSPEED == true
    draw_speed_ladder((int)td->airspeed*TO_MPH, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE);
    #else
    draw_speed_ladder((int)td->speed*TO_MPH, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE);
    #endif
    #else
    #if SPEEDLADDER_USEAIRSPEED == true
    draw_speed_ladder((int)td->airspeed, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE);
    #else
    draw_speed_ladder((int)td->speed, SPEEDLADDER_POS_X, 50, SPEEDLADDER_SCALE * GLOBAL_SCALE);
    #endif
    #endif
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
    draw_TOTAL_AMPS(td->ampere, TOTAL_AMPS_POS_X, TOTAL_AMPS_POS_Y, TOTAL_AMPS_SCALE * GLOBAL_SCALE);
#endif

#ifdef TOTAL_DIST
    draw_TOTAL_DIST((int)td->speed, TOTAL_DIST_POS_X, TOTAL_DIST_POS_Y, TOTAL_DIST_SCALE * GLOBAL_SCALE);
 #endif

#ifdef TOTAL_TIME
    draw_TOTAL_TIME((int)td->speed, TOTAL_TIME_POS_X, TOTAL_TIME_POS_Y, TOTAL_TIME_SCALE * GLOBAL_SCALE);
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
    #elif defined(MAVLINK) || defined(SMARTPORT) || defined(LTM)
    draw_sat(td->sats, td->fix, td->hdop, SAT_POS_X, SAT_POS_Y, SAT_SCALE * GLOBAL_SCALE);
    #endif
#endif


#ifdef BATT_GAUGE
    draw_batt_gauge(((td->voltage/CELLS)-CELL_MIN)/(CELL_MAX-CELL_MIN)*100, BATT_GAUGE_POS_X, BATT_GAUGE_POS_Y, BATT_GAUGE_SCALE * GLOBAL_SCALE);
#endif


#ifdef AHI
#if defined(FRSKY) || defined(SMARTPORT)
    float x_val, y_val, z_val;
    x_val = td->x;
    y_val = td->y;
    z_val = td->z;
    #if AHI_SWAP_ROLL_AND_PITCH == true
    draw_ahi(AHI_INVERT_ROLL * TO_DEG * (atan(y_val / sqrt((x_val*x_val) + (z_val*z_val)))), AHI_INVERT_PITCH * TO_DEG * (atan(x_val / sqrt((y_val*y_val)+(z_val*z_val)))), AHI_SCALE * GLOBAL_SCALE);
    #else
    draw_ahi(AHI_INVERT_ROLL * TO_DEG * (atan(x_val / sqrt((y_val*y_val) + (z_val*z_val)))), AHI_INVERT_PITCH * TO_DEG * (atan(y_val / sqrt((x_val*x_val)+(z_val*z_val)))), AHI_SCALE * GLOBAL_SCALE);
    #endif // AHI_SWAP_ROLL_AND_PITCH
#elif defined(LTM) || defined(MAVLINK)
    draw_ahi(AHI_INVERT_ROLL * td->roll, AHI_INVERT_PITCH * td->pitch, AHI_SCALE * GLOBAL_SCALE);
#endif //protocol
#endif //AHI

    End(); // Render end (causes everything to be drawn on next vsync)
}


void draw_mode(int mode, int armed, float pos_x, float pos_y, float scale){
    //autopilot mode, mavlink specific, could be used if mode is in telemetry data of other protocols as well
    float text_scale = getWidth(2) * scale;

    if (armed == 1){
    switch (mode) {
    #if COPTER == true
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
    case 15: sprintf(buffer, "GUID"); break;
    case 16: sprintf(buffer, "INIT"); break;
    case 255: sprintf(buffer, "-----"); break;
    #endif
    default: sprintf(buffer, "-----"); break; // TODO: find out why strange numbers when using zs6bujs telemetry logs, default to something more sensible like "unknown mode"
    }
    } else {
    switch (mode) {
    #if COPTER == true
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
    case 15: sprintf(buffer, "[GUID]"); break;
    case 16: sprintf(buffer, "[INIT]"); break;
    case 255: sprintf(buffer, "[-----]"); break;
    #endif
    default: sprintf(buffer, "[-----]"); break; // TODO: find out why strange numbers when using zs6bujs telemetry logs, default to something more sensible like "unknown mode"
    }
    }
    TextMid(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}


void draw_rssi(int rssi, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale);

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale * 0.6);

    sprintf(buffer, "%02d", rssi);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    Text(getWidth(pos_x), getHeight(pos_y), "%", myfont, text_scale*0.6);
}



void draw_cog(int cog, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("000°", myfont, text_scale);

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale*0.7);

    sprintf(buffer, "%d°", cog);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
}



void draw_climb(float climb, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("-00.0", myfont, text_scale);

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale*0.6);
    if (climb > 0.0f) {
    sprintf(buffer, "+%.1f", climb);
    } else {
    sprintf(buffer, "%.1f", climb);
    }
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "m/s", myfont, text_scale*0.6);
}



void draw_baroalt(float baroalt, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;

    #if IMPERIAL == true
    VGfloat width_value = TextWidth("0000", myfont, text_scale);
    sprintf(buffer, "%.0f", baroalt*TO_FEET);
    #else
    VGfloat width_value = TextWidth("000.0", myfont, text_scale);
    sprintf(buffer, "%.1f", baroalt);
    #endif
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    TextEnd(getWidth(pos_x)-width_value-getWidth(0.3)*scale, getHeight(pos_y), " ", osdicons, text_scale*0.7);

    #if IMPERIAL == true
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "ft", myfont, text_scale*0.6);
    #else
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "m", myfont, text_scale*0.6);
    #endif
}



void draw_gpsalt(float gpsalt, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;

    #if IMPERIAL == true
    VGfloat width_value = TextWidth("0000", myfont, text_scale);
    sprintf(buffer, "%.0f", gpsalt*TO_FEET);
    #else
    VGfloat width_value = TextWidth("000.0", myfont, text_scale);
    sprintf(buffer, "%.1f", gpsalt);
    #endif
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    TextEnd(getWidth(pos_x)-width_value-getWidth(0.3)*scale, getHeight(pos_y), " ", osdicons, text_scale*0.7);

    #if IMPERIAL == true
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "ft", myfont, text_scale*0.6);
    #else
    Text(getWidth(pos_x)+getWidth(0.4), getHeight(pos_y), "m", myfont, text_scale*0.6);
    #endif
}



void draw_airspeed(int airspeed, float pos_x, float pos_y, float scale){
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
    float text_scale = getWidth(2) * scale;
    VGfloat width_value = TextWidth("100", myfont, text_scale);
    VGfloat width_speedo = TextWidth("", osdicons, text_scale*0.65);

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale*0.65);
    TextEnd(getWidth(pos_x)-width_value-width_speedo, getHeight(pos_y), "", osdicons, text_scale*0.7);

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
    Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
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



void draw_kbitrate(int cts, int kbitrate, uint16_t kbitrate_measured_tx, uint16_t kbitrate_tx, uint32_t fecs_skipped, uint32_t injection_failed, long long injection_time,float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat height_text_small = TextHeight(myfont, text_scale*0.6)+getHeight(0.3)*scale;
    VGfloat width_value = TextWidth("10.0", myfont, text_scale);
    VGfloat width_symbol = TextWidth("", osdicons, text_scale*0.8);
//    VGfloat width_value_ms = TextWidth("0.0", myfont, text_scale*0.6);

    float mbit = (float)kbitrate / 1000;
    float mbit_measured = (float)kbitrate_measured_tx / 1000;
    float mbit_tx = (float)kbitrate_tx / 1000;
    float ms = (float)injection_time / 1000;

    if (cts == 0) {
    sprintf(buffer, "%.1f (%.1f)", mbit_tx, mbit_measured);
    } else {
    sprintf(buffer, "%.1f (%.1f) CTS", mbit_tx, mbit_measured);
    }
    Text(getWidth(pos_x)-width_value-width_symbol, getHeight(pos_y)-height_text_small, buffer, myfont, text_scale*0.6);

    if (fecs_skipped > fecs_skipped_last) {
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    } else {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }
    fecs_skipped_last = fecs_skipped;

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "", osdicons, text_scale * 0.8);

    if (mbit > mbit_measured*0.98) {
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    } else if (mbit > mbit_measured*0.90) {
        Fill(229,255,20,getOpacity(COLOR)); // yellow
    Stroke(229,255,20,getOpacity(OUTLINECOLOR));
    } else {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }
    sprintf(buffer, "%.1f", mbit);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    Text(getWidth(pos_x), getHeight(pos_y), "Mbit", myfont, text_scale*0.6);

//    sprintf(buffer, "%.1f", ms);
//    TextEnd(getWidth(pos_x)-width_value-width_symbol+width_value_ms, getHeight(pos_y)-height_text-height_text_small, buffer, myfont, text_scale*0.6);
//    sprintf(buffer, "ms");
//    Text(getWidth(pos_x)-width_value-width_symbol+width_value_ms, getHeight(pos_y)-height_text-height_text_small, buffer, myfont, text_scale*0.4);

    sprintf(buffer, "%d/%d",injection_failed,fecs_skipped);
    Text(getWidth(pos_x)-width_value-width_symbol, getHeight(pos_y)-height_text_small-height_text_small, buffer, myfont, text_scale*0.6);
}



void draw_sys(uint8_t cpuload_air, uint8_t temp_air, uint8_t cpuload_gnd, uint8_t temp_gnd, float pos_x, float pos_y, float scale) {
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    VGfloat width_value = TextWidth("00", myfont, text_scale) + getWidth(0.5)*scale;
    VGfloat width_label = TextWidth("%", myfont, text_scale*0.6) + getWidth(0.5)*scale;
    VGfloat width_ag = TextWidth("A", osdicons, text_scale*0.4) - getWidth(0.3)*scale;

    TextEnd(getWidth(pos_x)-width_value-width_ag, getHeight(pos_y), "", osdicons, text_scale*0.7);

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y), "A", myfont, text_scale*0.4);

    if (cpuload_air > 95) {
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    } else if (cpuload_air > 85) {
        Fill(229,255,20,getOpacity(COLOR)); // yellow
    Stroke(229,255,20,getOpacity(OUTLINECOLOR));
    } else {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }
    sprintf(buffer, "%d", cpuload_air);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    sprintf(buffer, "%%");
    Text(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale*0.6);

    if (temp_air > 79) {
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    } else if (temp_air > 74) {
        Fill(229,255,20,getOpacity(COLOR)); // yellow
    Stroke(229,255,20,getOpacity(OUTLINECOLOR));
    } else {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }
    sprintf(buffer, "%d°", temp_air);
    TextEnd(getWidth(pos_x)+width_value+width_label+getWidth(0.7), getHeight(pos_y), buffer, myfont, text_scale);
    Fill(COLOR);
    Stroke(OUTLINECOLOR);

    TextEnd(getWidth(pos_x)-width_value-width_ag, getHeight(pos_y)-height_text, "", osdicons, text_scale*0.7);

    TextEnd(getWidth(pos_x)-width_value, getHeight(pos_y)-height_text, "G", myfont, text_scale*0.4);

    if (cpuload_gnd > 95) {
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    } else if (cpuload_gnd > 85) {
        Fill(229,255,20,getOpacity(COLOR)); // yellow
    Stroke(229,255,20,getOpacity(OUTLINECOLOR));
    } else {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }
    sprintf(buffer, "%d", cpuload_gnd);
    TextEnd(getWidth(pos_x), getHeight(pos_y)-height_text, buffer, myfont, text_scale);

    Text(getWidth(pos_x), getHeight(pos_y)-height_text, "%", myfont, text_scale*0.6);

    if (temp_gnd > 79) {
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
    } else if (temp_gnd > 74) {
        Fill(229,255,20,getOpacity(COLOR)); // yellow
    Stroke(229,255,20,getOpacity(OUTLINECOLOR));
    } else {
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    }
    sprintf(buffer, "%d°", temp_gnd);
    TextEnd(getWidth(pos_x)+width_value+width_label+getWidth(0.7), getHeight(pos_y)-height_text, buffer, myfont, text_scale);
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
            c = "N";
            break;
        case 90:
            draw = true;
            c = "E";
            break;
        case 180:
            draw = true;
            c = "S";
            break;
        case 270:
            draw = true;
            c = "W";
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
 
    // get time passed since last rendering
    long time_diff = current_ts() - amps_ts;
    amps_ts = current_ts();
    total_amps = total_amps + current*(float)time_diff/3600;
 
  
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    sprintf(buffer, "%5.0f", total_amps);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), " mAh", myfont, text_scale*0.6);
 
}
void draw_TOTAL_DIST(int gpsspeed, float pos_x, float pos_y, float scale){
 
    // get time passed since last rendering
    long time_diff = current_ts() - dist_ts;
    dist_ts = current_ts();
    total_dist = total_dist + gpsspeed*(float)time_diff/3600000;
 
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    sprintf(buffer, "%3.1f", total_dist);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), " km", myfont, text_scale*0.6);
 
}
void draw_TOTAL_TIME(int gpsspeed, float pos_x, float pos_y, float scale){
 
    // get time passed since last rendering
    long time_diff = current_ts() - time_ts;
    
    time_ts = current_ts();
    if(gpsspeed>0){	
        total_time = total_time + (float)time_diff/60000; // flying time in minutes	    if ( (armed == 1) && (current > 3) )
    }
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    sprintf(buffer, "%3.0f:%02d", total_time, (int)(total_time*60) % 60);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);
    Text(getWidth(pos_x), getHeight(pos_y), " mins", myfont, text_scale*0.6);
 
}

void draw_position(float lat, float lon, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;
    VGfloat height_text = TextHeight(myfont, text_scale)+getHeight(0.3)*scale;
    VGfloat width_value = TextWidth("-100.000000", myfont, text_scale);

    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y), "  ", osdicons, text_scale*0.6);

    sprintf(buffer, "%.6f", lon);
    TextEnd(getWidth(pos_x), getHeight(pos_y), buffer, myfont, text_scale);

    TextEnd(getWidth(pos_x) - width_value, getHeight(pos_y)+height_text, "  ", osdicons, text_scale*0.6);

    sprintf(buffer, "%.6f", lat);
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



void draw_alt_ladder(int alt, float pos_x, float pos_y, float scale){
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

    sprintf(buffer, "%d", alt); // large alt number
    Text(pxlabel+width_ladder_value+width_symbol, getHeight(pos_y)-offset_alt_value, buffer, myfont, text_scale*2);
    Text(pxlabel+width_ladder_value, getHeight(pos_y)-offset_symbol, "", osdicons, text_scale*2);

    int k;
    for (k = (int) (alt - range / 2); k <= alt + range / 2; k++) {
    int y = getHeight(pos_y) + (k - alt) * ratio_alt;
    if (k % 50 == 0) {
        Rect(px-width_element, y, width_element*2, height_element);
        sprintf(buffer, "%d", k);
        Text(pxlabel, y-offset_text_ladder, buffer, myfont, text_scale);
    } else if (k % 5 == 0) {
            Rect(px-width_element, y, width_element, height_element);
        }
    }
}



void draw_speed_ladder(int speed, float pos_x, float pos_y, float scale){
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
    TextEnd(pxlabel-width_ladder_value-width_symbol, getHeight(pos_y)-offset_speed_value, buffer, myfont, text_scale*2);
    TextEnd(pxlabel-width_ladder_value, getHeight(pos_y)-offset_symbol, "", osdicons, text_scale*2);

    int k;
    for (k = (int) (speed - range_half); k <= speed + range_half; k++) {
    int y = getHeight(pos_y) + (k - speed) * ratio_speed;
    if (k % 5 == 0) { // wide element plus number label every 5 'ticks' on the scale
        Rect(px-width_element, y, width_element*2, height_element);
        if (k >= 0) {
        sprintf(buffer, "%d", k);
        TextEnd(pxlabel, y-offset_text_ladder, buffer, myfont, text_scale);
        }
    } else if (k % 1 == 0) { // narrow element every single 'tick' on the scale
        Rect(px, y, width_element, height_element);
    }
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
    sprintf(buffer, "%d", signal);
    }
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
    
    Text(getWidth(pos_x)-width_value-width_symbol, getHeight(pos_y)-height_text, buffer, myfont, text_scale*0.6);

    TextEnd(getWidth(pos_x)-width_value - getWidth(0.3) * scale, getHeight(pos_y), "", osdicons, text_scale * 0.7);

    switch (lost_per_block) {
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
    #endif
}



void draw_sat(int sats, int fixtype, int hdop, float pos_x, float pos_y, float scale){
    float text_scale = getWidth(2) * scale;

    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    StrokeWidth(OUTLINEWIDTH);
    if (fixtype < 2){
        Fill(255,20,20,getOpacity(COLOR)); // red
    Stroke(255,20,20,getOpacity(OUTLINECOLOR));
        TextEnd(getWidth(pos_x), getHeight(pos_y), "", osdicons, text_scale*0.7);
    }else{
        Fill(COLOR);
    Stroke(OUTLINECOLOR);
    TextEnd(getWidth(pos_x), getHeight(pos_y), "", osdicons, text_scale*0.7);
    }

    #if defined(LTM) || defined(MAVLINK)
    Fill(COLOR);
    Stroke(OUTLINECOLOR);
    float decimal_hdop = (float) hdop/100;
    sprintf(buffer, "%d(%.2f)", sats, decimal_hdop);
    Text(getWidth(pos_x)+getWidth(0.2), getHeight(pos_y), buffer, myfont, text_scale);
    #endif
}



void draw_batt_gauge(int remaining, float pos_x, float pos_y, float scale){
    //new stuff from fritz walter https://www.youtube.com/watch?v=EQ01b3aJ-rk
    //prevent black empty indicator to draw left to battery
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
