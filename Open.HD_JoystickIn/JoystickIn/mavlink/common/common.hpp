/** @file
 *	@brief MAVLink comm protocol generated from common.xml
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
namespace common {

/**
 * Array of msg_entry needed for @p mavlink_parse_char() (trought @p mavlink_get_msg_entry())
 */
constexpr std::array<mavlink_msg_entry_t, 154> MESSAGE_ENTRIES {{ {0, 50, 9, 0, 0, 0}, {1, 124, 31, 0, 0, 0}, {2, 137, 12, 0, 0, 0}, {4, 237, 14, 3, 12, 13}, {5, 217, 28, 1, 0, 0}, {6, 104, 3, 0, 0, 0}, {7, 119, 32, 0, 0, 0}, {11, 89, 6, 1, 4, 0}, {20, 214, 20, 3, 2, 3}, {21, 159, 2, 3, 0, 1}, {22, 220, 25, 0, 0, 0}, {23, 168, 23, 3, 4, 5}, {24, 24, 30, 0, 0, 0}, {25, 23, 101, 0, 0, 0}, {26, 170, 22, 0, 0, 0}, {27, 144, 26, 0, 0, 0}, {28, 67, 16, 0, 0, 0}, {29, 115, 14, 0, 0, 0}, {30, 39, 28, 0, 0, 0}, {31, 246, 32, 0, 0, 0}, {32, 185, 28, 0, 0, 0}, {33, 104, 28, 0, 0, 0}, {34, 237, 22, 0, 0, 0}, {35, 244, 22, 0, 0, 0}, {36, 222, 21, 0, 0, 0}, {37, 212, 6, 3, 4, 5}, {38, 9, 6, 3, 4, 5}, {39, 254, 37, 3, 32, 33}, {40, 230, 4, 3, 2, 3}, {41, 28, 4, 3, 2, 3}, {42, 28, 2, 0, 0, 0}, {43, 132, 2, 3, 0, 1}, {44, 221, 4, 3, 2, 3}, {45, 232, 2, 3, 0, 1}, {46, 11, 2, 0, 0, 0}, {47, 153, 3, 3, 0, 1}, {48, 41, 13, 1, 12, 0}, {49, 39, 12, 0, 0, 0}, {50, 78, 37, 3, 18, 19}, {51, 196, 4, 3, 2, 3}, {54, 15, 27, 3, 24, 25}, {55, 3, 25, 0, 0, 0}, {61, 167, 72, 0, 0, 0}, {62, 183, 26, 0, 0, 0}, {63, 119, 181, 0, 0, 0}, {64, 191, 225, 0, 0, 0}, {65, 118, 42, 0, 0, 0}, {66, 148, 6, 3, 2, 3}, {67, 21, 4, 0, 0, 0}, {69, 243, 11, 0, 0, 0}, {70, 124, 18, 3, 16, 17}, {73, 38, 37, 3, 32, 33}, {74, 20, 20, 0, 0, 0}, {75, 158, 35, 3, 30, 31}, {76, 152, 33, 3, 30, 31}, {77, 143, 3, 0, 0, 0}, {81, 106, 22, 0, 0, 0}, {82, 49, 39, 3, 36, 37}, {83, 22, 37, 0, 0, 0}, {84, 143, 53, 3, 50, 51}, {85, 140, 51, 0, 0, 0}, {86, 5, 53, 3, 50, 51}, {87, 150, 51, 0, 0, 0}, {89, 231, 28, 0, 0, 0}, {90, 183, 56, 0, 0, 0}, {91, 63, 42, 0, 0, 0}, {92, 54, 33, 0, 0, 0}, {93, 47, 81, 0, 0, 0}, {100, 175, 26, 0, 0, 0}, {101, 102, 32, 0, 0, 0}, {102, 158, 32, 0, 0, 0}, {103, 208, 20, 0, 0, 0}, {104, 56, 32, 0, 0, 0}, {105, 93, 62, 0, 0, 0}, {106, 138, 44, 0, 0, 0}, {107, 108, 64, 0, 0, 0}, {108, 32, 84, 0, 0, 0}, {109, 185, 9, 0, 0, 0}, {110, 84, 254, 3, 1, 2}, {111, 34, 16, 0, 0, 0}, {112, 174, 12, 0, 0, 0}, {113, 124, 36, 0, 0, 0}, {114, 237, 44, 0, 0, 0}, {115, 4, 64, 0, 0, 0}, {116, 76, 22, 0, 0, 0}, {117, 128, 6, 3, 4, 5}, {118, 56, 14, 0, 0, 0}, {119, 116, 12, 3, 10, 11}, {120, 134, 97, 0, 0, 0}, {121, 237, 2, 3, 0, 1}, {122, 203, 2, 3, 0, 1}, {123, 250, 113, 3, 0, 1}, {124, 87, 35, 0, 0, 0}, {125, 203, 6, 0, 0, 0}, {126, 220, 79, 0, 0, 0}, {127, 25, 35, 0, 0, 0}, {128, 226, 35, 0, 0, 0}, {129, 46, 22, 0, 0, 0}, {130, 29, 13, 0, 0, 0}, {131, 223, 255, 0, 0, 0}, {132, 85, 14, 0, 0, 0}, {133, 6, 18, 0, 0, 0}, {134, 229, 43, 0, 0, 0}, {135, 203, 8, 0, 0, 0}, {136, 1, 22, 0, 0, 0}, {137, 195, 14, 0, 0, 0}, {138, 109, 36, 0, 0, 0}, {139, 168, 43, 3, 41, 42}, {140, 181, 41, 0, 0, 0}, {141, 47, 32, 0, 0, 0}, {142, 72, 243, 0, 0, 0}, {143, 131, 14, 0, 0, 0}, {144, 127, 93, 0, 0, 0}, {146, 103, 100, 0, 0, 0}, {147, 154, 36, 0, 0, 0}, {148, 178, 60, 0, 0, 0}, {149, 200, 30, 0, 0, 0}, {230, 163, 42, 0, 0, 0}, {231, 105, 40, 0, 0, 0}, {232, 151, 63, 0, 0, 0}, {233, 35, 182, 0, 0, 0}, {234, 150, 40, 0, 0, 0}, {241, 90, 32, 0, 0, 0}, {242, 104, 52, 0, 0, 0}, {243, 85, 53, 1, 52, 0}, {244, 95, 6, 0, 0, 0}, {245, 130, 2, 0, 0, 0}, {246, 184, 38, 0, 0, 0}, {247, 81, 19, 0, 0, 0}, {248, 8, 254, 3, 3, 4}, {249, 204, 36, 0, 0, 0}, {250, 49, 30, 0, 0, 0}, {251, 170, 18, 0, 0, 0}, {252, 44, 18, 0, 0, 0}, {253, 83, 51, 0, 0, 0}, {254, 46, 9, 0, 0, 0}, {256, 71, 42, 3, 8, 9}, {257, 131, 9, 0, 0, 0}, {258, 187, 32, 3, 0, 1}, {259, 92, 235, 0, 0, 0}, {260, 146, 5, 0, 0, 0}, {261, 179, 27, 0, 0, 0}, {262, 12, 18, 0, 0, 0}, {263, 133, 255, 0, 0, 0}, {264, 49, 28, 0, 0, 0}, {265, 26, 16, 0, 0, 0}, {266, 193, 255, 3, 2, 3}, {267, 35, 255, 3, 2, 3}, {268, 14, 4, 3, 2, 3}, {299, 19, 96, 0, 0, 0}, {310, 28, 17, 0, 0, 0}, {311, 95, 116, 0, 0, 0}, {330, 23, 158, 0, 0, 0}, {331, 58, 230, 0, 0, 0} }};

//! MAVLINK VERSION
constexpr auto MAVLINK_VERSION = 3;


// ENUM DEFINITIONS


/** @brief Micro air vehicle / autopilot classes. This identifies the individual model. */
enum class MAV_AUTOPILOT : uint8_t
{
    GENERIC=0, /* Generic autopilot, full support for everything | */
    RESERVED=1, /* Reserved for future use. | */
    SLUGS=2, /* SLUGS autopilot, http://slugsuav.soe.ucsc.edu | */
    ARDUPILOTMEGA=3, /* ArduPilot - Plane/Copter/Rover/Sub/Tracker, http://ardupilot.org | */
    OPENPILOT=4, /* OpenPilot, http://openpilot.org | */
    GENERIC_WAYPOINTS_ONLY=5, /* Generic autopilot only supporting simple waypoints | */
    GENERIC_WAYPOINTS_AND_SIMPLE_NAVIGATION_ONLY=6, /* Generic autopilot supporting waypoints and other simple navigation commands | */
    GENERIC_MISSION_FULL=7, /* Generic autopilot supporting the full mission command set | */
    INVALID=8, /* No valid autopilot, e.g. a GCS or other MAVLink component | */
    PPZ=9, /* PPZ UAV - http://nongnu.org/paparazzi | */
    UDB=10, /* UAV Dev Board | */
    FP=11, /* FlexiPilot | */
    PX4=12, /* PX4 Autopilot - http://pixhawk.ethz.ch/px4/ | */
    SMACCMPILOT=13, /* SMACCMPilot - http://smaccmpilot.org | */
    AUTOQUAD=14, /* AutoQuad -- http://autoquad.org | */
    ARMAZILA=15, /* Armazila -- http://armazila.com | */
    AEROB=16, /* Aerob -- http://aerob.ru | */
    ASLUAV=17, /* ASLUAV autopilot -- http://www.asl.ethz.ch | */
    SMARTAP=18, /* SmartAP Autopilot - http://sky-drones.com | */
    AIRRAILS=19, /* AirRails - http://uaventure.com | */
};

