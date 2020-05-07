/* #################################################################################################################
/ Vector Open Telemetry Revision 0
// NOTES:
// 1) UART protocol is 8N1 (8 bits, no parity bit, 1 stop bit), 57600 baud, 3.3V input/outputs levels (input is NOT 5V tolerant!)
// 2) all fields BIG-ENDIAN byte order
// 3) The VECTOR_OPEN_TELEMETRY packet is sent as frequently as every 80mS, but timing will vary considerably
// 4) To enable telemetry output on the Vector's UART port, select the "Open Telm" option
//    for the "Configure the UART port for" stick menu item, under the ""EagleEyes and Telemetry" OSD menu

// Vector UART Pinout (using standard Vector "BUS" cable colors):
// Yellow: RX (Receive data TO the Vector - note that this connection is not needed)
// Orange: TX (Transmit data FROM the Vector)
// Black: Ground
// Red: 5V Out, 150mA max (from Vector PSU or backup power input - do not exceed 1A total load on Vector PSU! Don't connect this wire unless the device receiving the telemetry requires power from the Vector)
// IMPORTANT: NEVER connect the telemetry cable to any Vector port other than UART!  Doing so can damage your equipment!

/*THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * ################################################################################################################# */
#include "vot.h"
#include <stdio.h>


#define VOTFrameLength 97 // 97 bytes
#define VOT_REVISION 0
#define VOT_SC1 0xB0
#define VOT_SC2 0x1E
#define VOT_SC3 0xDE
#define VOT_SC4 0xAD
#define VOT_CRC_Init 0xFFFF

typedef struct {
    uint32_t StartCode;            //  0xB01EDEAD
    uint32_t TimestampMS;          // -not used- timestamp in milliseconds
    signed long BaroAltitudecm;    // -fl baro_altitude- zero referenced (from home position) barometric altitude in cm
    uint16_t AirspeedKPHX10;       // -fl airspeed- KPH * 10, requires optional pitot sensor
    int16_t ClimbRateMSX100;       // -fl vario - meters/second * 100
    uint16_t RPM;                  // -not used- requires optional RPM sensor
    int16_t PitchDegrees;          // -i16 pitch-
    int16_t RollDegrees;           // -i16 roll-
    int16_t YawDegrees;            // -fl heading-
    int16_t AccelXCentiGrav;       // -not used-
    int16_t AccelYCentiGrav;       // -not used-
    int16_t AccelZCentiGrav;       // -not used-
    uint16_t PackVoltageX100;      // -fl voltage-
    uint16_t VideoTxVoltageX100;   // -fl vtxvoltage
    uint16_t CameraVoltageX100;    // -fl camvoltage
    uint16_t RxVoltageX100;        // -fl rxvoltage
    uint16_t PackCurrentX10;       // -fl ampere-
    int16_t TempDegreesCX10;       // -i16 temp- degrees C * 10, from optional temperature sensor
    uint16_t mAHConsumed;          // -u16 mahconsumed-
    uint16_t CompassDegrees;       // -u16 compassdegrees used- either magnetic compass reading (if compass enabled) or filtered GPS course over ground if not
    uint8_t RSSIPercent;           // -u8 rssi-
    uint8_t LQPercent;             // -u8 LQ-
    signed long LatitudeX1E7;      // -dbl latitude- (degrees * 10,000,000 )
    signed long LongitudeX1E7;     // -dbl longitude- (degrees * 10,000,000 )
    uint32_t DistanceFromHomeMX10; // -fl distance- horizontal GPS distance from home point, in meters X 10 (decimeters)
    uint16_t GroundspeedKPHX10;    // -fl speed- ( km/h * 10 )
    uint16_t CourseDegrees;        // -u16 coursedegrees- GPS course over ground, in degrees
    signed long GPSAltitudecm;     // -fl altitude- ( GPS altitude, using WGS-84 ellipsoid, cm)
    uint8_t HDOPx10;               // -fl hdop- GPS HDOP * 10
    uint8_t SatsInUse;             // -u8 sats- satellites used for navigation
    uint8_t PresentFlightMode;     // -u8 uav_flightmode- present flight mode, as defined in VECTOR_FLIGHT_MODES
    uint8_t RFU[24];               // -not used- reserved for future use
    uint16_t CRC;
} VOT_td_t; //97 bytes


static union {
    uint8_t SerialBuffer[sizeof(VOT_td_t)];
    
