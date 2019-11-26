#include "osdconfig.h"

#ifdef VOT
#include "telemetry.h"

void VOT_read(telemetry_data_t *td, uint8_t *buf, int buflen);

void telemetry_off(void);

#endif
