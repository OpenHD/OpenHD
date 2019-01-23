#pragma once

#include <stdint.h>
#include <time.h>
#include "osdconfig.h"

typedef struct {
        uint32_t received_packet_cnt;
        uint32_t wrong_crc_cnt;
        int8_t current_signal_dbm;
	int8_t type;
	int signal_good;
} wifi_adapter_rx_status_t;

typedef struct {
        uint32_t received_packet_cnt;
        uint32_t wrong_crc_cnt;
        int8_t current_signal_dbm;
	int8_t type;
	int signal_good;
} wifi_adapter_rx_status_t_osd;

typedef struct {
        uint32_t received_packet_cnt;
        uint32_t wrong_crc_cnt;
        int8_t current_signal_dbm;
	int8_t type;
	int signal_good;
} wifi_adapter_rx_status_t_uplink;

typedef struct {
        time_t last_update;
        uint32_t received_block_cnt;
        uint32_t damaged_block_cnt;
	uint32_t lost_packet_cnt;
	uint32_t received_packet_cnt;
	uint32_t lost_per_block_cnt;
        uint32_t tx_restart_cnt;
	uint32_t kbitrate;
        uint32_t wifi_adapter_cnt;
        wifi_adapter_rx_status_t adapter[8];
} wifibroadcast_rx_status_t;

typedef struct {
        time_t last_update;
        uint32_t received_block_cnt;
        uint32_t damaged_block_cnt;
	uint32_t lost_packet_cnt;
	uint32_t received_packet_cnt;
	uint32_t lost_per_block_cnt;
        uint32_t tx_restart_cnt;
	uint32_t kbitrate;
        uint32_t wifi_adapter_cnt;
        wifi_adapter_rx_status_t adapter[8];
} wifibroadcast_rx_status_t_osd;

typedef struct {
        time_t last_update;
        uint32_t received_block_cnt;
        uint32_t damaged_block_cnt;
	uint32_t lost_packet_cnt;
	uint32_t received_packet_cnt;
	uint32_t lost_per_block_cnt;
        uint32_t tx_restart_cnt;
	uint32_t kbitrate;
        uint32_t wifi_adapter_cnt;
        wifi_adapter_rx_status_t adapter[8];
} wifibroadcast_rx_status_t_rc;

typedef struct {
        time_t last_update;
        uint32_t received_block_cnt;
        uint32_t damaged_block_cnt;
	uint32_t lost_packet_cnt;
	uint32_t received_packet_cnt;
	uint32_t lost_per_block_cnt;
        uint32_t tx_restart_cnt;
	uint32_t kbitrate;
        uint32_t wifi_adapter_cnt;
        wifi_adapter_rx_status_t adapter[8];
} wifibroadcast_rx_status_t_uplink;

typedef struct {
    time_t last_update;
    uint8_t cpuload;
    uint8_t temp;
    uint32_t injected_block_cnt;
    uint32_t skipped_fec_cnt;
    uint32_t injection_fail_cnt;
    long long injection_time_block;
    uint16_t bitrate_kbit;
    uint16_t bitrate_measured_kbit;
    uint8_t cts;
    uint8_t undervolt;
} wifibroadcast_rx_status_t_sysair;


typedef struct {
	uint32_t validmsgsrx;
	uint32_t datarx;

	float voltage;
	float ampere;
	int32_t mah;
	float baro_altitude;
	float altitude;
	double longitude;
	double latitude;
	float heading;
	float cog; //course over ground
	float speed;
	float airspeed;
	float roll, pitch;
	uint8_t sats;
	uint8_t fix;
	uint8_t armed;
	uint8_t rssi;

	uint8_t home_fix;

//#if defined(FRSKY)
	int16_t x, y, z; // also needed for smartport
	int16_t ew, ns;
//#endif

#if defined(SMARTPORT)
	uint8_t swr;
	float rx_batt;
	float adc1;
	float adc2;
	float vario;
#endif

#if defined(MAVLINK)
	uint32_t mav_flightmode;
	float mav_climb;
#endif

#if defined(LTM)
// ltm S frame
	uint8_t ltm_status;
	uint8_t ltm_failsafe;
	uint8_t ltm_flightmode;
// ltm N frame
	uint8_t ltm_gpsmode;
	uint8_t ltm_navmode;
	uint8_t ltm_navaction;
	uint8_t ltm_wpnumber;
	uint8_t ltm_naverror;
// ltm X frame
	uint16_t ltm_hdop;
	uint8_t ltm_hw_status;
	uint8_t ltm_x_counter;
	uint8_t ltm_disarm_reason;
// ltm O frame
	float ltm_home_altitude;
	double ltm_home_longitude;
	double ltm_home_latitude;
	uint8_t ltm_osdon;
	uint8_t ltm_homefix;
#endif


	wifibroadcast_rx_status_t *rx_status;
	wifibroadcast_rx_status_t_osd *rx_status_osd;
	wifibroadcast_rx_status_t_rc *rx_status_rc;
	wifibroadcast_rx_status_t_uplink *rx_status_uplink;
	wifibroadcast_rx_status_t_sysair *rx_status_sysair;
} telemetry_data_t;

wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void);
wifibroadcast_rx_status_t_osd *telemetry_wbc_status_memory_open_osd(void);
wifibroadcast_rx_status_t_rc *telemetry_wbc_status_memory_open_rc(void);
wifibroadcast_rx_status_t_uplink *telemetry_wbc_status_memory_open_uplink(void);
wifibroadcast_rx_status_t_sysair *telemetry_wbc_status_memory_open_sysair(void);
