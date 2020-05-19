#pragma once

#include "telemetry.h"

void vot_read(telemetry_data_t_osd *td, uint8_t *buf, int buflen);
void vot_decode(telemetry_data_t_osd *td);

void telemetry_off(void);
