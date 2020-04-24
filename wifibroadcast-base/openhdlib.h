#pragma once

#include "wifibroadcast.h"
#include <stdint.h>
#include <stdlib.h>



#define MAX_PENUMBRA_INTERFACES 8
#define SWITCH_COUNT 16



typedef struct {
    uint32_t received_packet_cnt;
    uint32_t wrong_crc_cnt;

    int8_t current_signal_dbm;

    /*
     * 0 = Atheros
     * 
     * 1 = Ralink
     * 
     */
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




typedef struct
{
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

    wifi_adapter_rx_status_t adapter[MAX_PENUMBRA_INTERFACES];
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

    wifi_adapter_rx_status_t adapter[MAX_PENUMBRA_INTERFACES];
} wifibroadcast_rx_status_t_uplink;




typedef struct {
    time_t last_update;
    
    uint32_t received_block_cnt;
    uint32_t damaged_block_cnt;

    uint32_t lost_packet_cnt;
    uint32_t received_packet_cnt;

    uint32_t lost_per_block_cnt;

    uint32_t tx_restart_cnt;

    uint32_t kbitrate;

    uint32_t current_air_datarate_kbit;

    uint32_t wifi_adapter_cnt;

    wifi_adapter_rx_status_t adapter[MAX_PENUMBRA_INTERFACES];
} wifibroadcast_rx_status_t;




typedef struct {
    time_t last_update;

    uint32_t injected_block_cnt;

    uint32_t skipped_fec_cnt;
    uint32_t injection_fail_cnt;
    
    long long injection_time_block;
} wifibroadcast_tx_status_t;




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

    wifi_adapter_rx_status_t adapter[MAX_PENUMBRA_INTERFACES];
} wifibroadcast_rx_status_t_rc;




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
    int valid;

    int crc_correct;

    /*
     * This is the actual length of the packet stored in data
     * 
     */
    size_t len;

    uint8_t *data;
} packet_buffer_t;




/*
 * This sits at the payload of the wifi packet (outside of FEC)
 * 
 */
typedef struct {
    uint32_t sequence_number;
} __attribute__((packed)) wifi_packet_header_t;



/*
 * This sits at the data payload (which is usually right after the wifi_packet_header_t) (inside of FEC)
 * 
 */
typedef struct {
    uint32_t data_length;
} __attribute__((packed)) payload_header_t;




packet_buffer_t *lib_alloc_packet_buffer_list(size_t num_packets, size_t packet_length);



/*
 * From OSD for rssitx and tx
 * 
 */
typedef struct {
    wifibroadcast_rx_status_t *rx_status;
    wifibroadcast_rx_status_t_rc *rx_status_rc;
    wifibroadcast_tx_status_t *tx_status;
} telemetry_data_t;



wifibroadcast_rx_status_t *telemetry_wbc_status_memory_open(void);
wifibroadcast_rx_status_t_rc *telemetry_wbc_status_memory_open_rc(void);
wifibroadcast_tx_status_t *telemetry_wbc_status_memory_open_tx(void);

//wifibroadcast_rx_status_t_sysair *telemetry_wbc_status_memory_open_sysair(void);
