// MESSAGE ICAROUS_KINEMATIC_BANDS support class

#pragma once

namespace mavlink {
namespace icarous {
namespace msg {

/**
 * @brief ICAROUS_KINEMATIC_BANDS message
 *
 * Kinematic multi bands (track) output from Daidalus
 */
struct ICAROUS_KINEMATIC_BANDS : mavlink::Message {
    static constexpr msgid_t MSG_ID = 42001;
    static constexpr size_t LENGTH = 46;
    static constexpr size_t MIN_LENGTH = 46;
    static constexpr uint8_t CRC_EXTRA = 239;
    static constexpr auto NAME = "ICAROUS_KINEMATIC_BANDS";


    int8_t numBands; /*<  Number of track bands */
    uint8_t type1; /*<  See the TRACK_BAND_TYPES enum. */
    float min1; /*< [deg] min angle (degrees) */
    float max1; /*< [deg] max angle (degrees) */
    uint8_t type2; /*<  See the TRACK_BAND_TYPES enum. */
    float min2; /*< [deg] min angle (degrees) */
    float max2; /*< [deg] max angle (degrees) */
    uint8_t type3; /*<  See the TRACK_BAND_TYPES enum. */
    float min3; /*< [deg] min angle (degrees) */
    float max3; /*< [deg] max angle (degrees) */
    uint8_t type4; /*<  See the TRACK_BAND_TYPES enum. */
    float min4; /*< [deg] min angle (degrees) */
    float max4; /*< [deg] max angle (degrees) */
    uint8_t type5; /*<  See the TRACK_BAND_TYPES enum. */
    float min5; /*< [deg] min angle (degrees) */
    float max5; /*< [deg] max angle (degrees) */


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
        ss << "  numBands: " << +numBands << std::endl;
        ss << "  type1: " << +type1 << std::endl;
        ss << "  min1: " << min1 << std::endl;
        ss << "  max1: " << max1 << std::endl;
        ss << "  type2: " << +type2 << std::endl;
        ss << "  min2: " << min2 << std::endl;
        ss << "  max2: " << max2 << std::endl;
        ss << "  type3: " << +type3 << std::endl;
        ss << "  min3: " << min3 << std::endl;
        ss << "  max3: " << max3 << std::endl;
        ss << "  type4: " << +type4 << std::endl;
        ss << "  min4: " << min4 << std::endl;
        ss << "  max4: " << max4 << std::endl;
        ss << "  type5: " << +type5 << std::endl;
        ss << "  min5: " << min5 << std::endl;
        ss << "  max5: " << max5 << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << min1;                          // offset: 0
        map << max1;                          // offset: 4
        map << min2;                          // offset: 8
        map << max2;                          // offset: 12
        map << min3;                          // offset: 16
        map << max3;                          // offset: 20
        map << min4;                          // offset: 24
        map << max4;                          // offset: 28
        map << min5;                          // offset: 32
        map << max5;                          // offset: 36
        map << numBands;                      // offset: 40
        map << type1;                         // offset: 41
        map << type2;                         // offset: 42
        map << type3;                         // offset: 43
        map << type4;                         // offset: 44
        map << type5;                         // offset: 45
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> min1;                          // offset: 0
        map >> max1;                          // offset: 4
        map >> min2;                          // offset: 8
        map >> max2;                          // offset: 12
        map >> min3;                          // offset: 16
        map >> max3;                          // offset: 20
        map >> min4;                          // offset: 24
        map >> max4;                          // offset: 28
        map >> min5;                          // offset: 32
        map >> max5;                          // offset: 36
        map >> numBands;                      // offset: 40
        map >> type1;                         // offset: 41
        map >> type2;                         // offset: 42
        map >> type3;                         // offset: 43
        map >> type4;                         // offset: 44
        map >> type5;                         // offset: 45
    }
};

} // namespace msg
} // namespace icarous
} // namespace mavlink