//! MAV_AUTOPILOT ENUM_END
constexpr auto MAV_AUTOPILOT_ENUM_END = 20;

/** @brief  */
enum class MAV_TYPE : uint8_t
{
    GENERIC=0, /* Generic micro air vehicle. | */
    FIXED_WING=1, /* Fixed wing aircraft. | */
    QUADROTOR=2, /* Quadrotor | */
    COAXIAL=3, /* Coaxial helicopter | */
    HELICOPTER=4, /* Normal helicopter with tail rotor. | */
    ANTENNA_TRACKER=5, /* Ground installation | */
    GCS=6, /* Operator control unit / ground control station | */
    AIRSHIP=7, /* Airship, controlled | */
    FREE_BALLOON=8, /* Free balloon, uncontrolled | */
    ROCKET=9, /* Rocket | */
    GROUND_ROVER=10, /* Ground rover | */
    SURFACE_BOAT=11, /* Surface vessel, boat, ship | */
    SUBMARINE=12, /* Submarine | */
    HEXAROTOR=13, /* Hexarotor | */
    OCTOROTOR=14, /* Octorotor | */
    TRICOPTER=15, /* Tricopter | */
    FLAPPING_WING=16, /* Flapping wing | */
    KITE=17, /* Kite | */
    ONBOARD_CONTROLLER=18, /* Onboard companion controller | */
    VTOL_DUOROTOR=19, /* Two-rotor VTOL using control surfaces in vertical operation in addition. Tailsitter. | */
    VTOL_QUADROTOR=20, /* Quad-rotor VTOL using a V-shaped quad config in vertical operation. Tailsitter. | */
    VTOL_TILTROTOR=21, /* Tiltrotor VTOL | */
    VTOL_RESERVED2=22, /* VTOL reserved 2 | */
    VTOL_RESERVED3=23, /* VTOL reserved 3 | */
    VTOL_RESERVED4=24, /* VTOL reserved 4 | */
    VTOL_RESERVED5=25, /* VTOL reserved 5 | */
    GIMBAL=26, /* Onboard gimbal | */
    ADSB=27, /* Onboard ADSB peripheral | */
    PARAFOIL=28, /* Steerable, nonrigid airfoil | */
    DODECAROTOR=29, /* Dodecarotor | */
    CAMERA=30, /* Camera | */
    CHARGING_STATION=31, /* Charging station | */
    FLARM=32, /* Onboard FLARM collision avoidance system | */
};

//! MAV_TYPE ENUM_END
constexpr auto MAV_TYPE_ENUM_END = 33;

/** @brief These values define the type of firmware release.  These values indicate the first version or release of this type.  For example the first alpha release would be 64, the second would be 65. */
enum class FIRMWARE_VERSION_TYPE
{
    DEV=0, /* development release | */
    ALPHA=64, /* alpha release | */
    BETA=128, /* beta release | */
    RC=192, /* release candidate | */
    OFFICIAL=255, /* official stable release | */
};

//! FIRMWARE_VERSION_TYPE ENUM_END
constexpr auto FIRMWARE_VERSION_TYPE_ENUM_END = 256;

/** @brief These flags encode the MAV mode. */
enum class MAV_MODE_FLAG : uint8_t
{
    CUSTOM_MODE_ENABLED=1, /* 0b00000001 Reserved for future use. | */
    TEST_ENABLED=2, /* 0b00000010 system has a test mode enabled. This flag is intended for temporary system tests and should not be used for stable implementations. | */
    AUTO_ENABLED=4, /* 0b00000100 autonomous mode enabled, system finds its own goal positions. Guided flag can be set or not, depends on the actual implementation. | */
    GUIDED_ENABLED=8, /* 0b00001000 guided mode enabled, system flies waypoints / mission items. | */
    STABILIZE_ENABLED=16, /* 0b00010000 system stabilizes electronically its attitude (and optionally position). It needs however further control inputs to move around. | */
    HIL_ENABLED=32, /* 0b00100000 hardware in the loop simulation. All motors / actuators are blocked, but internal software is full operational. | */
    MANUAL_INPUT_ENABLED=64, /* 0b01000000 remote control input is enabled. | */
    SAFETY_ARMED=128, /* 0b10000000 MAV safety set to armed. Motors are enabled / running / can start. Ready to fly. Additional note: this flag is to be ignore when sent in the command MAV_CMD_DO_SET_MODE and MAV_CMD_COMPONENT_ARM_DISARM shall be used instead. The flag can still be used to report the armed state. | */
};

//! MAV_MODE_FLAG ENUM_END
constexpr auto MAV_MODE_FLAG_ENUM_END = 129;

/** @brief These values encode the bit positions of the decode position. These values can be used to read the value of a flag bit by combining the base_mode variable with AND with the flag position value. The result will be either 0 or 1, depending on if the flag is set or not. */
enum class MAV_MODE_FLAG_DECODE_POSITION
{
    CUSTOM_MODE=1, /* Eighth bit: 00000001 | */
    TEST=2, /* Seventh bit: 00000010 | */
    AUTO=4, /* Sixt bit:   00000100 | */
    GUIDED=8, /* Fifth bit:  00001000 | */
    STABILIZE=16, /* Fourth bit: 00010000 | */
    HIL=32, /* Third bit:  00100000 | */
    MANUAL=64, /* Second bit: 01000000 | */
    SAFETY=128, /* First bit:  10000000 | */
};

//! MAV_MODE_FLAG_DECODE_POSITION ENUM_END
constexpr auto MAV_MODE_FLAG_DECODE_POSITION_ENUM_END = 129;

/** @brief Override command, pauses current mission execution and moves immediately to a position */
enum class MAV_GOTO
{
    DO_HOLD=0, /* Hold at the current position. | */
    DO_CONTINUE=1, /* Continue with the next item in mission execution. | */
    HOLD_AT_CURRENT_POSITION=2, /* Hold at the current position of the system | */
    HOLD_AT_SPECIFIED_POSITION=3, /* Hold at the position specified in the parameters of the DO_HOLD action | */
};

//! MAV_GOTO ENUM_END
constexpr auto MAV_GOTO_ENUM_END = 4;

/** @brief These defines are predefined OR-combined mode flags. There is no need to use values from this enum, but it
               simplifies the use of the mode flags. Note that manual input is enabled in all modes as a safety override. */
enum class MAV_MODE : uint8_t
{
    PREFLIGHT=0, /* System is not ready to fly, booting, calibrating, etc. No flag is set. | */
    MANUAL_DISARMED=64, /* System is allowed to be active, under manual (RC) control, no stabilization | */
    TEST_DISARMED=66, /* UNDEFINED mode. This solely depends on the autopilot - use with caution, intended for developers only. | */
    STABILIZE_DISARMED=80, /* System is allowed to be active, under assisted RC control. | */
    GUIDED_DISARMED=88, /* System is allowed to be active, under autonomous control, manual setpoint | */
    AUTO_DISARMED=92, /* System is allowed to be active, under autonomous control and navigation (the trajectory is decided onboard and not pre-programmed by waypoints) | */
    MANUAL_ARMED=192, /* System is allowed to be active, under manual (RC) control, no stabilization | */
    TEST_ARMED=194, /* UNDEFINED mode. This solely depends on the autopilot - use with caution, intended for developers only. | */
    STABILIZE_ARMED=208, /* System is allowed to be active, under assisted RC control. | */
    GUIDED_ARMED=216, /* System is allowed to be active, under autonomous control, manual setpoint | */
    AUTO_ARMED=220, /* System is allowed to be active, under autonomous control and navigation (the trajectory is decided onboard and not pre-programmed by waypoints) | */
};

//! MAV_MODE ENUM_END
constexpr auto MAV_MODE_ENUM_END = 221;

/** @brief  */
enum class MAV_STATE : uint8_t
{
    UNINIT=0, /* Uninitialized system, state is unknown. | */
    BOOT=1, /* System is booting up. | */
    CALIBRATING=2, /* System is calibrating and not flight-ready. | */
    STANDBY=3, /* System is grounded and on standby. It can be launched any time. | */
    ACTIVE=4, /* System is active and might be already airborne. Motors are engaged. | */
    CRITICAL=5, /* System is in a non-normal flight mode. It can however still navigate. | */
    EMERGENCY=6, /* System is in a non-normal flight mode. It lost control over parts or over the whole airframe. It is in mayday and going down. | */
    POWEROFF=7, /* System just initialized its power-down sequence, will shut down now. | */
    FLIGHT_TERMINATION=8, /* System is terminating itself. | */
};

//! MAV_STATE ENUM_END
constexpr auto MAV_STATE_ENUM_END = 9;

