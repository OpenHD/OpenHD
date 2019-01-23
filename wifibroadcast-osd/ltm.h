#include "osdconfig.h"

#ifdef LTM
#include "telemetry.h"

int ltm_read(telemetry_data_t *td, uint8_t *buf, int buflen);
int ltm_check(telemetry_data_t *td);

#define LIGHTTELEMETRY_START1 0x24 //$ Header byte 1
#define LIGHTTELEMETRY_START2 0x54 //T Header byte 2
#define LIGHTTELEMETRY_GFRAME 0x47 //G GPS frame: GPS + Baro altitude data (Lat, Lon, Speed, Alt, Sats, Sat fix)
#define LIGHTTELEMETRY_AFRAME 0x41 //A Attitude frame: Attitude data (Roll, Pitch, Heading)
#define LIGHTTELEMETRY_SFRAME 0x53 //S Status frame: Sensors/Status data (VBat, Consumed current, Rssi, Airspeed, Arm status, Failsafe status, Flight mode)
#define LIGHTTELEMETRY_OFRAME 0x4f //O Origin frame: (Lat, Lon, Alt, OSD on, home fix)
#define LIGHTTELEMETRY_NFRAME 0x53 //N Navigation frame
#define LIGHTTELEMETRY_XFRAME 0x4f //X GPS eXtra frame: (GPS HDOP value, hw_status (failed sensor))

// complete length including headers and checksum
#define LIGHTTELEMETRY_GFRAMELENGTH 18
#define LIGHTTELEMETRY_AFRAMELENGTH 10
#define LIGHTTELEMETRY_SFRAMELENGTH 11
#define LIGHTTELEMETRY_OFRAMELENGTH 18
#define LIGHTTELEMETRY_NFRAMELENGTH 10
#define LIGHTTELEMETRY_XFRAMELENGTH 10

void telemetry_off(void);

#endif