    //unfortunately, this doesn't work because of the damn Endians..
    VOT_td_t td; 
} v;



static uint8_t VOTReceiverIndex;
static uint8_t VOTReadIndex;
static uint16_t VOTRcvChecksum;

static enum _serial_state {
    IDLE,
    HEADER_START1,
    HEADER_START2,
    HEADER_START3,
    HEADER_START4,
    HEADER_MSGTYPE
}

c_state = IDLE;


uint8_t votread_u8() {
    return v.SerialBuffer[VOTReadIndex++];
}


uint16_t votread_u16() {
    uint16_t t = votread_u8();

    t |= (uint16_t)votread_u8() << 8;

    return t;
}


uint16_t votbread_u16() {
    uint16_t t = votread_u8() << 8;

    t |= votread_u8();

    return t;
}


uint32_t votread_u32() {
    uint32_t t = votread_u16();

    t |= (uint32_t)votread_u16() << 16;

    return t;
}


uint32_t votbread_u32() {
    uint32_t t = votbread_u16() << 16;

    t |= votbread_u16();

    return t;
}



uint16_t CRC16Worker(uint16_t icrc, uint8_t r0) {
    union {
        uint16_t crc16;

        struct {
            uint8_t crcl, crch;
        } s;
    } u;

    // scratch byte
    uint8_t a1; 
    u.crc16 = icrc;
    r0 = r0 ^ u.s.crch;
    a1 = u.s.crcl;
    u.s.crch = r0;
    u.s.crch = (u.s.crch << 4) | (u.s.crch >> 4);
    u.s.crcl = u.s.crch ^ r0;
    u.crc16 &= 0x0ff0;
    r0 ^= u.s.crch;
    a1 ^= u.s.crcl;
    u.crc16 <<= 1;
    u.s.crcl ^= r0;
    u.s.crch ^= a1;

    return (u.crc16);
}



uint16_t CalculateCRC(uint8_t *pPacket, uint8_t Size, uint16_t InitCRC) {
    uint16_t i;
    uint16_t CRC;

    CRC = InitCRC;

    for (i = 0; i < Size; i++) {
        CRC = CRC16Worker(CRC, pPacket[i]);
    }

    return CRC;
}



void vot_read(telemetry_data_t_osd *td, uint8_t *buf, int buflen) {
    int i;
    uint8_t c;
    uint16_t cs;

    td->datarx++;

    for (i = 0; i < buflen; ++i) {
        uint8_t c = buf[i];

        if (c_state == IDLE) {
            c_state = (c == VOT_SC1) ? HEADER_START1 : IDLE;
            VOTReceiverIndex = 0;
        } else if (c_state == HEADER_START1) {
            c_state = (c == VOT_SC2) ? HEADER_START2 : IDLE;
        } else if (c_state == HEADER_START2) {
            c_state = (c == VOT_SC3) ? HEADER_START3 : IDLE;
        } else if (c_state == HEADER_START3) {
            c_state = (c == VOT_SC4) ? HEADER_START4 : IDLE;
        } else if (c_state == HEADER_START4) {
            c_state = HEADER_MSGTYPE;
        }
      
        if (c_state != IDLE) {
            v.SerialBuffer[VOTReceiverIndex++] = c;

            if (VOTReceiverIndex == VOTFrameLength) { 
                // received checksum word little endian
                VOTRcvChecksum = CalculateCRC((uint8_t *)&v.SerialBuffer[0], 95, 0xFFFF);
                cs = (uint16_t)(v.SerialBuffer[VOTFrameLength - 1] << 8) + v.SerialBuffer[VOTFrameLength - 2];
                
                if (VOTRcvChecksum == cs) {
                    vot_decode(td);
                    c_state = IDLE;
                } else { 
                    // wrong checksum, drop packet
                    c_state = IDLE;
                }
            }
        }
    } //next i


    return;
}



/*
 * Decoded received commands
 * 
 */
