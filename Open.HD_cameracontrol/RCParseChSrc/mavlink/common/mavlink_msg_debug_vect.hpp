// MESSAGE DEBUG_VECT support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief DEBUG_VECT message
 *
 * To debug something using a named 3D vector.
 */
struct DEBUG_VECT : mavlink::Message {
    static constexpr msgid_t MSG_ID = 250;
    static constexpr size_t LENGTH = 30;
    static constexpr size_t MIN_LENGTH = 30;
    static constexpr uint8_t CRC_EXTRA = 49;
    static constexpr auto NAME = "DEBUG_VECT";


    std::array<char, 10> name; /*<  Name */
    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot). The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude the number. */
    float x; /*<  x */
    float y; /*<  y */
    float z; /*<  z */


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
        ss << "  name: \"" << to_string(name) << "\"" << std::endl;
        ss << "  time_usec: " << time_usec << std::endl;
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << x;                             // offset: 8
        map << y;                             // offset: 12
        map << z;                             // offset: 16
        map << name;                          // offset: 20
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> x;                             // offset: 8
        map >> y;                             // offset: 12
        map >> z;                             // offset: 16
        map >> name;                          // offset: 20
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
