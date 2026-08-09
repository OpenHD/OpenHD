/******************************************************************************
 * Minimal, versioned C ABI shared by OpenHD and independently-built plugins.
 * Keep this header C-compatible: plugins must not depend on OpenHD C++ types.
 ******************************************************************************/

#ifndef OPENHD_PLUGIN_H
#define OPENHD_PLUGIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPENHD_PLUGIN_ABI_VERSION 1U
#define OPENHD_PLUGIN_ENTRYPOINT "openhd_plugin_get_descriptor"

enum openhd_plugin_role {
  OPENHD_PLUGIN_ROLE_AIR = 1U << 0U,
  OPENHD_PLUGIN_ROLE_GROUND = 1U << 1U,
};

enum openhd_plugin_log_level {
  OPENHD_PLUGIN_LOG_DEBUG = 0,
  OPENHD_PLUGIN_LOG_INFO = 1,
  OPENHD_PLUGIN_LOG_WARN = 2,
  OPENHD_PLUGIN_LOG_ERROR = 3,
};

enum openhd_plugin_video_codec {
  OPENHD_PLUGIN_VIDEO_H264 = 0,
  OPENHD_PLUGIN_VIDEO_H265 = 1,
};

struct openhd_plugin_host {
  uint32_t struct_size;
  uint32_t abi_version;
  void (*log)(int32_t level, const char* plugin_name, const char* message);
};

struct openhd_plugin_context {
  uint32_t struct_size;
  uint32_t role;
};

struct openhd_plugin_video_bitrate_event {
  uint32_t struct_size;
  uint32_t camera_index;
  int32_t camera_type;
  int32_t bitrate_kbits;
  int32_t codec;
  uint16_t width;
  uint16_t height;
  const char* ip_address;
};

/* Complete camera stream configuration. Added as an ABI-v1 extension; use
 * struct_size before accessing fields so older hosts/plugins remain valid. */
struct openhd_plugin_video_settings_event {
  uint32_t struct_size;
  uint32_t camera_index;
  int32_t camera_type;
  int32_t bitrate_kbits;
  int32_t codec;
  uint16_t width;
  uint16_t height;
  uint16_t framerate;
  const char* ip_address;
};

/* Generic camera/gimbal controls. These are intentionally expressed without
 * MAVLink types so independently-built plugins do not depend on OpenHD's
 * generated MAVLink headers. Values use the units documented per action. */
enum openhd_plugin_camera_control_action {
  OPENHD_PLUGIN_GIMBAL_RATE = 0,       /* value1=pitch, value2=yaw; -1..1 */
  OPENHD_PLUGIN_GIMBAL_ANGLE = 1,      /* value1=pitch, value2=yaw; degrees */
  OPENHD_PLUGIN_GIMBAL_CENTER = 2,
  OPENHD_PLUGIN_GIMBAL_MODE = 3,
  /* value1: enum openhd_plugin_gimbal_mode */
  OPENHD_PLUGIN_CAMERA_ZOOM_RATE = 4,  /* value1: -1, 0, or 1 */
  OPENHD_PLUGIN_CAMERA_ZOOM_ABSOLUTE = 5, /* value1: zoom multiple */
  OPENHD_PLUGIN_CAMERA_FOCUS_RATE = 6, /* value1: -1, 0, or 1 */
  OPENHD_PLUGIN_CAMERA_AUTO_FOCUS = 7,
  OPENHD_PLUGIN_CAMERA_TAKE_PHOTO = 8,
  OPENHD_PLUGIN_CAMERA_RECORD_START = 9,
  OPENHD_PLUGIN_CAMERA_RECORD_STOP = 10,
  OPENHD_PLUGIN_CAMERA_IMAGE_TYPE = 11, /* value1: vendor image type */
  OPENHD_PLUGIN_CAMERA_THERMAL_PALETTE = 12,
  /* value1: vendor palette */
  OPENHD_PLUGIN_CAMERA_ZOOM_PERCENT = 13, /* value1: 0..100 */
};

enum openhd_plugin_gimbal_mode {
  OPENHD_PLUGIN_GIMBAL_MODE_LOCK = 0,
  OPENHD_PLUGIN_GIMBAL_MODE_FOLLOW = 1,
  OPENHD_PLUGIN_GIMBAL_MODE_FPV = 2,
};

enum openhd_plugin_camera_control_flags {
  OPENHD_PLUGIN_CAMERA_CONTROL_YAW_LOCK = 1U << 0U,
};

struct openhd_plugin_camera_control_event {
  uint32_t struct_size;
  uint32_t camera_index;
  int32_t action;
  float value1;
  float value2;
  uint32_t flags;
};

struct openhd_plugin_descriptor {
  uint32_t struct_size;
  uint32_t abi_version;
  uint32_t supported_roles;
  const char* name;
  const char* version;
  int32_t (*init)(const struct openhd_plugin_host* host,
                  const struct openhd_plugin_context* context);
  void (*shutdown)(void);
  void (*on_video_bitrate_changed)(
      const struct openhd_plugin_video_bitrate_event* event);
  void (*on_video_settings_changed)(
      const struct openhd_plugin_video_settings_event* event);
  /* Return 0 when accepted, a positive value when unsupported, and a negative
   * value when the command could not be sent. Added as an ABI-v1 extension. */
  int32_t (*on_camera_control)(
      const struct openhd_plugin_camera_control_event* event);
};

typedef const struct openhd_plugin_descriptor* (*openhd_plugin_get_descriptor_fn)(
    void);

#ifdef __cplusplus
}
#endif

#endif  // OPENHD_PLUGIN_H