/** @brief  */
enum class MAV_COMPONENT
{
    COMP_ID_ALL=0, /*  | */
    COMP_ID_AUTOPILOT1=1, /*  | */
    COMP_ID_CAMERA=100, /*  | */
    COMP_ID_CAMERA2=101, /*  | */
    COMP_ID_CAMERA3=102, /*  | */
    COMP_ID_CAMERA4=103, /*  | */
    COMP_ID_CAMERA5=104, /*  | */
    COMP_ID_CAMERA6=105, /*  | */
    COMP_ID_SERVO1=140, /*  | */
    COMP_ID_SERVO2=141, /*  | */
    COMP_ID_SERVO3=142, /*  | */
    COMP_ID_SERVO4=143, /*  | */
    COMP_ID_SERVO5=144, /*  | */
    COMP_ID_SERVO6=145, /*  | */
    COMP_ID_SERVO7=146, /*  | */
    COMP_ID_SERVO8=147, /*  | */
    COMP_ID_SERVO9=148, /*  | */
    COMP_ID_SERVO10=149, /*  | */
    COMP_ID_SERVO11=150, /*  | */
    COMP_ID_SERVO12=151, /*  | */
    COMP_ID_SERVO13=152, /*  | */
    COMP_ID_SERVO14=153, /*  | */
    COMP_ID_GIMBAL=154, /*  | */
    COMP_ID_LOG=155, /*  | */
    COMP_ID_ADSB=156, /*  | */
    COMP_ID_OSD=157, /* On Screen Display (OSD) devices for video links | */
    COMP_ID_PERIPHERAL=158, /* Generic autopilot peripheral component ID. Meant for devices that do not implement the parameter sub-protocol | */
    COMP_ID_QX1_GIMBAL=159, /*  | */
    COMP_ID_FLARM=160, /*  | */
    COMP_ID_MAPPER=180, /*  | */
    COMP_ID_MISSIONPLANNER=190, /*  | */
    COMP_ID_PATHPLANNER=195, /*  | */
    COMP_ID_IMU=200, /*  | */
    COMP_ID_IMU_2=201, /*  | */
    COMP_ID_IMU_3=202, /*  | */
    COMP_ID_GPS=220, /*  | */
    COMP_ID_GPS2=221, /*  | */
    COMP_ID_UDP_BRIDGE=240, /*  | */
    COMP_ID_UART_BRIDGE=241, /*  | */
    COMP_ID_SYSTEM_CONTROL=250, /*  | */
};

//! MAV_COMPONENT ENUM_END
constexpr auto MAV_COMPONENT_ENUM_END = 251;

/** @brief These encode the sensors whose status is sent as part of the SYS_STATUS message. */
enum class MAV_SYS_STATUS_SENSOR : uint32_t
{
    SENSOR_3D_GYRO=1, /* 0x01 3D gyro | */
    SENSOR_3D_ACCEL=2, /* 0x02 3D accelerometer | */
    SENSOR_3D_MAG=4, /* 0x04 3D magnetometer | */
    ABSOLUTE_PRESSURE=8, /* 0x08 absolute pressure | */
    DIFFERENTIAL_PRESSURE=16, /* 0x10 differential pressure | */
    GPS=32, /* 0x20 GPS | */
    OPTICAL_FLOW=64, /* 0x40 optical flow | */
    VISION_POSITION=128, /* 0x80 computer vision position | */
    LASER_POSITION=256, /* 0x100 laser based position | */
    EXTERNAL_GROUND_TRUTH=512, /* 0x200 external ground truth (Vicon or Leica) | */
    ANGULAR_RATE_CONTROL=1024, /* 0x400 3D angular rate control | */
    ATTITUDE_STABILIZATION=2048, /* 0x800 attitude stabilization | */
    YAW_POSITION=4096, /* 0x1000 yaw position | */
    Z_ALTITUDE_CONTROL=8192, /* 0x2000 z/altitude control | */
    XY_POSITION_CONTROL=16384, /* 0x4000 x/y position control | */
    MOTOR_OUTPUTS=32768, /* 0x8000 motor outputs / control | */
    RC_RECEIVER=65536, /* 0x10000 rc receiver | */
    SENSOR_3D_GYRO2=131072, /* 0x20000 2nd 3D gyro | */
    SENSOR_3D_ACCEL2=262144, /* 0x40000 2nd 3D accelerometer | */
    SENSOR_3D_MAG2=524288, /* 0x80000 2nd 3D magnetometer | */
    GEOFENCE=1048576, /* 0x100000 geofence | */
    AHRS=2097152, /* 0x200000 AHRS subsystem health | */
    TERRAIN=4194304, /* 0x400000 Terrain subsystem health | */
    REVERSE_MOTOR=8388608, /* 0x800000 Motors are reversed | */
    LOGGING=16777216, /* 0x1000000 Logging | */
    BATTERY=33554432, /* 0x2000000 Battery | */
    PROXIMITY=67108864, /* 0x4000000 Proximity | */
    SATCOM=134217728, /* 0x8000000 Satellite Communication  | */
};

//! MAV_SYS_STATUS_SENSOR ENUM_END
constexpr auto MAV_SYS_STATUS_SENSOR_ENUM_END = 134217729;

/** @brief  */
enum class MAV_FRAME : uint8_t
{
    GLOBAL=0, /* Global coordinate frame, WGS84 coordinate system. First value / x: latitude, second value / y: longitude, third value / z: positive altitude over mean sea level (MSL). | */
    LOCAL_NED=1, /* Local coordinate frame, Z-down (x: north, y: east, z: down). | */
    MISSION=2, /* NOT a coordinate frame, indicates a mission command. | */
    GLOBAL_RELATIVE_ALT=3, /* Global coordinate frame, WGS84 coordinate system, relative altitude over ground with respect to the home position. First value / x: latitude, second value / y: longitude, third value / z: positive altitude with 0 being at the altitude of the home location. | */
    LOCAL_ENU=4, /* Local coordinate frame, Z-up (x: east, y: north, z: up). | */
    GLOBAL_INT=5, /* Global coordinate frame, WGS84 coordinate system. First value / x: latitude in degrees*1.0e-7, second value / y: longitude in degrees*1.0e-7, third value / z: positive altitude over mean sea level (MSL). | */
    GLOBAL_RELATIVE_ALT_INT=6, /* Global coordinate frame, WGS84 coordinate system, relative altitude over ground with respect to the home position. First value / x: latitude in degrees*10e-7, second value / y: longitude in degrees*10e-7, third value / z: positive altitude with 0 being at the altitude of the home location. | */
    LOCAL_OFFSET_NED=7, /* Offset to the current local frame. Anything expressed in this frame should be added to the current local frame position. | */
    BODY_NED=8, /* Setpoint in body NED frame. This makes sense if all position control is externalized - e.g. useful to command 2 m/s^2 acceleration to the right. | */
    BODY_OFFSET_NED=9, /* Offset in body NED frame. This makes sense if adding setpoints to the current flight path, to avoid an obstacle - e.g. useful to command 2 m/s^2 acceleration to the east. | */
    GLOBAL_TERRAIN_ALT=10, /* Global coordinate frame with above terrain level altitude. WGS84 coordinate system, relative altitude over terrain with respect to the waypoint coordinate. First value / x: latitude in degrees, second value / y: longitude in degrees, third value / z: positive altitude in meters with 0 being at ground level in terrain model. | */
    GLOBAL_TERRAIN_ALT_INT=11, /* Global coordinate frame with above terrain level altitude. WGS84 coordinate system, relative altitude over terrain with respect to the waypoint coordinate. First value / x: latitude in degrees*10e-7, second value / y: longitude in degrees*10e-7, third value / z: positive altitude in meters with 0 being at ground level in terrain model. | */
    BODY_FRD=12, /* Body fixed frame of reference, Z-down (x: forward, y: right, z: down). | */
    BODY_FLU=13, /* Body fixed frame of reference, Z-up (x: forward, y: left, z: up). | */
    MOCAP_NED=14, /* Odometry local coordinate frame of data given by a motion capture system, Z-down (x: north, y: east, z: down). | */
    MOCAP_ENU=15, /* Odometry local coordinate frame of data given by a motion capture system, Z-up (x: east, y: north, z: up). | */
    VISION_NED=16, /* Odometry local coordinate frame of data given by a vision estimation system, Z-down (x: north, y: east, z: down). | */
    VISION_ENU=17, /* Odometry local coordinate frame of data given by a vision estimation system, Z-up (x: east, y: north, z: up). | */
    ESTIM_NED=18, /* Odometry local coordinate frame of data given by an estimator running onboard the vehicle, Z-down (x: north, y: east, z: down). | */
    ESTIM_ENU=19, /* Odometry local coordinate frame of data given by an estimator running onboard the vehicle, Z-up (x: east, y: noth, z: up). | */
};

//! MAV_FRAME ENUM_END
constexpr auto MAV_FRAME_ENUM_END = 20;

/** @brief  */
enum class MAVLINK_DATA_STREAM_TYPE
{
    IMG_JPEG=1, /*  | */
    IMG_BMP=2, /*  | */
    IMG_RAW8U=3, /*  | */
    IMG_RAW32U=4, /*  | */
    IMG_PGM=5, /*  | */
    IMG_PNG=6, /*  | */
};

//! MAVLINK_DATA_STREAM_TYPE ENUM_END
constexpr auto MAVLINK_DATA_STREAM_TYPE_ENUM_END = 7;

