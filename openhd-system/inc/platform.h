#ifndef PLATFORM_H
#define PLATFORM_H

#include <array>
#include <chrono>

#include "json.hpp"

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


typedef enum WiFiHotspotType {
    WiFiHotspotTypeInternal2GBand,
    WiFiHotspotTypeInternal5GBand,
    WiFiHotspotTypeInternalDualBand,
    WiFiHotspotTypeExternal,
    WiFiHotspotTypeNone
} WiFiHotspotType;


class Platform {
public:
    Platform();
    
    virtual ~Platform() {}

    void discover();

    nlohmann::json generate_manifest();

    PlatformType platform_type() {
        return m_platform_type;
    }

    BoardType board_type() {
        return m_board_type;
    }

    CarrierType carrier_type() {
        return m_carrier_type;
    }

    WiFiHotspotType wifi_hotspot_type() {
        return m_wifi_hotspot_type;
    }

private:
    std::string platform_type_string(PlatformType platform_type);
    std::string board_type_string(BoardType board_type);
    std::string carrier_type_string(CarrierType carrier_type);

    void detect_raspberrypi();
    void detect_jetson();
    void detect_pc();

    PlatformType m_platform_type = PlatformTypeUnknown;
    BoardType m_board_type = BoardTypeUnknown;
    CarrierType m_carrier_type = CarrierTypeNone;

    WiFiHotspotType m_wifi_hotspot_type = WiFiHotspotTypeNone;
};

#endif
