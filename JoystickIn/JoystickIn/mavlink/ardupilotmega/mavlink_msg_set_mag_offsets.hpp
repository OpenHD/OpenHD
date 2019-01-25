// MESSAGE SET_MAG_OFFSETS support class

#pragma once

namespace mavlink {
namespace ardupilotmega {
namespace msg {

/**
 * @brief SET_MAG_OFFSETS message
 *
 * Set the magnetometer offsets
 */
struct SET_MAG_OFFSETS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 151;
    static constexpr size_t LENGTH = 8;
    static constexpr size_t MIN_LENGTH = 8;
    static constexpr uint8_t CRC_EXTRA = 219;
    static constexpr auto NAME = "SET_MAG_OFFSETS";


    uint8_t target_system; /*<  System ID. */
    uint8_t target_component; /*<  Component ID. */
    int16_t mag_ofs_x; /*<  Magnetometer X offset. */
    int16_t mag_ofs_y; /*<  Magnetometer Y offset. */
    int16_t mag_ofs_z; /*<  Magnetometer Z offset. */


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
        ss << "  target_system: " << +target_system << std::endl;
        ss << "  target_component: " << +target_component << std::endl;
        ss << "  mag_ofs_x: " << mag_ofs_x << std::endl;
        ss << "  mag_ofs_y: " << mag_ofs_y << std::endl;
        ss << "  mag_ofs_z: " << mag_ofs_z << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << mag_ofs_x;                     // offset: 0
        map << mag_ofs_y;                     // offset: 2
        map << mag_ofs_z;                     // offset: 4
        map << target_system;                 // offset: 6
        map << target_component;              // offset: 7
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> mag_ofs_x;                     // offset: 0
        map >> mag_ofs_y;                     // offset: 2
        map >> mag_ofs_z;                     // offset: 4
        map >> target_system;                 // offset: 6
        map >> target_component;              // offset: 7
    }
};

} // namespace msg
} // namespace ardupilotmega
} // namespace mavlink