/** @brief  */
enum class FENCE_ACTION
{
    NONE=0, /* Disable fenced mode | */
    GUIDED=1, /* Switched to guided mode to return point (fence point 0) | */
    REPORT=2, /* Report fence breach, but don't take action | */
    GUIDED_THR_PASS=3, /* Switched to guided mode to return point (fence point 0) with manual throttle control | */
    RTL=4, /* Switch to RTL (return to launch) mode and head for the return point. | */
};

//! FENCE_ACTION ENUM_END
constexpr auto FENCE_ACTION_ENUM_END = 5;

/** @brief  */
enum class FENCE_BREACH
{
    NONE=0, /* No last fence breach | */
    MINALT=1, /* Breached minimum altitude | */
    MAXALT=2, /* Breached maximum altitude | */
    BOUNDARY=3, /* Breached fence boundary | */
};

//! FENCE_BREACH ENUM_END
constexpr auto FENCE_BREACH_ENUM_END = 4;

/** @brief Enumeration of possible mount operation modes */
enum class MAV_MOUNT_MODE
{
    RETRACT=0, /* Load and keep safe position (Roll,Pitch,Yaw) from permant memory and stop stabilization | */
    NEUTRAL=1, /* Load and keep neutral position (Roll,Pitch,Yaw) from permanent memory. | */
    MAVLINK_TARGETING=2, /* Load neutral position and start MAVLink Roll,Pitch,Yaw control with stabilization | */
    RC_TARGETING=3, /* Load neutral position and start RC Roll,Pitch,Yaw control with stabilization | */
    GPS_POINT=4, /* Load neutral position and start to point to Lat,Lon,Alt | */
};

//! MAV_MOUNT_MODE ENUM_END
constexpr auto MAV_MOUNT_MODE_ENUM_END = 5;

/** @brief Generalized UAVCAN node health */
enum class UAVCAN_NODE_HEALTH : uint8_t
{
    OK=0, /* The node is functioning properly. | */
    WARNING=1, /* A critical parameter went out of range or the node has encountered a minor failure. | */
    ERROR=2, /* The node has encountered a major failure. | */
    CRITICAL=3, /* The node has suffered a fatal malfunction. | */
};

//! UAVCAN_NODE_HEALTH ENUM_END
constexpr auto UAVCAN_NODE_HEALTH_ENUM_END = 4;

/** @brief Generalized UAVCAN node mode */
enum class UAVCAN_NODE_MODE : uint8_t
{
    OPERATIONAL=0, /* The node is performing its primary functions. | */
    INITIALIZATION=1, /* The node is initializing; this mode is entered immediately after startup. | */
    MAINTENANCE=2, /* The node is under maintenance. | */
    SOFTWARE_UPDATE=3, /* The node is in the process of updating its software. | */
    OFFLINE=7, /* The node is no longer available online. | */
};

//! UAVCAN_NODE_MODE ENUM_END
constexpr auto UAVCAN_NODE_MODE_ENUM_END = 8;

/** @brief A data stream is not a fixed set of messages, but rather a
     recommendation to the autopilot software. Individual autopilots may or may not obey
     the recommended messages. */
enum class MAV_DATA_STREAM
{
    ALL=0, /* Enable all data streams | */
    RAW_SENSORS=1, /* Enable IMU_RAW, GPS_RAW, GPS_STATUS packets. | */
    EXTENDED_STATUS=2, /* Enable GPS_STATUS, CONTROL_STATUS, AUX_STATUS | */
    RC_CHANNELS=3, /* Enable RC_CHANNELS_SCALED, RC_CHANNELS_RAW, SERVO_OUTPUT_RAW | */
    RAW_CONTROLLER=4, /* Enable ATTITUDE_CONTROLLER_OUTPUT, POSITION_CONTROLLER_OUTPUT, NAV_CONTROLLER_OUTPUT. | */
    POSITION=6, /* Enable LOCAL_POSITION, GLOBAL_POSITION/GLOBAL_POSITION_INT messages. | */
    EXTRA1=10, /* Dependent on the autopilot | */
    EXTRA2=11, /* Dependent on the autopilot | */
    EXTRA3=12, /* Dependent on the autopilot | */
};

//! MAV_DATA_STREAM ENUM_END
constexpr auto MAV_DATA_STREAM_ENUM_END = 13;

/** @brief THIS INTERFACE IS DEPRECATED AS OF JANUARY 2018. Please use MAV_CMD_DO_SET_ROI_* messages instead. The ROI (region of interest) for the vehicle. This can be
                be used by the vehicle for camera/vehicle attitude alignment (see
                MAV_CMD_NAV_ROI). */
enum class MAV_ROI
{
    NONE=0, /* No region of interest. | */
    WPNEXT=1, /* Point toward next waypoint, with optional pitch/roll/yaw offset. | */
    WPINDEX=2, /* Point toward given waypoint. | */
    LOCATION=3, /* Point toward fixed location. | */
    TARGET=4, /* Point toward of given id. | */
};

//! MAV_ROI ENUM_END
constexpr auto MAV_ROI_ENUM_END = 5;

/** @brief ACK / NACK / ERROR values as a result of MAV_CMDs and for mission item transmission. */
enum class MAV_CMD_ACK
{
    OK=1, /* Command / mission item is ok. | */
    ERR_FAIL=2, /* Generic error message if none of the other reasons fails or if no detailed error reporting is implemented. | */
    ERR_ACCESS_DENIED=3, /* The system is refusing to accept this command from this source / communication partner. | */
    ERR_NOT_SUPPORTED=4, /* Command or mission item is not supported, other commands would be accepted. | */
    ERR_COORDINATE_FRAME_NOT_SUPPORTED=5, /* The coordinate frame of this command / mission item is not supported. | */
    ERR_COORDINATES_OUT_OF_RANGE=6, /* The coordinate frame of this command is ok, but he coordinate values exceed the safety limits of this system. This is a generic error, please use the more specific error messages below if possible. | */
    ERR_X_LAT_OUT_OF_RANGE=7, /* The X or latitude value is out of range. | */
    ERR_Y_LON_OUT_OF_RANGE=8, /* The Y or longitude value is out of range. | */
    ERR_Z_ALT_OUT_OF_RANGE=9, /* The Z or altitude value is out of range. | */
};

//! MAV_CMD_ACK ENUM_END
constexpr auto MAV_CMD_ACK_ENUM_END = 10;

/** @brief Specifies the datatype of a MAVLink parameter. */
enum class MAV_PARAM_TYPE : uint8_t
{
    UINT8=1, /* 8-bit unsigned integer | */
    INT8=2, /* 8-bit signed integer | */
    UINT16=3, /* 16-bit unsigned integer | */
    INT16=4, /* 16-bit signed integer | */
    UINT32=5, /* 32-bit unsigned integer | */
    INT32=6, /* 32-bit signed integer | */
    UINT64=7, /* 64-bit unsigned integer | */
    INT64=8, /* 64-bit signed integer | */
    REAL32=9, /* 32-bit floating-point | */
    REAL64=10, /* 64-bit floating-point | */
};

//! MAV_PARAM_TYPE ENUM_END
constexpr auto MAV_PARAM_TYPE_ENUM_END = 11;

/** @brief result from a mavlink command */
enum class MAV_RESULT : uint8_t
{
    ACCEPTED=0, /* Command ACCEPTED and EXECUTED | */
    TEMPORARILY_REJECTED=1, /* Command TEMPORARY REJECTED/DENIED | */
    DENIED=2, /* Command PERMANENTLY DENIED | */
    UNSUPPORTED=3, /* Command UNKNOWN/UNSUPPORTED | */
    FAILED=4, /* Command executed, but failed | */
};

//! MAV_RESULT ENUM_END
constexpr auto MAV_RESULT_ENUM_END = 5;

/** @brief result in a MAVLink mission ack */
enum class MAV_MISSION_RESULT : uint8_t
{
    ACCEPTED=0, /* mission accepted OK | */
    ERROR=1, /* generic error / not accepting mission commands at all right now | */
    UNSUPPORTED_FRAME=2, /* coordinate frame is not supported | */
    UNSUPPORTED=3, /* command is not supported | */
    NO_SPACE=4, /* mission item exceeds storage space | */
    INVALID=5, /* one of the parameters has an invalid value | */
    INVALID_PARAM1=6, /* param1 has an invalid value | */
    INVALID_PARAM2=7, /* param2 has an invalid value | */
    INVALID_PARAM3=8, /* param3 has an invalid value | */
    INVALID_PARAM4=9, /* param4 has an invalid value | */
    INVALID_PARAM5_X=10, /* x/param5 has an invalid value | */
    INVALID_PARAM6_Y=11, /* y/param6 has an invalid value | */
    INVALID_PARAM7=12, /* param7 has an invalid value | */
    INVALID_SEQUENCE=13, /* received waypoint out of sequence | */
    DENIED=14, /* not accepting any mission commands from this communication partner | */
};

//! MAV_MISSION_RESULT ENUM_END
constexpr auto MAV_MISSION_RESULT_ENUM_END = 15;

