#include <stdint.h>

#include "flightmode.h"

#include <openhd/mavlink.h>


const char * sub_mode_from_enum(SUB_MODE mode) {
    switch (mode) {
       case SUB_MODE_MANUAL:
            return "MAN";
       case SUB_MODE_ACRO:
            return "ACRO";
       case SUB_MODE_AUTO:
            return "AUTO";
       case SUB_MODE_GUIDED:
            return "GUIDED";
       case SUB_MODE_STABILIZE:
            return "STAB";
       case SUB_MODE_ALT_HOLD:
            return "ALTHOLD";
       case SUB_MODE_CIRCLE:
            return "CIRCLE";
       case SUB_MODE_SURFACE:
            return "SURFACE";
       case SUB_MODE_POSHOLD:
            return "POSHOLD";
    }
    return "-----";
}

const char * rover_mode_from_enum(ROVER_MODE mode) {
    switch (mode) {
       case ROVER_MODE_HOLD:
            return "HOLD";
       case ROVER_MODE_MANUAL:
            return "MANUAL";
       case ROVER_MODE_STEERING:
            return "STEER";
       case ROVER_MODE_INITIALIZING:
            return "INIT";
       case ROVER_MODE_SMART_RTL:
            return "SMRTL";
       case ROVER_MODE_ACRO:
            return "ACRO";
       case ROVER_MODE_AUTO:
            return "AUTO";
       case ROVER_MODE_RTL:
            return "RTL";
       case ROVER_MODE_LOITER:
            return "LOITER";
       case ROVER_MODE_GUIDED:
            return "GUIDED";
    }
    return "-----";
}

const char * chinese_copter_mode_from_enum(COPTER_MODE mode) {
    switch (mode) {
        case COPTER_MODE_STABILIZE: 
            return "自   稳"; 
        case COPTER_MODE_ACRO: 
            return "特   技"; 
        case COPTER_MODE_ALT_HOLD: 
            return "定   高"; 
        case COPTER_MODE_AUTO: 
            return "自   动"; 
        case COPTER_MODE_GUIDED: 
            return "指   引"; 
        case COPTER_MODE_LOITER: 
            return "悬   停"; 
        case COPTER_MODE_RTL: 
            return "返   航"; 
        case COPTER_MODE_CIRCLE: 
            return "绕   圈"; 
        case COPTER_MODE_LAND: 
            return "降   落"; 
        case COPTER_MODE_DRIFT: 
            return "漂   移"; 
        case COPTER_MODE_SPORT: 
            return "运   动"; 
        case COPTER_MODE_FLIP: 
            return "翻   滚"; 
        case COPTER_MODE_AUTOTUNE: 
            return "自 动 调 参"; 
        case COPTER_MODE_POSHOLD: 
            return "定   点"; 
        case COPTER_MODE_BRAKE: 
            return "制   动"; 
        case COPTER_MODE_THROW: 
            return "抛   飞"; 
        case COPTER_MODE_AVOID_ADSB: 
            return "避   障"; 
        case COPTER_MODE_GUIDED_NOGPS: 
            return "无 GPS 指 引"; 
        case COPTER_MODE_SMART_RTL:
            return "SMARTRTL";
    }
    return "-----";
}

const char * copter_mode_from_enum(COPTER_MODE mode) {
    switch (mode) {
       case COPTER_MODE_LAND:
            return "LAND";
       case COPTER_MODE_FLIP:
            return "FLIP";
       case COPTER_MODE_BRAKE:
            return "BRAKE";
       case COPTER_MODE_DRIFT:
            return "DRIFT";
       case COPTER_MODE_SPORT:
            return "SPORT";
       case COPTER_MODE_THROW:
            return "THROW";
       case COPTER_MODE_POSHOLD:
            return "POSHOLD";
       case COPTER_MODE_ALT_HOLD:
            return "ALTHOLD";
       case COPTER_MODE_SMART_RTL:
            return "SMRTL";
       case COPTER_MODE_GUIDED_NOGPS:
            return "GUIDEDNOGPS";
       case COPTER_MODE_CIRCLE:
            return "CIRCLE";
       case COPTER_MODE_STABILIZE:
            return "STAB";
       case COPTER_MODE_ACRO:
            return "ACRO";
       case COPTER_MODE_AUTOTUNE:
            return "AUTOTUNE";
       case COPTER_MODE_AUTO:
            return "AUTO";
       case COPTER_MODE_RTL:
            return "RTL";
       case COPTER_MODE_LOITER:
            return "LOITER";
       case COPTER_MODE_AVOID_ADSB:
            return "AVOIDADSB";
       case COPTER_MODE_GUIDED:
            return "GUIDED";
    }
    return "-----";
}

