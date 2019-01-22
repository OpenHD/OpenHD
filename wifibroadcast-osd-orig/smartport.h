#include "osdconfig.h"

#ifdef SMARTPORT
#include "telemetry.h"

#define START_STOP 0x7e
#define DATA_FRAME 0x10

//Frsky DATA ID's
#define FR_ID_ALTITUDE 0x0100 //ALT_FIRST_ID
#define FR_ID_VARIO 0x0110 //VARIO_FIRST_ID
#define FR_ID_ALTITUDE 0x0100 //ALT_FIRST_ID
#define FR_ID_VARIO 0x0110 //VARIO_FIRST_ID
#define FR_ID_VFAS 0x0210 //VFAS_FIRST_ID
#define FR_ID_CURRENT 0x0200 //CURR_FIRST_ID
#define FR_ID_CELLS 0x0300 //CELLS_FIRST_ID
#define FR_ID_CELLS_LAST 0x030F //CELLS_LAST_ID
#define FR_ID_T1 0x0400 //T1_FIRST_ID
#define FR_ID_T2 0x0410 //T2_FIRST_ID
#define FR_ID_RPM 0x0500 //RPM_FIRST_ID
#define FR_ID_FUEL 0x0600 //FUEL_FIRST_ID
#define FR_ID_ACCX 0x0700 //ACCX_FIRST_ID
#define FR_ID_ACCY 0x0710 //ACCY_FIRST_ID
#define FR_ID_ACCZ 0x0720 //ACCZ_FIRST_ID
#define FR_ID_LATLONG 0x0800 //GPS_LONG_LATI_FIRST_ID
#define FR_ID_GPS_ALT 0x0820 //GPS_ALT_FIRST_ID
#define FR_ID_SPEED 0x0830 //GPS_SPEED_FIRST_ID
#define FR_ID_GPS_COURSE 0x0840 //GPS_COURS_FIRST_ID
#define FR_ID_GPS_TIME_DATE 0x0850 //GPS_TIME_DATE_FIRST_ID
#define FR_ID_GPS_SAT 0x0860 //GPS satellite count and fix state (own definition)
#define FR_ID_A3_FIRST 0x0900 //A3_FIRST_ID
#define FR_ID_A4_FIRST 0x0910 //A4_FIRST_ID
#define FR_ID_AIR_SPEED_FIRST 0x0A00 //AIR_SPEED_FIRST_ID
#define FR_ID_RSSI 0xF101 // used by the radio system
#define FR_ID_ADC1 0xF102 //ADC1_ID
#define FR_ID_ADC2 0xF103 //ADC2_ID
#define FR_ID_RXBATT 0xF104 // used by the radio system
#define FR_ID_SWR 0xF105 // used by the radio system
#define FR_ID_FIRMWARE 0xF106 // used by the radio system
#define FR_ID_VFAS 0x0210 //VFAS_FIRST_ID

typedef struct {
	uint16_t id;
	union {
		uint32_t u32;
		int32_t i32;
		uint16_t u16;
		int16_t i16;
		uint8_t u8;
		uint8_t b[4];
		int8_t i8;
		float f;
		}data;
	uint8_t crc;
	} tSPortData;

void smartport_read(telemetry_data_t *td, uint8_t *buf, int buflen);
uint8_t u8CheckCrcSPORT( uint8_t *t );
void smartport_check(telemetry_data_t *td, uint8_t *t); // Frsky-specific

#endif
