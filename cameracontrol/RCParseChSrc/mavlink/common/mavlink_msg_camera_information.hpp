// MESSAGE CAMERA_INFORMATION support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief CAMERA_INFORMATION message
 *
 * Information about a camera
 */
struct CAMERA_INFORMATION : mavlink::Message {
    static constexpr msgid_t MSG_ID = 259;
    static constexpr size_t LENGTH = 235;
    static constexpr size_t MIN_LENGTH = 235;
    static constexpr uint8_t CRC_EXTRA = 92;
    static constexpr auto NAME = "CAMERA_INFORMATION";


    uint32_t time_boot_ms; /*< [ms] Timestamp (time since system boot). */
    std::array<uint8_t, 32> vendor_name; /*<  Name of the camera vendor */
    std::array<uint8_t, 32> model_name; /*<  Name of the camera model */
    uint32_t firmware_version; /*<  Version of the camera firmware (v << 24 & 0xff = Dev, v << 16 & 0xff = Patch, v << 8 & 0xff = Minor, v & 0xff = Major) */
    float focal_length; /*< [mm] Focal length */
    float sensor_size_h; /*< [mm] Image sensor size horizontal */
    float sensor_size_v; /*< [mm] Image sensor size vertical */
    uint16_t resolution_h; /*< [pix] Horizontal image resolution */
    uint16_t resolution_v; /*< [pix] Vertical image resolution */
    uint8_t lens_id; /*<  Reserved for a lens ID */
    uint32_t flags; /*<  Bitmap of camera capability flags. */
    uint16_t cam_definition_version; /*<  Camera definition version (iteration) */
    std::array<char, 140> cam_definition_uri; /*<  Camera definition URI (if any, otherwise only basic functions will be available). */


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
        ss << "  time_boot_ms: " << time_boot_ms << std::endl;
        ss << "  vendor_name: [" << to_string(vendor_name) << "]" << std::endl;
        ss << "  model_name: [" << to_string(model_name) << "]" << std::endl;
        ss << "  firmware_version: " << firmware_version << std::endl;
        ss << "  focal_length: " << focal_length << std::endl;
        ss << "  sensor_size_h: " << sensor_size_h << std::endl;
        ss << "  sensor_size_v: " << sensor_size_v << std::endl;
        ss << "  resolution_h: " << resolution_h << std::endl;
        ss << "  resolution_v: " << resolution_v << std::endl;
        ss << "  lens_id: " << +lens_id << std::endl;
        ss << "  flags: " << flags << std::endl;
        ss << "  cam_definition_version: " << cam_definition_version << std::endl;
        ss << "  cam_definition_uri: \"" << to_string(cam_definition_uri) << "\"" << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_boot_ms;                  // offset: 0
        map << firmware_version;              // offset: 4
        map << focal_length;                  // offset: 8
        map << sensor_size_h;                 // offset: 12
        map << sensor_size_v;                 // offset: 16
        map << flags;                         // offset: 20
        map << resolution_h;                  // offset: 24
        map << resolution_v;                  // offset: 26
        map << cam_definition_version;        // offset: 28
        map << vendor_name;                   // offset: 30
        map << model_name;                    // offset: 62
        map << lens_id;                       // offset: 94
        map << cam_definition_uri;            // offset: 95
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_boot_ms;                  // offset: 0
        map >> firmware_version;              // offset: 4
        map >> focal_length;                  // offset: 8
        map >> sensor_size_h;                 // offset: 12
        map >> sensor_size_v;                 // offset: 16
        map >> flags;                         // offset: 20
        map >> resolution_h;                  // offset: 24
        map >> resolution_v;                  // offset: 26
        map >> cam_definition_version;        // offset: 28
        map >> vendor_name;                   // offset: 30
        map >> model_name;                    // offset: 62
        map >> lens_id;                       // offset: 94
        map >> cam_definition_uri;            // offset: 95
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