const char * chinese_plane_mode_from_enum(PLANE_MODE mode) {
    switch (mode) {
        case PLANE_MODE_MANUAL:
            return "手   动";
        case PLANE_MODE_CIRCLE:
            return "盘   旋";
        case PLANE_MODE_STABILIZE:
            return "自   稳";
        case PLANE_MODE_TRAINING:
            return "教   练";
        case PLANE_MODE_ACRO:
            return "特   技";
        case PLANE_MODE_FLY_BY_WIRE_A:
            return "自   稳";
        case PLANE_MODE_FLY_BY_WIRE_B:
            return "自 稳 定 高";
        case PLANE_MODE_CRUISE:
            return "巡   航";
        case PLANE_MODE_AUTOTUNE:
            return "自 动 调 参";
        case PLANE_MODE_AUTO:
            return "自   动";
        case PLANE_MODE_RTL:
            return "返   航";
        case PLANE_MODE_LOITER:
            return "定   点";
        case PLANE_MODE_TAKEOFF:
            return "TAKEOFF";
        case PLANE_MODE_AVOID_ADSB:
            return "AVOIDADSB";
        case PLANE_MODE_GUIDED:
            return "指   引";
        case PLANE_MODE_INITIALIZING:
            return "INIT";
        case PLANE_MODE_QSTABILIZE:
            return "QSTAB";
        case PLANE_MODE_QHOVER:
            return "QHOVER";
        case PLANE_MODE_QLOITER:
            return "QLOITER";
        case PLANE_MODE_QLAND:
            return "QLAND";
        case PLANE_MODE_QRTL:
            return "QRTL";
        case PLANE_MODE_QAUTOTUNE:
            return "QAUTOTUNE";
    }
    return "-----";
}

const char * plane_mode_from_enum(PLANE_MODE mode) {
    switch (mode) {
        case PLANE_MODE_MANUAL:
            return "MAN";
        case PLANE_MODE_CIRCLE:
            return "CIRCLE";
        case PLANE_MODE_STABILIZE:
            return "STAB";
        case PLANE_MODE_TRAINING:
            return "TRAIN";
        case PLANE_MODE_ACRO:
            return "ACRO";
        case PLANE_MODE_FLY_BY_WIRE_A:
            return "FBWA";
        case PLANE_MODE_FLY_BY_WIRE_B:
            return "FBWB";
        case PLANE_MODE_CRUISE:
            return "CRUISE";
        case PLANE_MODE_AUTOTUNE:
            return "AUTOTUNE";
        case PLANE_MODE_AUTO:
            return "AUTO";
        case PLANE_MODE_RTL:
            return "RTL";
        case PLANE_MODE_LOITER:
            return "LOITER";
        case PLANE_MODE_TAKEOFF:
            return "TAKEOFF";
        case PLANE_MODE_AVOID_ADSB:
            return "AVOIDADSB";
        case PLANE_MODE_GUIDED:
            return "GUIDED";
        case PLANE_MODE_INITIALIZING:
            return "INIT";
        case PLANE_MODE_QSTABILIZE:
            return "QSTAB";
        case PLANE_MODE_QHOVER:
            return "QHOVER";
        case PLANE_MODE_QLOITER:
            return "QLOITER";
        case PLANE_MODE_QLAND:
            return "QLAND";
        case PLANE_MODE_QRTL:
            return "QRTL";
        case PLANE_MODE_QAUTOTUNE:
            return "QAUTOTUNE";
    }
    return "-----";
}

const char * tracker_mode_from_enum(TRACKER_MODE mode) {
    switch (mode) {
       case TRACKER_MODE_MANUAL:
            return "MAN";
       case TRACKER_MODE_STOP:
            return "STOP";
       case TRACKER_MODE_SCAN:
            return "SCAN";
       case TRACKER_MODE_SERVO_TEST:
            return "SERVOTEST";
       case TRACKER_MODE_AUTO:
            return "AUTO";
       case TRACKER_MODE_INITIALIZING:
            return "INIT";
    }
    return "-----";
}

const char * vot_mode_from_telemetry(uint8_t mode) {
    switch (mode) {
        case 0:
            return "2D";
        case 1:
            return "2DAH";
        case 2:
            return "2DHH";
        case 3:
            return "2DAHH";
        case 4:
            return "LOITER";
        case 5:
            return "3D";
        case 6:
            return "3DHH";
        case 7:
            return "RTH";
        case 8:
            return "LAND";
        case 9:
            return "CART";
        case 10:
            return "CARTLOI";
        case 11:
            return "POLAR";
        case 12:
            return "POLARLOI";
        case 13:
            return "CENTERSTICK";
        case 14:
            return "OFF";
        case 15:
            return "WAYPOINT";
        case 16:
            return "MAX";
    }
    return "-----";
}


