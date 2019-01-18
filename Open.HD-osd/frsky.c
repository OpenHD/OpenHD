#include "frsky.h"

#ifdef FRSKY

#include <stdint.h>

#include "telemetry.h"
#include <stdio.h>


int frsky_parse_buffer(frsky_state_t *state, telemetry_data_t *td, uint8_t *buf, int buflen) {
	int new_data = 0;
	int i;

	td->datarx++;

	for(i=0; i<buflen; ++i) {
		uint8_t ch = buf[i];
		switch(state->sm_state) {
			case 0:
				if(ch == 0x5e)
					state->sm_state = 1;
				break;
			case 1:
				if(ch == 0x5e)
					state->sm_state = 2;
				else
					state->sm_state = 0;
				break;
			case 2:
				if(ch == 0x5e) {
					state->pkg_pos = 0;
					new_data = new_data | frsky_interpret_packet(state, td);
				}
				else {
					if(state->pkg_pos >= sizeof(state->pkg)) {
						state->pkg_pos = 0;
						state->sm_state = 0;
					} else {
						state->pkg[state->pkg_pos] = ch;
						state->pkg_pos++;
					}
				}
				break;
			default:
				state->sm_state = 0;
			break;
		}
	}
	return new_data;
}

int frsky_interpret_packet(frsky_state_t *state, telemetry_data_t *td) {
	uint16_t data;
	int new_data = 1;

	data = *(uint16_t*)(state->pkg+1);
	switch(state->pkg[0]) {
		case ID_VOLTAGE_AMP:
//		{
			//uint16_t val = (state->pkg[2] >> 8) | ((state->pkg[1] & 0xf) << 8);
			//float battery = 3.0f * val / 500.0f;
			//td->voltage = battery;
			td->validmsgsrx++;
			td->voltage = data / 10.0f;
			printf("voltage:%f  ", td->voltage);
//		}
			break;
		case ID_ALTITUDE_BP:
			td->validmsgsrx++;
			td->baro_altitude = data;
			printf("baro altitude BP:%f  ", td->baro_altitude);
			break;
		case ID_ALTITUDE_AP:
			//td->baro_altitude += data/100;
			//printf("Baro Altitude AP:%f  ", td->baro_altitude);
			break;
		case ID_GPS_ALTITUDE_BP:
			td->validmsgsrx++;
			td->altitude = data;
			printf("GPS altitude:%f  ", td->altitude);
			break;
		case ID_LONGITUDE_BP:
			td->validmsgsrx++;
			td->longitude = data / 100;
			td->longitude += 1.0 * (data - td->longitude * 100) / 60;
			printf("longitude BP:%f  ", td->longitude);
			break;
		case ID_LONGITUDE_AP:
			td->validmsgsrx++;
			td->longitude +=  1.0 * data / 60 / 10000;
			printf("longitude AP:%f  ", td->longitude);
			break;
		case ID_LATITUDE_BP:
			td->validmsgsrx++;
			td->latitude = data / 100;
			td->latitude += 1.0 * (data - td->latitude * 100) / 60;
			printf("latitude BP:%f  ", td->latitude);
			break;
		case ID_LATITUDE_AP:
			td->validmsgsrx++;
			td->latitude +=  1.0 * data / 60 / 10000;
			printf("latitude AP:%f  ", td->latitude);
			break;
		case ID_COURSE_BP:
			td->validmsgsrx++;
			td->heading = data;
			printf("heading:%f  ", td->heading);
			break;
		case ID_GPS_SPEED_BP:
			td->validmsgsrx++;
			td->speed = 1.0 * data / 0.0194384449;
			printf("GPS speed BP:%f  ", td->speed);
			break;
		case ID_GPS_SPEED_AP:
			td->validmsgsrx++;
			td->speed += 1.0 * data / 1.94384449; //now we are in cm/s
			td->speed = td->speed / 100 / 1000 * 3600; //now we are in km/h
			printf("GPS speed AP:%f  ", td->speed);
			break;
		case ID_ACC_X:
			td->validmsgsrx++;
			td->x = data;
			printf("accel X:%d  ", td->x);
			break;
		case ID_ACC_Y:
			td->validmsgsrx++;
			td->y = data;
			printf("accel Y:%d  ", td->y);
			break;
		case ID_ACC_Z:
			td->validmsgsrx++;
			td->z = data;
			printf("accel Z:%d  ", td->z);
			break;
		case ID_E_W:
			td->validmsgsrx++;
			td->ew = data;
			printf("E/W:%d  ", td->ew);
			break;
		case ID_N_S:
			td->validmsgsrx++;
			td->ns = data;
			printf("N/S:%d  ", td->ns);
			break;
		default:
			new_data = 0;
			//printf("%x\n", pkg[0]);
		break;
	}
	printf("\n");
	return new_data;
}
#endif
