#pragma once

#include "telemetry.h"

#include <openhd/mavlink.h>

int mavlink_read(telemetry_data_t_osd *td, uint8_t *buf, int buflen);
