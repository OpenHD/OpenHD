#pragma once
#include "osdconfig.h"

#ifdef FRSKY

#include "telemetry.h"


// Data Ids (bp = before decimal point; af = after decimal point)
// Official data IDs
#define ID_GPS_ALTITUDE_BP 0x01
#define ID_GPS_ALTITUDE_AP 0x09
#define ID_TEMPRATURE1 0x02
#define ID_RPM 0x03
#define ID_FUEL_LEVEL 0x04
#define ID_TEMPRATURE2 0x05
#define ID_VOLT 0x06
#define ID_ALTITUDE_BP 0x10
#define ID_ALTITUDE_AP 0x21
#define ID_GPS_SPEED_BP 0x11
#define ID_GPS_SPEED_AP 0x19
#define ID_LONGITUDE_BP 0x12
#define ID_LONGITUDE_AP 0x1A
#define ID_E_W 0x22
#define ID_LATITUDE_BP 0x13
#define ID_LATITUDE_AP 0x1B
#define ID_N_S 0x23
#define ID_COURSE_BP 0x14
#define ID_COURSE_AP 0x1C
#define ID_DATE_MONTH 0x15
#define ID_YEAR 0x16
#define ID_HOUR_MINUTE 0x17
#define ID_SECOND 0x18
#define ID_ACC_X 0x24
#define ID_ACC_Y 0x25
#define ID_ACC_Z 0x26
#define ID_VOLTAGE_AMP 0x39
#define ID_VOLTAGE_AMP_BP 0x3A
#define ID_VOLTAGE_AMP_AP 0x3B
#define ID_CURRENT 0x28
// User defined data IDs
#define ID_GYRO_X 0x40
#define ID_GYRO_Y 0x41
#define ID_GYRO_Z 0x42
#define ID_VERT_SPEED 0x30 //opentx vario

typedef struct {
	int sm_state;
	uint8_t pkg[64];
	int pkg_pos;
} frsky_state_t;

int frsky_interpret_packet(frsky_state_t *state, telemetry_data_t *td);
int frsky_parse_buffer(frsky_state_t *state, telemetry_data_t *td, uint8_t *buf, int buflen);
#endif
