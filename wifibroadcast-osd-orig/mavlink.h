#include "osdconfig.h"

#ifdef MAVLINK
#include "telemetry.h"
#include "mavlink/common/mavlink.h"

int mavlink_read(telemetry_data_t *td, uint8_t *buf, int buflen);
#endif