void vot_decode(telemetry_data_t_osd *td) {
    int i;
    uint8_t dummy8;
    uint16_t dummy16;
    uint32_t dummy32;

    VOTReadIndex = 0;
    fprintf(telemetry_file, "\n");
    dummy32 = (uint32_t)votread_u32();                              // StartCode
    dummy32 = (uint32_t)votread_u32();                              // TimeStamp
    td->rel_altitude = (float)(signed long)votbread_u32() / 100.0f; // BaroAltitudecm
    td->airspeed = (float)(uint16_t)votbread_u16() / 10.0f;         // airspeed
    td->vario = (float)(uint16_t)votbread_u16() / 100.0f;           // ClimbRateMSX100 -not used- meters/second * 100
    td->rpm = (uint16_t)votbread_u16();                             // RPM - requires optional RPM sensor
    td->pitch = (int16_t)votbread_u16();                            // PitchDegrees-pitch-
    td->roll = (int16_t)votbread_u16();                             // RollDegrees-roll-
    td->heading = (float)(int16_t)votbread_u16();                   // YawDegrees-heading-
    td->x = (int16_t)votbread_u16();                                // AccelXCentiGrav-not used-
    td->y = (int16_t)votbread_u16();                                // AccelZCentiGrav -not used-
    td->z = (int16_t)votbread_u16();                                // AccelXCentiGrav-not used-
    td->voltage = (float)(uint16_t)votbread_u16() / 100.0f;         // PackVoltageX100 -voltage-
    td->vtxvoltage = (float)(uint16_t)votbread_u16() / 100.0f;      // VideoTxVoltageX100-vtxvoltage-
    td->camvoltage = (float)(uint16_t)votbread_u16() / 100.0f;      // CameraVoltageX100-camvoltage-
    td->rxvoltage = (float)(uint16_t)votbread_u16() / 100.0f;       // RxVoltageX100-rxvoltage-
    td->ampere = (float)(uint16_t)votbread_u16() / 10.0f;           // PackCurrentX10-ampere-
    dummy16 = (uint16_t)votbread_u16();                             // TempDegreesCX10-- degrees C * 10, from optional temperature sensor
    dummy16 = (uint16_t)votbread_u16();                             // mAHConsumed-not used-
    dummy16 = (uint16_t)votbread_u16();                             // CompassDegrees-not used- either magnetic compass reading (if compass enabled) or filtered GPS course over ground if not
    td->rssi = (uint8_t)votread_u8();                               // RSSIPercent-rssi-
    td->lq = (uint8_t)votread_u8();                                 // LQPercent-not used-
    td->latitude = (double)(int32_t)votbread_u32() / 10000000;      // -latitude- (degrees * 10,000,000 )
    td->longitude = (double)(int32_t)votbread_u32() / 10000000;     // -longitude- (degrees * 10,000,000 )
    td->distance = (float)(uint32_t)votbread_u32() / 10.0f;         // DistanceFromHomeMX10 horizontal GPS distance from home point, in meters X 10 (decimeters)
    td->speed = (float)(uint16_t)votbread_u16() / 10.0f;            // -speed- ( km/h * 10 )
    td->coursedegrees = (uint16_t)votbread_u16();                   // CourseDegrees -not used- GPS course over ground, in degrees
    td->msl_altitude = (float)(int32_t)votbread_u32() / 100.0f;     // -altitude- ( GPS altitude, using WGS-84 ellipsoid, cm)
    td->hdop = (float)(uint8_t)votread_u8();                        // -HDOPx10- GPS HDOP * 10
    td->sats = (uint8_t)votread_u8();                               // -SatsInUse- satellites used for navigation
    td->flightmode = (uint8_t)votread_u8();                         // PresentFlightMode -uav_flightmode- present flight mode, as defined in VECTOR_FLIGHT_MODES
                                                                    //RFU[24];
                                                                    // CRC);
    VOTReadIndex = 0;

    fprintf(telemetry_file, "\n");
    fprintf(telemetry_file, "latitude:%f  ", td->latitude);
    fprintf(telemetry_file, "longitude:%f  ", td->longitude);
    fprintf(telemetry_file, "groundspeed:%d  ", td->speed);
    fprintf(telemetry_file, "altitude:%f  ", td->rel_altitude);
    fprintf(telemetry_file, "sats:%d  ", td->sats);
    fprintf(telemetry_file, "pitch:%d  ", td->pitch);
    fprintf(telemetry_file, "roll:%d  ", td->roll);
    fprintf(telemetry_file, "heading:%f  ", td->heading);
    fprintf(telemetry_file, "voltage:%f  ", td->voltage);
    fprintf(telemetry_file, "ampere:%f  ", td->ampere);
    fprintf(telemetry_file, "rssi:%f  ", td->rssi);
    //fprintf(telemetry_file, "GPS hdop:%f  ", td->hdop);
    //fprintf(telemetry_file, "flightmode:%d  ", td->flightmode);
    fprintf(telemetry_file, "\n");

    td->validmsgsrx++;
}
