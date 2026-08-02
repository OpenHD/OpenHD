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
};

typedef const struct openhd_plugin_descriptor* (*openhd_plugin_get_descriptor_fn)(
    void);

#ifdef __cplusplus
}
#endif

#endif  // OPENHD_PLUGIN_H