/** @brief Indicates the severity level, generally used for status messages to indicate their relative urgency. Based on RFC-5424 using expanded definitions at: http://www.kiwisyslog.com/kb/info:-syslog-message-levels/. */
enum class MAV_SEVERITY : uint8_t
{
    EMERGENCY=0, /* System is unusable. This is a "panic" condition. | */
    ALERT=1, /* Action should be taken immediately. Indicates error in non-critical systems. | */
    CRITICAL=2, /* Action must be taken immediately. Indicates failure in a primary system. | */
    ERROR=3, /* Indicates an error in secondary/redundant systems. | */
    WARNING=4, /* Indicates about a possible future error if this is not resolved within a given timeframe. Example would be a low battery warning. | */
    NOTICE=5, /* An unusual event has occurred, though not an error condition. This should be investigated for the root cause. | */
    INFO=6, /* Normal operational messages. Useful for logging. No action is required for these messages. | */
    DEBUG=7, /* Useful non-operational messages that can assist in debugging. These should not occur during normal operation. | */
};

//! MAV_SEVERITY ENUM_END
constexpr auto MAV_SEVERITY_ENUM_END = 8;

/** @brief Power supply status flags (bitmask) */
enum class MAV_POWER_STATUS : uint16_t
{
    BRICK_VALID=1, /* main brick power supply valid | */
    SERVO_VALID=2, /* main servo power supply valid for FMU | */
    USB_CONNECTED=4, /* USB power is connected | */
    PERIPH_OVERCURRENT=8, /* peripheral supply is in over-current state | */
    PERIPH_HIPOWER_OVERCURRENT=16, /* hi-power peripheral supply is in over-current state | */
    CHANGED=32, /* Power status has changed since boot | */
};

//! MAV_POWER_STATUS ENUM_END
constexpr auto MAV_POWER_STATUS_ENUM_END = 33;

/** @brief SERIAL_CONTROL device types */
enum class SERIAL_CONTROL_DEV : uint8_t
{
    TELEM1=0, /* First telemetry port | */
    TELEM2=1, /* Second telemetry port | */
    GPS1=2, /* First GPS port | */
    GPS2=3, /* Second GPS port | */
    SHELL=10, /* system shell | */
};

//! SERIAL_CONTROL_DEV ENUM_END
constexpr auto SERIAL_CONTROL_DEV_ENUM_END = 11;

/** @brief SERIAL_CONTROL flags (bitmask) */
enum class SERIAL_CONTROL_FLAG : uint8_t
{
    REPLY=1, /* Set if this is a reply | */
    RESPOND=2, /* Set if the sender wants the receiver to send a response as another SERIAL_CONTROL message | */
    EXCLUSIVE=4, /* Set if access to the serial port should be removed from whatever driver is currently using it, giving exclusive access to the SERIAL_CONTROL protocol. The port can be handed back by sending a request without this flag set | */
    BLOCKING=8, /* Block on writes to the serial port | */
    MULTI=16, /* Send multiple replies until port is drained | */
};

//! SERIAL_CONTROL_FLAG ENUM_END
constexpr auto SERIAL_CONTROL_FLAG_ENUM_END = 17;

/** @brief Enumeration of distance sensor types */
enum class MAV_DISTANCE_SENSOR : uint8_t
{
    LASER=0, /* Laser rangefinder, e.g. LightWare SF02/F or PulsedLight units | */
    ULTRASOUND=1, /* Ultrasound rangefinder, e.g. MaxBotix units | */
    INFRARED=2, /* Infrared rangefinder, e.g. Sharp units | */
    RADAR=3, /* Radar type, e.g. uLanding units | */
    UNKNOWN=4, /* Broken or unknown type, e.g. analog units | */
};

//! MAV_DISTANCE_SENSOR ENUM_END
constexpr auto MAV_DISTANCE_SENSOR_ENUM_END = 5;

/** @brief Enumeration of sensor orientation, according to its rotations */
enum class MAV_SENSOR_ORIENTATION : uint8_t
{
    ROTATION_NONE=0, /* Roll: 0, Pitch: 0, Yaw: 0 | */
    ROTATION_YAW_45=1, /* Roll: 0, Pitch: 0, Yaw: 45 | */
    ROTATION_YAW_90=2, /* Roll: 0, Pitch: 0, Yaw: 90 | */
    ROTATION_YAW_135=3, /* Roll: 0, Pitch: 0, Yaw: 135 | */
    ROTATION_YAW_180=4, /* Roll: 0, Pitch: 0, Yaw: 180 | */
    ROTATION_YAW_225=5, /* Roll: 0, Pitch: 0, Yaw: 225 | */
    ROTATION_YAW_270=6, /* Roll: 0, Pitch: 0, Yaw: 270 | */
    ROTATION_YAW_315=7, /* Roll: 0, Pitch: 0, Yaw: 315 | */
    ROTATION_ROLL_180=8, /* Roll: 180, Pitch: 0, Yaw: 0 | */
    ROTATION_ROLL_180_YAW_45=9, /* Roll: 180, Pitch: 0, Yaw: 45 | */
    ROTATION_ROLL_180_YAW_90=10, /* Roll: 180, Pitch: 0, Yaw: 90 | */
    ROTATION_ROLL_180_YAW_135=11, /* Roll: 180, Pitch: 0, Yaw: 135 | */
    ROTATION_PITCH_180=12, /* Roll: 0, Pitch: 180, Yaw: 0 | */
    ROTATION_ROLL_180_YAW_225=13, /* Roll: 180, Pitch: 0, Yaw: 225 | */
    ROTATION_ROLL_180_YAW_270=14, /* Roll: 180, Pitch: 0, Yaw: 270 | */
    ROTATION_ROLL_180_YAW_315=15, /* Roll: 180, Pitch: 0, Yaw: 315 | */
    ROTATION_ROLL_90=16, /* Roll: 90, Pitch: 0, Yaw: 0 | */
    ROTATION_ROLL_90_YAW_45=17, /* Roll: 90, Pitch: 0, Yaw: 45 | */
    ROTATION_ROLL_90_YAW_90=18, /* Roll: 90, Pitch: 0, Yaw: 90 | */
    ROTATION_ROLL_90_YAW_135=19, /* Roll: 90, Pitch: 0, Yaw: 135 | */
    ROTATION_ROLL_270=20, /* Roll: 270, Pitch: 0, Yaw: 0 | */
    ROTATION_ROLL_270_YAW_45=21, /* Roll: 270, Pitch: 0, Yaw: 45 | */
    ROTATION_ROLL_270_YAW_90=22, /* Roll: 270, Pitch: 0, Yaw: 90 | */
    ROTATION_ROLL_270_YAW_135=23, /* Roll: 270, Pitch: 0, Yaw: 135 | */
    ROTATION_PITCH_90=24, /* Roll: 0, Pitch: 90, Yaw: 0 | */
    ROTATION_PITCH_270=25, /* Roll: 0, Pitch: 270, Yaw: 0 | */
    ROTATION_PITCH_180_YAW_90=26, /* Roll: 0, Pitch: 180, Yaw: 90 | */
    ROTATION_PITCH_180_YAW_270=27, /* Roll: 0, Pitch: 180, Yaw: 270 | */
    ROTATION_ROLL_90_PITCH_90=28, /* Roll: 90, Pitch: 90, Yaw: 0 | */
    ROTATION_ROLL_180_PITCH_90=29, /* Roll: 180, Pitch: 90, Yaw: 0 | */
    ROTATION_ROLL_270_PITCH_90=30, /* Roll: 270, Pitch: 90, Yaw: 0 | */
    ROTATION_ROLL_90_PITCH_180=31, /* Roll: 90, Pitch: 180, Yaw: 0 | */
    ROTATION_ROLL_270_PITCH_180=32, /* Roll: 270, Pitch: 180, Yaw: 0 | */
    ROTATION_ROLL_90_PITCH_270=33, /* Roll: 90, Pitch: 270, Yaw: 0 | */
    ROTATION_ROLL_180_PITCH_270=34, /* Roll: 180, Pitch: 270, Yaw: 0 | */
    ROTATION_ROLL_270_PITCH_270=35, /* Roll: 270, Pitch: 270, Yaw: 0 | */
    ROTATION_ROLL_90_PITCH_180_YAW_90=36, /* Roll: 90, Pitch: 180, Yaw: 90 | */
    ROTATION_ROLL_90_YAW_270=37, /* Roll: 90, Pitch: 0, Yaw: 270 | */
    ROTATION_ROLL_90_PITCH_68_YAW_293=38, /* Roll: 90, Pitch: 68, Yaw: 293 | */
    ROTATION_PITCH_315=39, /* Pitch: 315 | */
    ROTATION_ROLL_90_PITCH_315=40, /* Roll: 90, Pitch: 315 | */
    ROTATION_CUSTOM=100, /* Custom orientation | */
};

//! MAV_SENSOR_ORIENTATION ENUM_END
constexpr auto MAV_SENSOR_ORIENTATION_ENUM_END = 101;