const char * chinese_ltm_mode_from_telem(int mode) {
    switch (mode) {
        case 0:
            return "手   动";
        case 1:
            return "RATE";
        case 2:
            return "自   稳";
        case 3:
            return "半 自 稳";
        case 4:
            return "特   技";
        case 5:
            return "自 稳 1";
        case 6:
            return "自 稳 2";
        case 7:
            return "自 稳 3";
        case 8:
            return "定   高";
        case 9:
            return "GPS 定点";
        case 10:
            return "自   动";
        case 11:
            return "无   头";
        case 12:
            return "绕   圈";
        case 13:
            return "返   航";
        case 14:
            return "跟   随";
        case 15:
            return "降   落";
        case 16:
            return "线 性 增 稳";
        case 17:
            return "增 稳 定 高";
        case 18:
            return "巡   航";
    }
    return "-----";
}


const char * ltm_mode_from_telem(int mode) {
    switch (mode) {
        case 0:
            return "MAN";
        case 1:
            return "RATE";
        case 2:
            return "ANGLE";
        case 3:
            return "HORIZON";
        case 4:
            return "ACRO";
        case 5:
            return "STAB1";
        case 6:
            return "STAB2";
        case 7:
            return "STAB3";
        case 8:
            return "ALTHOLD";
        case 9:
            return "GPSHOLD";
        case 10:
            return "WAY";
        case 11:
            return "HEADFREE";
        case 12:
            return "CIRCLE";
        case 13:
            return "RTH";
        case 14:
            return "FOLLOWME";
        case 15:
            return "LAND";
        case 16:
            return "FBWA";
        case 17:
            return "FBWB";
        case 18:
            return "CRUISE";
    }
    return "-----";
}

const char * px4_mode_from_custom_mode(int custom_mode) {
    union px4_custom_mode px4_mode;
    px4_mode.data = custom_mode;

    auto main_mode = px4_mode.main_mode;

    switch (main_mode) {
        case PX4_CUSTOM_MAIN_MODE_MANUAL: {
            return "Manual";
        }
        case PX4_CUSTOM_MAIN_MODE_ALTCTL: {
            return "Altitude Control";
        }
        case PX4_CUSTOM_MAIN_MODE_POSCTL: {
            switch (px4_mode.sub_mode) {
                case PX4_CUSTOM_SUB_MODE_POSCTL_POSCTL: {
                    return "Position Control";
                }
                case PX4_CUSTOM_SUB_MODE_POSCTL_ORBIT: {
                    return "Position Control Orbit";
                }
                default: {
                    break;
                }
            }

            break;
        }
        case PX4_CUSTOM_MAIN_MODE_AUTO: {
            switch (px4_mode.sub_mode) {
                case PX4_CUSTOM_SUB_MODE_AUTO_READY: {
                    return "Auto Ready";
                }
                case PX4_CUSTOM_SUB_MODE_AUTO_TAKEOFF: {
                    return "Takeoff";
                }
                case PX4_CUSTOM_SUB_MODE_AUTO_LOITER: {
                    return "Auto Loiter";
                }
                case PX4_CUSTOM_SUB_MODE_AUTO_MISSION: {
                    return "Auto Mission";
                }
                case PX4_CUSTOM_SUB_MODE_AUTO_RTL: {
                    return "Auto RTL";
                }
                case PX4_CUSTOM_SUB_MODE_AUTO_LAND: {
                    return "Auto Land";
                }
                case PX4_CUSTOM_SUB_MODE_AUTO_RESERVED_DO_NOT_USE: {
                    break;
                }
                case PX4_CUSTOM_SUB_MODE_AUTO_FOLLOW_TARGET: {
                    return "Auto Follow Tgt";
                }
                case PX4_CUSTOM_SUB_MODE_AUTO_PRECLAND: {
                    return "Auto Precision Land";
                }
                default: {
                    break;
                }
            }
            break;
        }
        case PX4_CUSTOM_MAIN_MODE_ACRO: {
            return "Acro";
        }
        case PX4_CUSTOM_MAIN_MODE_OFFBOARD: {
            return "Offboard";
        }
        case PX4_CUSTOM_MAIN_MODE_STABILIZED: {
            return "Stabilized";
        }
        case PX4_CUSTOM_MAIN_MODE_RATTITUDE: {
            return "Rattitude";
        }
        case PX4_CUSTOM_MAIN_MODE_SIMPLE: {
            return "Simple";
        }
        default: {
            break;
        }
    }
    return "-----";
}
