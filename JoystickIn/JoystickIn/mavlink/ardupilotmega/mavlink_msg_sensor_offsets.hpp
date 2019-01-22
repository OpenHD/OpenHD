// MESSAGE SENSOR_OFFSETS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief SENSOR_OFFSETS message
 *
 * Offsets and calibrations values for hardware sensors. This makes it easier to debug the calibration process.
 */
struct SENSOR_OFFSETS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 150;
    static constexpr size_t LENGTH = 42;
    static constexpr size_t MIN_LENGTH = 42;
    static constexpr uint8_t CRC_EXTRA = 134;
    static constexpr auto NAME = "SENSOR_OFFSETS";


    int16_t mag_ofs_x; /*<  Magnetometer X offset. */
    int16_t mag_ofs_y; /*<  Magnetometer Y offset. */
    int16_t mag_ofs_z; /*<  Magnetometer Z offset. */
    float mag_declination; /*< [rad] Magnetic declination. */
    int32_t raw_press; /*<  Raw pressure from barometer. */
    int32_t raw_temp; /*<  Raw temperature from barometer. */
    float gyro_cal_x; /*<  Gyro X calibration. */
    float gyro_cal_y; /*<  Gyro Y calibration. */
    float gyro_cal_z; /*<  Gyro Z calibration. */
    float accel_cal_x; /*<  Accel X calibration. */
    float accel_cal_y; /*<  Accel Y calibration. */
    float accel_cal_z; /*<  Accel Z calibration. */


    inline std::string get_name(void) const override
    {
            return NAME;
    }

    inline Info get_message_info(void) const override
    {
            return { MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA };
    }

    inline std::string to_yaml(void) const override
    {
        std::stringstream ss;

        ss << NAME << ":" << std::endl;
        ss << "  mag_ofs_x: " << mag_ofs_x << std::endl;
        ss << "  mag_ofs_y: " << mag_ofs_y << std::endl;
        ss << "  mag_ofs_z: " << mag_ofs_z << std::endl;
        ss << "  mag_declination: " << mag_declination << std::endl;
        ss << "  raw_press: " << raw_press << std::endl;
        ss << "  raw_temp: " << raw_temp << std::endl;
        ss << "  gyro_cal_x: " << gyro_cal_x << std::endl;
        ss << "  gyro_cal_y: " << gyro_cal_y << std::endl;
        ss << "  gyro_cal_z: " << gyro_cal_z << std::endl;
        ss << "  accel_cal_x: " << accel_cal_x << std::endl;
        ss << "  accel_cal_y: " << accel_cal_y << std::endl;
        ss << "  accel_cal_z: " << accel_cal_z << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << mag_declination;               // offset: 0
        map << raw_press;                     // offset: 4
        map << raw_temp;                      // offset: 8
        map << gyro_cal_x;                    // offset: 12
        map << gyro_cal_y;                    // offset: 16
        map << gyro_cal_z;                    // offset: 20
        map << accel_cal_x;                   // offset: 24
        map << accel_cal_y;                   // offset: 28
        map << accel_cal_z;                   // offset: 32
        map << mag_ofs_x;                     // offset: 36
        map << mag_ofs_y;                     // offset: 38
        map << mag_ofs_z;                     // offset: 40
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> mag_declination;               // offset: 0
        map >> raw_press;                     // offset: 4
        map >> raw_temp;                      // offset: 8
        map >> gyro_cal_x;                    // offset: 12
        map >> gyro_cal_y;                    // offset: 16
        map >> gyro_cal_z;                    // offset: 20
        map >> accel_cal_x;                   // offset: 24
        map >> accel_cal_y;                   // offset: 28
        map >> accel_cal_z;                   // offset: 32
        map >> mag_ofs_x;                     // offset: 36
        map >> mag_ofs_y;                     // offset: 38
        map >> mag_ofs_z;                     // offset: 40
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