/** @brief Bitmask of (optional) autopilot capabilities (64 bit). If a bit is set, the autopilot supports this capability. */
enum class MAV_PROTOCOL_CAPABILITY : uint64_t
{
    MISSION_FLOAT=1, /* Autopilot supports MISSION float message type. | */
    PARAM_FLOAT=2, /* Autopilot supports the new param float message type. | */
    MISSION_INT=4, /* Autopilot supports MISSION_INT scaled integer message type. | */
    COMMAND_INT=8, /* Autopilot supports COMMAND_INT scaled integer message type. | */
    PARAM_UNION=16, /* Autopilot supports the new param union message type. | */
    FTP=32, /* Autopilot supports the new FILE_TRANSFER_PROTOCOL message type. | */
    SET_ATTITUDE_TARGET=64, /* Autopilot supports commanding attitude offboard. | */
    SET_POSITION_TARGET_LOCAL_NED=128, /* Autopilot supports commanding position and velocity targets in local NED frame. | */
    SET_POSITION_TARGET_GLOBAL_INT=256, /* Autopilot supports commanding position and velocity targets in global scaled integers. | */
    TERRAIN=512, /* Autopilot supports terrain protocol / data handling. | */
    SET_ACTUATOR_TARGET=1024, /* Autopilot supports direct actuator control. | */
    FLIGHT_TERMINATION=2048, /* Autopilot supports the flight termination command. | */
    COMPASS_CALIBRATION=4096, /* Autopilot supports onboard compass calibration. | */
    MAVLINK2=8192, /* Autopilot supports MAVLink version 2. | */
    MISSION_FENCE=16384, /* Autopilot supports mission fence protocol. | */
    MISSION_RALLY=32768, /* Autopilot supports mission rally point protocol. | */
    FLIGHT_INFORMATION=65536, /* Autopilot supports the flight information protocol. | */
};

//! MAV_PROTOCOL_CAPABILITY ENUM_END
constexpr auto MAV_PROTOCOL_CAPABILITY_ENUM_END = 65537;

/** @brief Type of mission items being requested/sent in mission protocol. */
enum class MAV_MISSION_TYPE : uint8_t
{
    MISSION=0, /* Items are mission commands for main mission. | */
    FENCE=1, /* Specifies GeoFence area(s). Items are MAV_CMD_FENCE_ GeoFence items. | */
    RALLY=2, /* Specifies the rally points for the vehicle. Rally points are alternative RTL points. Items are MAV_CMD_RALLY_POINT rally point items. | */
    ALL=255, /* Only used in MISSION_CLEAR_ALL to clear all mission types. | */
};

//! MAV_MISSION_TYPE ENUM_END
constexpr auto MAV_MISSION_TYPE_ENUM_END = 256;

/** @brief Enumeration of estimator types */
enum class MAV_ESTIMATOR_TYPE : uint8_t
{
    NAIVE=1, /* This is a naive estimator without any real covariance feedback. | */
    VISION=2, /* Computer vision based estimate. Might be up to scale. | */
    VIO=3, /* Visual-inertial estimate. | */
    GPS=4, /* Plain GPS estimate. | */
    GPS_INS=5, /* Estimator integrating GPS and inertial sensing. | */
};

//! MAV_ESTIMATOR_TYPE ENUM_END
constexpr auto MAV_ESTIMATOR_TYPE_ENUM_END = 6;

/** @brief Enumeration of battery types */
enum class MAV_BATTERY_TYPE : uint8_t
{
    UNKNOWN=0, /* Not specified. | */
    LIPO=1, /* Lithium polymer battery | */
    LIFE=2, /* Lithium-iron-phosphate battery | */
    LION=3, /* Lithium-ION battery | */
    NIMH=4, /* Nickel metal hydride battery | */
};

//! MAV_BATTERY_TYPE ENUM_END
constexpr auto MAV_BATTERY_TYPE_ENUM_END = 5;

/** @brief Enumeration of battery functions */
enum class MAV_BATTERY_FUNCTION : uint8_t
{
    UNKNOWN=0, /* Battery function is unknown | */
    ALL=1, /* Battery supports all flight systems | */
    PROPULSION=2, /* Battery for the propulsion system | */
    AVIONICS=3, /* Avionics battery | */
    TYPE_PAYLOAD=4, /* Payload battery | */
};

//! MAV_BATTERY_FUNCTION ENUM_END
constexpr auto MAV_BATTERY_FUNCTION_ENUM_END = 5;

/** @brief Enumeration for low battery states. */
enum class MAV_BATTERY_CHARGE_STATE : uint8_t
{
    UNDEFINED=0, /* Low battery state is not provided | */
    OK=1, /* Battery is not in low state. Normal operation. | */
    LOW=2, /* Battery state is low, warn and monitor close. | */
    CRITICAL=3, /* Battery state is critical, return or abort immediately. | */
    EMERGENCY=4, /* Battery state is too low for ordinary abort sequence. Perform fastest possible emergency stop to prevent damage. | */
    FAILED=5, /* Battery failed, damage unavoidable. | */
    UNHEALTHY=6, /* Battery is diagnosed to be defective or an error occurred, usage is discouraged / prohibited. | */
};

//! MAV_BATTERY_CHARGE_STATE ENUM_END
constexpr auto MAV_BATTERY_CHARGE_STATE_ENUM_END = 7;

/** @brief Enumeration of VTOL states */
enum class MAV_VTOL_STATE : uint8_t
{
    UNDEFINED=0, /* MAV is not configured as VTOL | */
    TRANSITION_TO_FW=1, /* VTOL is in transition from multicopter to fixed-wing | */
    TRANSITION_TO_MC=2, /* VTOL is in transition from fixed-wing to multicopter | */
    MC=3, /* VTOL is in multicopter state | */
    FW=4, /* VTOL is in fixed-wing state | */
};

//! MAV_VTOL_STATE ENUM_END
constexpr auto MAV_VTOL_STATE_ENUM_END = 5;

/** @brief Enumeration of landed detector states */
enum class MAV_LANDED_STATE : uint8_t
{
    UNDEFINED=0, /* MAV landed state is unknown | */
    ON_GROUND=1, /* MAV is landed (on ground) | */
    IN_AIR=2, /* MAV is in air | */
    TAKEOFF=3, /* MAV currently taking off | */
    LANDING=4, /* MAV currently landing | */
};

//! MAV_LANDED_STATE ENUM_END
constexpr auto MAV_LANDED_STATE_ENUM_END = 5;

/** @brief Enumeration of the ADSB altimeter types */
enum class ADSB_ALTITUDE_TYPE : uint8_t
{
    PRESSURE_QNH=0, /* Altitude reported from a Baro source using QNH reference | */
    GEOMETRIC=1, /* Altitude reported from a GNSS source | */
};

//! ADSB_ALTITUDE_TYPE ENUM_END
constexpr auto ADSB_ALTITUDE_TYPE_ENUM_END = 2;

/** @brief ADSB classification for the type of vehicle emitting the transponder signal */
enum class ADSB_EMITTER_TYPE : uint8_t
{
    NO_INFO=0, /*  | */
    LIGHT=1, /*  | */
    SMALL=2, /*  | */
    LARGE=3, /*  | */
    HIGH_VORTEX_LARGE=4, /*  | */
    HEAVY=5, /*  | */
    HIGHLY_MANUV=6, /*  | */
    ROTOCRAFT=7, /*  | */
    UNASSIGNED=8, /*  | */
    GLIDER=9, /*  | */
    LIGHTER_AIR=10, /*  | */
    PARACHUTE=11, /*  | */
    ULTRA_LIGHT=12, /*  | */
    UNASSIGNED2=13, /*  | */
    UAV=14, /*  | */
    SPACE=15, /*  | */
    UNASSGINED3=16, /*  | */
    EMERGENCY_SURFACE=17, /*  | */
    SERVICE_SURFACE=18, /*  | */
    POINT_OBSTACLE=19, /*  | */
};

//! ADSB_EMITTER_TYPE ENUM_END
constexpr auto ADSB_EMITTER_TYPE_ENUM_END = 20;

/** @brief These flags indicate status such as data validity of each data source. Set = data valid */
enum class ADSB_FLAGS : uint16_t
{
    VALID_COORDS=1, /*  | */
    VALID_ALTITUDE=2, /*  | */
    VALID_HEADING=4, /*  | */
    VALID_VELOCITY=8, /*  | */
    VALID_CALLSIGN=16, /*  | */
    VALID_SQUAWK=32, /*  | */
    SIMULATED=64, /*  | */
};

//! ADSB_FLAGS ENUM_END
constexpr auto ADSB_FLAGS_ENUM_END = 65;

/** @brief Bitmap of options for the MAV_CMD_DO_REPOSITION */
enum class MAV_DO_REPOSITION_FLAGS
{
    CHANGE_MODE=1, /* The aircraft should immediately transition into guided. This should not be set for follow me applications | */
};

//! MAV_DO_REPOSITION_FLAGS ENUM_END
constexpr auto MAV_DO_REPOSITION_FLAGS_ENUM_END = 2;

/** @brief Flags in EKF_STATUS message */
enum class ESTIMATOR_STATUS_FLAGS : uint16_t
{
    ATTITUDE=1, /* True if the attitude estimate is good | */
    VELOCITY_HORIZ=2, /* True if the horizontal velocity estimate is good | */
    VELOCITY_VERT=4, /* True if the  vertical velocity estimate is good | */
    POS_HORIZ_REL=8, /* True if the horizontal position (relative) estimate is good | */
    POS_HORIZ_ABS=16, /* True if the horizontal position (absolute) estimate is good | */
    POS_VERT_ABS=32, /* True if the vertical position (absolute) estimate is good | */
    POS_VERT_AGL=64, /* True if the vertical position (above ground) estimate is good | */
    CONST_POS_MODE=128, /* True if the EKF is in a constant position mode and is not using external measurements (eg GPS or optical flow) | */
    PRED_POS_HORIZ_REL=256, /* True if the EKF has sufficient data to enter a mode that will provide a (relative) position estimate | */
    PRED_POS_HORIZ_ABS=512, /* True if the EKF has sufficient data to enter a mode that will provide a (absolute) position estimate | */
    GPS_GLITCH=1024, /* True if the EKF has detected a GPS glitch | */
    ACCEL_ERROR=2048, /* True if the EKF has detected bad accelerometer data | */
};

