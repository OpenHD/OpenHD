/* #################################################################################################################
 * LightTelemetry protocol (LTM)
 *
 * Ghettostation one way telemetry protocol for really low bitrates (1200/2400 bauds).
 *
 * Protocol details: 3 different frames, little endian.
 *   G Frame (GPS position) (2hz @ 1200 bauds , 5hz >= 2400 bauds): 18BYTES
 *    0x24 0x54 0x47 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF  0xFF   0xC0
 *     $     T    G  --------LAT-------- -------LON---------  SPD --------ALT-------- SAT/FIX  CRC
 *   A Frame (Attitude) (5hz @ 1200bauds , 10hz >= 2400bauds): 10BYTES
 *     0x24 0x54 0x41 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xC0
 *      $     T   A   --PITCH-- --ROLL--- -HEADING-  CRC
 *   S Frame (Sensors) (2hz @ 1200bauds, 5hz >= 2400bauds): 11BYTES
 *     0x24 0x54 0x53 0xFF 0xFF  0xFF 0xFF    0xFF    0xFF      0xFF       0xC0
 *      $     T   S   VBAT(mv)  Current(ma)   RSSI  AIRSPEED  ARM/FS/FMOD   CRC
 * ################################################################################################################# */
#include "ltm.h"
#include <stdio.h>

#ifdef LTM
  static uint8_t LTMserialBuffer[LIGHTTELEMETRY_GFRAMELENGTH-4];
  static uint8_t LTMreceiverIndex;
  static uint8_t LTMcmd;
  static uint8_t LTMrcvChecksum;
  static uint8_t LTMreadIndex;
  static uint8_t LTMframelength;



uint8_t ltmread_u8()  {
  return LTMserialBuffer[LTMreadIndex++];
}

uint16_t ltmread_u16() {
  uint16_t t = ltmread_u8();
  t |= (uint16_t)ltmread_u8()<<8;
  return t;
}

uint32_t ltmread_u32() {
  uint32_t t = ltmread_u16();
  t |= (uint32_t)ltmread_u16()<<16;
  return t;
}

static enum _serial_state {
    IDLE,
    HEADER_START1,
    HEADER_START2,
    HEADER_MSGTYPE,
    HEADER_DATA
}
c_state = IDLE;

int ltm_read(telemetry_data_t *td, uint8_t *buf, int buflen) {
  int i;
  int render_data = 0;
  td->datarx++;

  for(i=0; i<buflen; ++i) {
    uint8_t c = buf[i];
    if (c_state == IDLE) {
      c_state = (c=='$') ? HEADER_START1 : IDLE;
    }
    else if (c_state == HEADER_START1) {
      c_state = (c=='T') ? HEADER_START2 : IDLE;
    }
    else if (c_state == HEADER_START2) {
      switch (c) {
         case 'G':
           LTMframelength = LIGHTTELEMETRY_GFRAMELENGTH;
           c_state = HEADER_MSGTYPE;
           break;
         case 'A':
           LTMframelength = LIGHTTELEMETRY_AFRAMELENGTH;
           c_state = HEADER_MSGTYPE;
           break;
         case 'S':
           LTMframelength = LIGHTTELEMETRY_SFRAMELENGTH;
           c_state = HEADER_MSGTYPE;
           break;
         case 'O':
           LTMframelength = LIGHTTELEMETRY_OFRAMELENGTH;
           c_state = HEADER_MSGTYPE;
           break;
         case 'N':
           LTMframelength = LIGHTTELEMETRY_NFRAMELENGTH;
           c_state = HEADER_MSGTYPE;
           break;
         case 'X':
           LTMframelength = LIGHTTELEMETRY_XFRAMELENGTH;
           c_state = HEADER_MSGTYPE;
           break;
         default:
           c_state = IDLE;
      }
      LTMcmd = c;
      LTMreceiverIndex=0;
    }
    else if (c_state == HEADER_MSGTYPE) {
	  if(LTMreceiverIndex == 0) {
	  LTMrcvChecksum = c;
	  }
	  else {
	  LTMrcvChecksum ^= c;
	  }
      if(LTMreceiverIndex == LTMframelength-4) {   // received checksum byte
        if(LTMrcvChecksum == 0) {
            if(ltm_check(td) == 1) render_data = 1;
            c_state = IDLE;
        }
        else { // wrong checksum, drop packet
        c_state = IDLE;
        }
      }
      else LTMserialBuffer[LTMreceiverIndex++]=c;
    }
  }
  return render_data;
}

