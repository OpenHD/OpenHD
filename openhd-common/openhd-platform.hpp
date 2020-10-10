#ifndef OPENHD_PLATFORM_H
#define OPENHD_PLATFORM_H


#include <string>

#include "openhd-util.hpp"

typedef enum PlatformType {
    PlatformTypeRaspberryPi,
    PlatformTypeJetson,
    PlatformTypeNanoPi,
    PlatformTypeRockchip,
    PlatformTypePC,
    PlatformTypeUnknown
} PlatformType;


typedef enum BoardType {
    BoardTypeRaspberryPiZero,
    BoardTypeRaspberryPiZeroW,
    BoardTypeRaspberryPi2B,
    BoardTypeRaspberryPi3A,
    BoardTypeRaspberryPi3APlus,
    BoardTypeRaspberryPi3B,
    BoardTypeRaspberryPi3BPlus,
    BoardTypeRaspberryPiCM,
    BoardTypeRaspberryPiCM3,
    BoardTypeRaspberryPiCM3Plus,
    BoardTypeRaspberryPiCM4,
    BoardTypeRaspberryPi4B,
    BoardTypeJetsonNano,
    BoardTypeJetsonTX1,
    BoardTypeJetsonTX2,
    BoardTypeJetsonNX,
    BoardTypeJetsonAGX,
    BoardTypeNanoPiNeo4,
    BoardTypeGenericPC,
    BoardTypeUnknown
} BoardType;


typedef enum CarrierType {
    CarrierTypeStereoPi,
    CarrierTypeComputeModuleIO,
    CarrierTypeJetsonNanoDevkit,
    CarrierTypeJetsonNCB00,
    CarrierTypeNone
} CarrierType;


inline std::string carrier_type_to_string(CarrierType carrier_type) {
    switch (carrier_type) {
        case CarrierTypeStereoPi: {
            return "stereopi";
        }
        case CarrierTypeComputeModuleIO: {
            return "computemoduleio";
        }
        case CarrierTypeJetsonNanoDevkit: {
            return "jetson-nano-devkit";
        }
        case CarrierTypeJetsonNCB00: {
            return "jetson-ncb00";
        }
        default: {
            return "none";
        }
    }
}


inline std::string board_type_to_string(BoardType board_type) {
    switch (board_type) {
        case BoardTypeRaspberryPiZero: {
            return "pizero";
        }
        case BoardTypeRaspberryPiZeroW: {
            return "pizerow";
        }
        case BoardTypeRaspberryPi2B: {
            return "pi2b";
        }
        case BoardTypeRaspberryPi3A: {
            return "pi3a";
        }
        case BoardTypeRaspberryPi3APlus: {
            return "pi3aplus";
        }
        case BoardTypeRaspberryPi3B: {
            return "pi3b";
        }
        case BoardTypeRaspberryPi3BPlus: {
            return "pi3bplus";
        }
        case BoardTypeRaspberryPiCM: {
            return "picm";
        }
        case BoardTypeRaspberryPiCM3: {
            return "picm3";
        }
        case BoardTypeRaspberryPiCM3Plus: {
            return "picm3plus";
        }
        case BoardTypeRaspberryPiCM4: {
            return "picm4";
        }
        case BoardTypeRaspberryPi4B: {
            return "pi4b";
        }
        case BoardTypeJetsonNano: {
            return "jetson-nano";
        }
        case BoardTypeJetsonTX1: {
            return "jetson-tx1";
        }
        case BoardTypeJetsonTX2: {
            return "jetson-tx2";
        }
        case BoardTypeJetsonNX: {
            return "jetson-nx";
        }
        case BoardTypeJetsonAGX: {
            return "jetson-agx";
        }
        case BoardTypeNanoPiNeo4: {
            return "nanopi-neo4";
        }
        case BoardTypeGenericPC: {
            return "generic-pc";
        }
        default: {
            return "unknown";
        }
    }
}


inline std::string platform_type_to_string(PlatformType platform_type) {
    switch (platform_type) {
        case PlatformTypeJetson: {
            return "jetson";
        }
        case PlatformTypeRaspberryPi: {
            return "raspberrypi";
        }
        case PlatformTypeNanoPi: {
            return "nanopi";
        }
        case PlatformTypePC: {
            return "pc";
        }
        default: {
            return "unknown";
        }
    }
}


inline PlatformType string_to_platform_type(std::string hotspot_type) {
    if (to_uppercase(hotspot_type).find(to_uppercase("jetson")) != std::string::npos) {
        return PlatformTypeJetson;
    } else if (to_uppercase(hotspot_type).find(to_uppercase("raspberrypi")) != std::string::npos) {
        return PlatformTypeRaspberryPi;
    } else if (to_uppercase(hotspot_type).find(to_uppercase("nanopi")) != std::string::npos) {
        return PlatformTypeNanoPi;
    } else if (to_uppercase(hotspot_type).find(to_uppercase("pc")) != std::string::npos) {
        return PlatformTypePC;
    }

    return PlatformTypeUnknown;
}

#endif