//! ESTIMATOR_STATUS_FLAGS ENUM_END
constexpr auto ESTIMATOR_STATUS_FLAGS_ENUM_END = 2049;

/** @brief  */
enum class MOTOR_TEST_ORDER
{
    DEFAULT=0, /* default autopilot motor test method | */
    SEQUENCE=1, /* motor numbers are specified as their index in a predefined vehicle-specific sequence | */
    BOARD=2, /* motor numbers are specified as the output as labeled on the board | */
};

//! MOTOR_TEST_ORDER ENUM_END
constexpr auto MOTOR_TEST_ORDER_ENUM_END = 3;

/** @brief  */
enum class MOTOR_TEST_THROTTLE_TYPE
{
    PERCENT=0, /* throttle as a percentage from 0 ~ 100 | */
    PWM=1, /* throttle as an absolute PWM value (normally in range of 1000~2000) | */
    PILOT=2, /* throttle pass-through from pilot's transmitter | */
    COMPASS_CAL=3, /* per-motor compass calibration test | */
};

//! MOTOR_TEST_THROTTLE_TYPE ENUM_END
constexpr auto MOTOR_TEST_THROTTLE_TYPE_ENUM_END = 4;

/** @brief  */
enum class GPS_INPUT_IGNORE_FLAGS : uint16_t
{
    FLAG_ALT=1, /* ignore altitude field | */
    FLAG_HDOP=2, /* ignore hdop field | */
    FLAG_VDOP=4, /* ignore vdop field | */
    FLAG_VEL_HORIZ=8, /* ignore horizontal velocity field (vn and ve) | */
    FLAG_VEL_VERT=16, /* ignore vertical velocity field (vd) | */
    FLAG_SPEED_ACCURACY=32, /* ignore speed accuracy field | */
    FLAG_HORIZONTAL_ACCURACY=64, /* ignore horizontal accuracy field | */
    FLAG_VERTICAL_ACCURACY=128, /* ignore vertical accuracy field | */
};

//! GPS_INPUT_IGNORE_FLAGS ENUM_END
constexpr auto GPS_INPUT_IGNORE_FLAGS_ENUM_END = 129;

/** @brief Possible actions an aircraft can take to avoid a collision. */
enum class MAV_COLLISION_ACTION : uint8_t
{
    NONE=0, /* Ignore any potential collisions | */
    REPORT=1, /* Report potential collision | */
    ASCEND_OR_DESCEND=2, /* Ascend or Descend to avoid threat | */
    MOVE_HORIZONTALLY=3, /* Move horizontally to avoid threat | */
    MOVE_PERPENDICULAR=4, /* Aircraft to move perpendicular to the collision's velocity vector | */
    RTL=5, /* Aircraft to fly directly back to its launch point | */
    HOVER=6, /* Aircraft to stop in place | */
};

//! MAV_COLLISION_ACTION ENUM_END
constexpr auto MAV_COLLISION_ACTION_ENUM_END = 7;

/** @brief Aircraft-rated danger from this threat. */
enum class MAV_COLLISION_THREAT_LEVEL : uint8_t
{
    NONE=0, /* Not a threat | */
    LOW=1, /* Craft is mildly concerned about this threat | */
    HIGH=2, /* Craft is panicing, and may take actions to avoid threat | */
};

//! MAV_COLLISION_THREAT_LEVEL ENUM_END
constexpr auto MAV_COLLISION_THREAT_LEVEL_ENUM_END = 3;

/** @brief Source of information about this collision. */
enum class MAV_COLLISION_SRC : uint8_t
{
    ADSB=0, /* ID field references ADSB_VEHICLE packets | */
    MAVLINK_GPS_GLOBAL_INT=1, /* ID field references MAVLink SRC ID | */
};

//! MAV_COLLISION_SRC ENUM_END
constexpr auto MAV_COLLISION_SRC_ENUM_END = 2;

/** @brief Type of GPS fix */
enum class GPS_FIX_TYPE : uint8_t
{
    NO_GPS=0, /* No GPS connected | */
    NO_FIX=1, /* No position information, GPS is connected | */
    TYPE_2D_FIX=2, /* 2D position | */
    TYPE_3D_FIX=3, /* 3D position | */
    DGPS=4, /* DGPS/SBAS aided 3D position | */
    RTK_FLOAT=5, /* RTK float, 3D position | */
    RTK_FIXED=6, /* RTK Fixed, 3D position | */
    STATIC=7, /* Static fixed, typically used for base stations | */
    PPP=8, /* PPP, 3D position. | */
};

//! GPS_FIX_TYPE ENUM_END
constexpr auto GPS_FIX_TYPE_ENUM_END = 9;

/** @brief RTK GPS baseline coordinate system, used for RTK corrections */
enum class RTK_BASELINE_COORDINATE_SYSTEM : uint8_t
{
    ECEF=0, /* Earth-centered, Earth-fixed | */
    NED=1, /* North, East, Down | */
};

//! RTK_BASELINE_COORDINATE_SYSTEM ENUM_END
constexpr auto RTK_BASELINE_COORDINATE_SYSTEM_ENUM_END = 2;

/** @brief Type of landing target */
enum class LANDING_TARGET_TYPE : uint8_t
{
    LIGHT_BEACON=0, /* Landing target signaled by light beacon (ex: IR-LOCK) | */
    RADIO_BEACON=1, /* Landing target signaled by radio beacon (ex: ILS, NDB) | */
    VISION_FIDUCIAL=2, /* Landing target represented by a fiducial marker (ex: ARTag) | */
    VISION_OTHER=3, /* Landing target represented by a pre-defined visual shape/feature (ex: X-marker, H-marker, square) | */
};

//! LANDING_TARGET_TYPE ENUM_END
constexpr auto LANDING_TARGET_TYPE_ENUM_END = 4;

/** @brief Direction of VTOL transition */
enum class VTOL_TRANSITION_HEADING
{
    VEHICLE_DEFAULT=0, /* Respect the heading configuration of the vehicle. | */
    NEXT_WAYPOINT=1, /* Use the heading pointing towards the next waypoint. | */
    TAKEOFF=2, /* Use the heading on takeoff (while sitting on the ground). | */
    SPECIFIED=3, /* Use the specified heading in parameter 4. | */
    ANY=4, /* Use the current heading when reaching takeoff altitude (potentially facing the wind when weather-vaning is active). | */
};

//! VTOL_TRANSITION_HEADING ENUM_END
constexpr auto VTOL_TRANSITION_HEADING_ENUM_END = 5;

/** @brief Camera capability flags (Bitmap). */
enum class CAMERA_CAP_FLAGS : uint32_t
{
    CAPTURE_VIDEO=1, /* Camera is able to record video. | */
    CAPTURE_IMAGE=2, /* Camera is able to capture images. | */
    HAS_MODES=4, /* Camera has separate Video and Image/Photo modes (MAV_CMD_SET_CAMERA_MODE) | */
    CAN_CAPTURE_IMAGE_IN_VIDEO_MODE=8, /* Camera can capture images while in video mode | */
    CAN_CAPTURE_VIDEO_IN_IMAGE_MODE=16, /* Camera can capture videos while in Photo/Image mode | */
    HAS_IMAGE_SURVEY_MODE=32, /* Camera has image survey mode (MAV_CMD_SET_CAMERA_MODE) | */
};

//! CAMERA_CAP_FLAGS ENUM_END
constexpr auto CAMERA_CAP_FLAGS_ENUM_END = 33;

/** @brief Camera Modes. */
enum class CAMERA_MODE : uint8_t
{
    IMAGE=0, /* Camera is in image/photo capture mode. | */
    VIDEO=1, /* Camera is in video capture mode. | */
    IMAGE_SURVEY=2, /* Camera is in image survey capture mode. It allows for camera controller to do specific settings for surveys. | */
};

//! CAMERA_MODE ENUM_END
constexpr auto CAMERA_MODE_ENUM_END = 3;

/** @brief  */
enum class MAV_ARM_AUTH_DENIED_REASON
{
    GENERIC=0, /* Not a specific reason | */
    NONE=1, /* Authorizer will send the error as string to GCS | */
    INVALID_WAYPOINT=2, /* At least one waypoint have a invalid value | */
    TIMEOUT=3, /* Timeout in the authorizer process(in case it depends on network) | */
    AIRSPACE_IN_USE=4, /* Airspace of the mission in use by another vehicle, second result parameter can have the waypoint id that caused it to be denied. | */
    BAD_WEATHER=5, /* Weather is not good to fly | */
};

//! MAV_ARM_AUTH_DENIED_REASON ENUM_END
constexpr auto MAV_ARM_AUTH_DENIED_REASON_ENUM_END = 6;

/** @brief RC type */
enum class RC_TYPE
{
    SPEKTRUM_DSM2=0, /* Spektrum DSM2 | */
    SPEKTRUM_DSMX=1, /* Spektrum DSMX | */
};

