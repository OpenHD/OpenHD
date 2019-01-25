/** @file
 *	@brief MAVLink comm protocol generated from icarous.xml
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
namespace icarous {

/**
 * Array of msg_entry needed for @p mavlink_parse_char() (trought @p mavlink_get_msg_entry())
 */
constexpr std::array<mavlink_msg_entry_t, 2> MESSAGE_ENTRIES {{ {42000, 227, 1, 0, 0, 0}, {42001, 239, 46, 0, 0, 0} }};

//! MAVLINK VERSION
constexpr auto MAVLINK_VERSION = 2;


// ENUM DEFINITIONS


/** @brief  */
enum class ICAROUS_TRACK_BAND_TYPES : uint8_t
{
    TYPE_NONE=0, /*  | */
    TYPE_NEAR=1, /*  | */
    TYPE_RECOVERY=2, /*  | */
};

//! ICAROUS_TRACK_BAND_TYPES ENUM_END
constexpr auto ICAROUS_TRACK_BAND_TYPES_ENUM_END = 3;

/** @brief  */
enum class ICAROUS_FMS_STATE : uint8_t
{
    IDLE=0, /*  | */
    TAKEOFF=1, /*  | */
    CLIMB=2, /*  | */
    CRUISE=3, /*  | */
    APPROACH=4, /*  | */
    LAND=5, /*  | */
};

//! ICAROUS_FMS_STATE ENUM_END
constexpr auto ICAROUS_FMS_STATE_ENUM_END = 6;


} // namespace icarous
} // namespace mavlink

// MESSAGE DEFINITIONS
#include "./mavlink_msg_icarous_heartbeat.hpp"
#include "./mavlink_msg_icarous_kinematic_bands.hpp"

// base include