// --------------------------------------------------------------------------------------
// Decoded received commands
int ltm_check(telemetry_data_t *td) {
  LTMreadIndex = 0;
  int render_data = 0;

  if (LTMcmd==LIGHTTELEMETRY_GFRAME)  {
    td->latitude = (double)((int32_t)ltmread_u32())/10000000;
    td->longitude = (double)((int32_t)ltmread_u32())/10000000;
    uint8_t uav_groundspeedms = ltmread_u8();
    td->speed = (float)(uav_groundspeedms * 3.6f); // convert to kmh
    td->altitude = (float)((int32_t)ltmread_u32())/100.0f;
    uint8_t ltm_satsfix = ltmread_u8();
    td->sats = (ltm_satsfix >> 2) & 0xFF;
    td->fix = ltm_satsfix & 0b00000011;

    td->validmsgsrx++;
    printf("LTM G FRAME: ");
    printf("fix:%d  ", td->fix);
    printf("sats:%d  ", td->sats);
    printf("altitude:%.2f  ", td->altitude);
    printf("latitude:%.2f  ", td->latitude);
    printf("longitude:%.2f  ", td->longitude);
    printf("groundspeed:%.2f  ", td->speed);

  }else if (LTMcmd==LIGHTTELEMETRY_AFRAME)  {
    td->pitch = (int16_t)ltmread_u16();
    td->roll =  (int16_t)ltmread_u16();
    td->heading = (float)((int16_t)ltmread_u16());
    if (td->heading < 0 ) td->heading = td->heading + 360; //convert from -180/180 to 0/360°
    td->validmsgsrx++;
    printf("LTM A FRAME: ");
    printf("heading:%.2f  ", td->heading);
    printf("roll:%.2f  ", td->roll);
    printf("pitch:%.2f  ", td->pitch);
    render_data = 1;

  }else if (LTMcmd==LIGHTTELEMETRY_OFRAME)  {
    td->ltm_home_latitude = (double)((int32_t)ltmread_u32())/10000000;
    td->ltm_home_longitude = (double)((int32_t)ltmread_u32())/10000000;
    td->ltm_home_altitude = (float)((int32_t)ltmread_u32())/100.0f;
    td->ltm_osdon = ltmread_u8();
    td->ltm_homefix = ltmread_u8();
    td->validmsgsrx++;
    printf("LTM O FRAME: ");
    printf("home_altitude:%.2f  ", td->ltm_home_latitude);
    printf("home_latitude:%.2f  ", td->ltm_home_longitude);
    printf("home_longitude:%.2f  ", td->ltm_home_altitude);
    printf("osdon:%d  ", td->ltm_osdon);
    printf("homefix:%d  ", td->ltm_homefix);

  }else if (LTMcmd==LIGHTTELEMETRY_XFRAME)  {
    //HDOP 		uint16 HDOP * 100
    //hw status 	uint8
    //LTM_X_counter 	uint8
    //Disarm Reason 	uint8
    //(unused) 		1byte
    td->ltm_hdop = (float)((uint16_t)ltmread_u16())/10000.0f;
    printf("LTM X FRAME:\n");
    printf("GPS hdop:%.2f  ", td->ltm_hdop);

  }else if (LTMcmd==LIGHTTELEMETRY_SFRAME)  {
    //Vbat 			uint16, mV
    //Battery Consumption 	uint16, mAh
    //RSSI 			uchar
    //Airspeed 			uchar, m/s
    //Status 			uchar
    td->voltage = (float)ltmread_u16()/1000.0f;
    td->mah = (float)ltmread_u16()/1000.0f;
    td->rssi = ltmread_u8();

    uint8_t uav_airspeedms = ltmread_u8();
    td->airspeed = (float)(uav_airspeedms * 3.6f); // convert to kmh

    uint8_t ltm_armfsmode = ltmread_u8();
    td->armed = ltm_armfsmode & 0b00000001;
    td->ltm_failsafe = (ltm_armfsmode >> 1) & 0b00000001;
    td->ltm_flightmode = (ltm_armfsmode >> 2) & 0b00111111;

    td->validmsgsrx++;
    printf("LTM S FRAME: ");
    printf("voltage:%.2f  ", td->voltage);
    printf("mAh:%.2f  ", td->mah);
    printf("rssi:%.2f  ", td->rssi);
    printf("airspeed:%.2f  ", td->airspeed);
    printf("arm:%d  ", td->armed);
    printf("failsafe:%d  ", td->ltm_failsafe);
    printf("flightmode:%d  ", td->ltm_flightmode);
  }
  printf("\n");
  return render_data;
}
#endif