//! RC_TYPE ENUM_END
constexpr auto RC_TYPE_ENUM_END = 2;


} // namespace common
} // namespace mavlink

// MESSAGE DEFINITIONS
#include "./mavlink_msg_heartbeat.hpp"
#include "./mavlink_msg_sys_status.hpp"
#include "./mavlink_msg_system_time.hpp"
#include "./mavlink_msg_ping.hpp"
#include "./mavlink_msg_change_operator_control.hpp"
#include "./mavlink_msg_change_operator_control_ack.hpp"
#include "./mavlink_msg_auth_key.hpp"
#include "./mavlink_msg_set_mode.hpp"
#include "./mavlink_msg_param_request_read.hpp"
#include "./mavlink_msg_param_request_list.hpp"
#include "./mavlink_msg_param_value.hpp"
#include "./mavlink_msg_param_set.hpp"
#include "./mavlink_msg_gps_raw_int.hpp"
#include "./mavlink_msg_gps_status.hpp"
#include "./mavlink_msg_scaled_imu.hpp"
#include "./mavlink_msg_raw_imu.hpp"
#include "./mavlink_msg_raw_pressure.hpp"
#include "./mavlink_msg_scaled_pressure.hpp"
#include "./mavlink_msg_attitude.hpp"
#include "./mavlink_msg_attitude_quaternion.hpp"
#include "./mavlink_msg_local_position_ned.hpp"
#include "./mavlink_msg_global_position_int.hpp"
#include "./mavlink_msg_rc_channels_scaled.hpp"
#include "./mavlink_msg_rc_channels_raw.hpp"
#include "./mavlink_msg_servo_output_raw.hpp"
#include "./mavlink_msg_mission_request_partial_list.hpp"
#include "./mavlink_msg_mission_write_partial_list.hpp"
#include "./mavlink_msg_mission_item.hpp"
#include "./mavlink_msg_mission_request.hpp"
#include "./mavlink_msg_mission_set_current.hpp"
#include "./mavlink_msg_mission_current.hpp"
#include "./mavlink_msg_mission_request_list.hpp"
#include "./mavlink_msg_mission_count.hpp"
#include "./mavlink_msg_mission_clear_all.hpp"
#include "./mavlink_msg_mission_item_reached.hpp"
#include "./mavlink_msg_mission_ack.hpp"
#include "./mavlink_msg_set_gps_global_origin.hpp"
#include "./mavlink_msg_gps_global_origin.hpp"
#include "./mavlink_msg_param_map_rc.hpp"
#include "./mavlink_msg_mission_request_int.hpp"
#include "./mavlink_msg_safety_set_allowed_area.hpp"
#include "./mavlink_msg_safety_allowed_area.hpp"
#include "./mavlink_msg_attitude_quaternion_cov.hpp"
#include "./mavlink_msg_nav_controller_output.hpp"
#include "./mavlink_msg_global_position_int_cov.hpp"
#include "./mavlink_msg_local_position_ned_cov.hpp"
#include "./mavlink_msg_rc_channels.hpp"
#include "./mavlink_msg_request_data_stream.hpp"
#include "./mavlink_msg_data_stream.hpp"
#include "./mavlink_msg_manual_control.hpp"
#include "./mavlink_msg_rc_channels_override.hpp"
#include "./mavlink_msg_mission_item_int.hpp"
#include "./mavlink_msg_vfr_hud.hpp"
#include "./mavlink_msg_command_int.hpp"
#include "./mavlink_msg_command_long.hpp"
#include "./mavlink_msg_command_ack.hpp"
#include "./mavlink_msg_manual_setpoint.hpp"
#include "./mavlink_msg_set_attitude_target.hpp"
#include "./mavlink_msg_attitude_target.hpp"
#include "./mavlink_msg_set_position_target_local_ned.hpp"
#include "./mavlink_msg_position_target_local_ned.hpp"
#include "./mavlink_msg_set_position_target_global_int.hpp"
#include "./mavlink_msg_position_target_global_int.hpp"
#include "./mavlink_msg_local_position_ned_system_global_offset.hpp"
#include "./mavlink_msg_hil_state.hpp"
#include "./mavlink_msg_hil_controls.hpp"
#include "./mavlink_msg_hil_rc_inputs_raw.hpp"
#include "./mavlink_msg_hil_actuator_controls.hpp"
#include "./mavlink_msg_optical_flow.hpp"
#include "./mavlink_msg_global_vision_position_estimate.hpp"
#include "./mavlink_msg_vision_position_estimate.hpp"
#include "./mavlink_msg_vision_speed_estimate.hpp"
#include "./mavlink_msg_vicon_position_estimate.hpp"
#include "./mavlink_msg_highres_imu.hpp"
#include "./mavlink_msg_optical_flow_rad.hpp"
#include "./mavlink_msg_hil_sensor.hpp"
#include "./mavlink_msg_sim_state.hpp"
#include "./mavlink_msg_radio_status.hpp"
#include "./mavlink_msg_file_transfer_protocol.hpp"
#include "./mavlink_msg_timesync.hpp"
#include "./mavlink_msg_camera_trigger.hpp"
#include "./mavlink_msg_hil_gps.hpp"
#include "./mavlink_msg_hil_optical_flow.hpp"
#include "./mavlink_msg_hil_state_quaternion.hpp"
#include "./mavlink_msg_scaled_imu2.hpp"
#include "./mavlink_msg_log_request_list.hpp"
#include "./mavlink_msg_log_entry.hpp"
#include "./mavlink_msg_log_request_data.hpp"
#include "./mavlink_msg_log_data.hpp"
#include "./mavlink_msg_log_erase.hpp"
#include "./mavlink_msg_log_request_end.hpp"
#include "./mavlink_msg_gps_inject_data.hpp"
#include "./mavlink_msg_gps2_raw.hpp"
#include "./mavlink_msg_power_status.hpp"
#include "./mavlink_msg_serial_control.hpp"
#include "./mavlink_msg_gps_rtk.hpp"
#include "./mavlink_msg_gps2_rtk.hpp"
#include "./mavlink_msg_scaled_imu3.hpp"
#include "./mavlink_msg_data_transmission_handshake.hpp"
#include "./mavlink_msg_encapsulated_data.hpp"
#include "./mavlink_msg_distance_sensor.hpp"
#include "./mavlink_msg_terrain_request.hpp"
#include "./mavlink_msg_terrain_data.hpp"
#include "./mavlink_msg_terrain_check.hpp"
#include "./mavlink_msg_terrain_report.hpp"
#include "./mavlink_msg_scaled_pressure2.hpp"
#include "./mavlink_msg_att_pos_mocap.hpp"
#include "./mavlink_msg_set_actuator_control_target.hpp"
#include "./mavlink_msg_actuator_control_target.hpp"
#include "./mavlink_msg_altitude.hpp"
#include "./mavlink_msg_resource_request.hpp"
#include "./mavlink_msg_scaled_pressure3.hpp"
#include "./mavlink_msg_follow_target.hpp"
#include "./mavlink_msg_control_system_state.hpp"
#include "./mavlink_msg_battery_status.hpp"
#include "./mavlink_msg_autopilot_version.hpp"
#include "./mavlink_msg_landing_target.hpp"
#include "./mavlink_msg_estimator_status.hpp"
#include "./mavlink_msg_wind_cov.hpp"
#include "./mavlink_msg_gps_input.hpp"
#include "./mavlink_msg_gps_rtcm_data.hpp"
#include "./mavlink_msg_high_latency.hpp"
#include "./mavlink_msg_vibration.hpp"
#include "./mavlink_msg_home_position.hpp"
#include "./mavlink_msg_set_home_position.hpp"
#include "./mavlink_msg_message_interval.hpp"
#include "./mavlink_msg_extended_sys_state.hpp"
#include "./mavlink_msg_adsb_vehicle.hpp"
#include "./mavlink_msg_collision.hpp"
#include "./mavlink_msg_v2_extension.hpp"
#include "./mavlink_msg_memory_vect.hpp"
#include "./mavlink_msg_debug_vect.hpp"
#include "./mavlink_msg_named_value_float.hpp"
#include "./mavlink_msg_named_value_int.hpp"
#include "./mavlink_msg_statustext.hpp"
#include "./mavlink_msg_debug.hpp"
#include "./mavlink_msg_setup_signing.hpp"
#include "./mavlink_msg_button_change.hpp"
#include "./mavlink_msg_play_tune.hpp"
#include "./mavlink_msg_camera_information.hpp"
#include "./mavlink_msg_camera_settings.hpp"
#include "./mavlink_msg_storage_information.hpp"
#include "./mavlink_msg_camera_capture_status.hpp"
#include "./mavlink_msg_camera_image_captured.hpp"
#include "./mavlink_msg_flight_information.hpp"
#include "./mavlink_msg_mount_orientation.hpp"
#include "./mavlink_msg_logging_data.hpp"
#include "./mavlink_msg_logging_data_acked.hpp"
#include "./mavlink_msg_logging_ack.hpp"
#include "./mavlink_msg_wifi_config_ap.hpp"
#include "./mavlink_msg_uavcan_node_status.hpp"
#include "./mavlink_msg_uavcan_node_info.hpp"
#include "./mavlink_msg_obstacle_distance.hpp"
#include "./mavlink_msg_odometry.hpp"

// base include

