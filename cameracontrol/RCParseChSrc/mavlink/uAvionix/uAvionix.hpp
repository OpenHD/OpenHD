/** @file
 *	@brief MAVLink comm protocol generated from uAvionix.xml
 *	@see http://mavlink.org
 */

#pragma once

#include <array>
#include <cstdint>
#include <sstream>

#ifndef MAVLINK_STX
#define MAVLINK_STX 253
#endif

#include "../message.hpp"

namespace mavlink {
namespace uAvionix {

/**
 * Array of msg_entry needed for @p mavlink_parse_char() (trought @p mavlink_get_msg_entry())
 */
constexpr std::array<mavlink_msg_entry_t, 3> MESSAGE_ENTRIES {{ {10001, 209, 20, 0, 0, 0}, {10002, 186, 41, 0, 0, 0}, {10003, 4, 1, 0, 0, 0} }};

//! MAVLINK VERSION
constexpr auto MAVLINK_VERSION = 2;


// ENUM DEFINITIONS


/** @brief State flags for ADS-B transponder dynamic report */
enum class UAVIONIX_ADSB_OUT_DYNAMIC_STATE : uint16_t
{
    INTENT_CHANGE=1, /*  | */
    AUTOPILOT_ENABLED=2, /*  | */
    NICBARO_CROSSCHECKED=4, /*  | */
    ON_GROUND=8, /*  | */
    IDENT=16, /*  | */
};

//! UAVIONIX_ADSB_OUT_DYNAMIC_STATE ENUM_END
constexpr auto UAVIONIX_ADSB_OUT_DYNAMIC_STATE_ENUM_END = 17;

/** @brief Transceiver RF control flags for ADS-B transponder dynamic reports */
enum class UAVIONIX_ADSB_OUT_RF_SELECT : uint8_t
{
    STANDBY=0, /*  | */
    RX_ENABLED=1, /*  | */
    TX_ENABLED=2, /*  | */
};

//! UAVIONIX_ADSB_OUT_RF_SELECT ENUM_END
constexpr auto UAVIONIX_ADSB_OUT_RF_SELECT_ENUM_END = 3;

/** @brief Status for ADS-B transponder dynamic input */
enum class UAVIONIX_ADSB_OUT_DYNAMIC_GPS_FIX : uint8_t
{
    NONE_0=0, /*  | */
    NONE_1=1, /*  | */
    FIX_2D=2, /*  | */
    FIX_3D=3, /*  | */
    DGPS=4, /*  | */
    RTK=5, /*  | */
};

//! UAVIONIX_ADSB_OUT_DYNAMIC_GPS_FIX ENUM_END
constexpr auto UAVIONIX_ADSB_OUT_DYNAMIC_GPS_FIX_ENUM_END = 6;

/** @brief Status flags for ADS-B transponder dynamic output */
enum class UAVIONIX_ADSB_RF_HEALTH : uint8_t
{
    INITIALIZING=0, /*  | */
    OK=1, /*  | */
    FAIL_TX=2, /*  | */
    FAIL_RX=16, /*  | */
};

//! UAVIONIX_ADSB_RF_HEALTH ENUM_END
constexpr auto UAVIONIX_ADSB_RF_HEALTH_ENUM_END = 17;

/** @brief Definitions for aircraft size */
enum class UAVIONIX_ADSB_OUT_CFG_AIRCRAFT_SIZE : uint8_t
{
    NO_DATA_=0, /*  | */
    L15M_W23M=1, /*  | */
    L25M_W28P5M=2, /*  | */
    L25_34M=3, /*  | */
    L35_33M=4, /*  | */
    L35_38M=5, /*  | */
    L45_39P5M=6, /*  | */
    L45_45M=7, /*  | */
    L55_45M=8, /*  | */
    L55_52M=9, /*  | */
    L65_59P5M=10, /*  | */
    L65_67M=11, /*  | */
    L75_W72P5M=12, /*  | */
    L75_W80M=13, /*  | */
    L85_W80M=14, /*  | */
    L85_W90M=15, /*  | */
};

//! UAVIONIX_ADSB_OUT_CFG_AIRCRAFT_SIZE ENUM_END
constexpr auto UAVIONIX_ADSB_OUT_CFG_AIRCRAFT_SIZE_ENUM_END = 16;

/** @brief GPS lataral offset encoding */
enum class UAVIONIX_ADSB_OUT_CFG_GPS_OFFSET_LAT : uint8_t
{
    NO_DATA_=0, /*  | */
    LEFT_2M=1, /*  | */
    LEFT_4M=2, /*  | */
    LEFT_6M=3, /*  | */
    RIGHT_0M=4, /*  | */
    RIGHT_2M=5, /*  | */
    RIGHT_4M=6, /*  | */
    RIGHT_6M=7, /*  | */
};

//! UAVIONIX_ADSB_OUT_CFG_GPS_OFFSET_LAT ENUM_END
constexpr auto UAVIONIX_ADSB_OUT_CFG_GPS_OFFSET_LAT_ENUM_END = 8;

/** @brief GPS longitudinal offset encoding */
enum class UAVIONIX_ADSB_OUT_CFG_GPS_OFFSET_LON : uint8_t
{
    NO_DATA_=0, /*  | */
    APPLIED_BY_SENSOR=1, /*  | */
};

//! UAVIONIX_ADSB_OUT_CFG_GPS_OFFSET_LON ENUM_END
constexpr auto UAVIONIX_ADSB_OUT_CFG_GPS_OFFSET_LON_ENUM_END = 2;

/** @brief Emergency status encoding */
enum class UAVIONIX_ADSB_EMERGENCY_STATUS : uint8_t
{
    OUT_NO_EMERGENCY=0, /*  | */
    OUT_GENERAL_EMERGENCY=1, /*  | */
    OUT_LIFEGUARD_EMERGENCY=2, /*  | */
    OUT_MINIMUM_FUEL_EMERGENCY=3, /*  | */
    OUT_NO_COMM_EMERGENCY=4, /*  | */
    OUT_UNLAWFUL_INTERFERANCE_EMERGENCY=5, /*  | */
    OUT_DOWNED_AIRCRAFT_EMERGENCY=6, /*  | */
    OUT_RESERVED=7, /*  | */
};

//! UAVIONIX_ADSB_EMERGENCY_STATUS ENUM_END
constexpr auto UAVIONIX_ADSB_EMERGENCY_STATUS_ENUM_END = 8;


} // namespace uAvionix
} // namespace mavlink

// MESSAGE DEFINITIONS
#include "./mavlink_msg_uavionix_adsb_out_cfg.hpp"
#include "./mavlink_msg_uavionix_adsb_out_dynamic.hpp"
#include "./mavlink_msg_uavionix_adsb_transceiver_health_report.hpp"

// base include

