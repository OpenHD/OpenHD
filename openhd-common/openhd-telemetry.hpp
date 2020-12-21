#ifndef OPENHD_TELEMETRY_H
#define OPENHD_TELEMETRY_H

#include <string>

#include "openhd-util.hpp"


typedef enum TelemetryType {
    TelemetryTypeMavlink,
    TelemetryTypeLTM,
    TelemetryTypeMSPV2,
    TelemetryTypeVector,
    TelemetryTypeFrSky,
    TelemetryTypeUnknown
} TelemetryType;


inline std::string telemetry_type_to_string(TelemetryType telemetry_type) {
    switch (telemetry_type) {
        case TelemetryTypeMavlink: {
            return "mavlink";
        }
        case TelemetryTypeLTM: {
            return "ltm";
        }
        case TelemetryTypeMSPV2: {
            return "mspv2";
        }
        case TelemetryTypeVector: {
            return "vector";
        }
        case TelemetryTypeFrSky: {
            return "frsky";
        }
        default: {
            return "unknown";
        }
    }
}


inline TelemetryType string_to_telemetry_type(std::string telemetry_type) {
    if (to_uppercase(telemetry_type).find(to_uppercase("mavlink")) != std::string::npos) {
        return TelemetryTypeMavlink;
    } else if (to_uppercase(telemetry_type).find(to_uppercase("ltm")) != std::string::npos) {
        return TelemetryTypeLTM;
    } else if (to_uppercase(telemetry_type).find(to_uppercase("mspv2")) != std::string::npos) {
        return TelemetryTypeMSPV2;
    } else if (to_uppercase(telemetry_type).find(to_uppercase("vector")) != std::string::npos) {
        return TelemetryTypeVector;
    } else if (to_uppercase(telemetry_type).find(to_uppercase("frsky")) != std::string::npos) {
        return TelemetryTypeFrSky;
    }

    return TelemetryTypeUnknown;
}

#endif
