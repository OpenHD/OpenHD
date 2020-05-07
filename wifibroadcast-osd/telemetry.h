#pragma once

#include "openhdlib.h"
#include <stdint.h>
#include <time.h>


extern FILE * telemetry_file;

typedef struct {
    uint32_t validmsgsrx;
    uint32_t datarx;

    double datarate;

    float voltage;
    float ampere;

    int32_t mah;

    float msl_altitude;
    float rel_altitude;

    double longitude;
    double latitude;

    float heading;
    float cog;

    float speed;
    float airspeed;

    uint16_t throttle;

    float roll, pitch;

    uint8_t sats;
    uint8_t fix;
    uint16_t hdop;

    uint8_t armed;

    uint8_t rssi;

    uint8_t home_fix;
    
    float mav_climb;
    
    double vx;
    double vy;
    double vz;

    // also needed for smartport
    //#if defined(FRSKY)
    int16_t x, y, z; 
    int16_t ew, ns;
    //#endif

    /*
     * Smartport
     */
    uint8_t swr;
    float rx_batt;
    float adc1;
    float adc2;
    // duplicated down in vot section
    //float vario;


    /*
     * Mavlink
     */
    uint32_t mav_flightmode;
    //float mav_climb;
    uint32_t version;
    uint16_t vendor;
    uint16_t product;
    //double vx;
    //double vy;
    //double vz;
    //uint16_t hdop;
    uint16_t servo1;
    uint16_t mission_current_seq;
    uint32_t SP;
    uint32_t SE;
    uint32_t SH;



    /*
     * LTM
     */

    // LTM S frame
    uint8_t ltm_status;
    uint8_t ltm_failsafe;
    uint8_t ltm_flightmode;

    // LTM N frame
    uint8_t ltm_gpsmode;
    uint8_t ltm_navmode;
    uint8_t ltm_navaction;
    uint8_t ltm_wpnumber;
    uint8_t ltm_naverror;

    // LTM X frame
    //uint16_t ltm_hdop;
    uint8_t ltm_hw_status;
    uint8_t ltm_x_counter;
    uint8_t ltm_disarm_reason;
    
    // LTM O frame
    float ltm_home_altitude;
    double ltm_home_longitude;
    double ltm_home_latitude;
    uint8_t ltm_osdon;
    uint8_t ltm_homefix;


    /*
     * VOT
     */
    uint8_t flightmode;
    float vtxvoltage;
    float camvoltage;
    float rxvoltage;
    float vario;
    int16_t rpm;
    int16_t temp;
    uint16_t mahconsumed;
    uint16_t compassdegrees;
    uint8_t lq;
    float distance;
    uint16_t coursedegrees;
    //float hdop;

    wifibroadcast_rx_status_t *rx_status;
    wifibroadcast_rx_status_t_osd *rx_status_osd;
    wifibroadcast_rx_status_t_rc *rx_status_rc;
    wifibroadcast_rx_status_t_uplink *rx_status_uplink;
    wifibroadcast_rx_status_t_sysair *rx_status_sysair;
} telemetry_data_t_osd;


void telemetry_init(telemetry_data_t_osd *td);


wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void);
wifibroadcast_rx_status_t_osd *telemetry_wbc_status_memory_open_osd(void);
wifibroadcast_rx_status_t_rc *telemetry_wbc_status_memory_open_rc(void);
wifibroadcast_rx_status_t_uplink *telemetry_wbc_status_memory_open_uplink(void);
wifibroadcast_rx_status_t_sysair *telemetry_wbc_status_memory_open_sysair(void);
