//
// Created by consti10 on 20.07.22.
//

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_

#include <optional>
#include <string>

// For now, only basic sanity checking on video settings
namespace openhd {

// We used to have some basic "sane user inputs" validation, but there were a
// couple of issues with that 1) Only for USB (UVC) cameras - sometimes, they
// only work with width, height or (especially) framerate omitted on the gst
// pipeline 2) Thermal cameras as an example can have ultra low resolutions /
// framerates, e.g. 110 pixels wide Therefore, we allow values >=0 for width
// height and fps, and when they are set to 0, the (gst) argument should be
// omitted. Old stuff left for reference max: 3840×2160 (4K) min: 320x240
static bool validate_video_with(int video_w) {
  // return video_w >= 320 && video_w<=3840;
  return video_w >= 0;
}
static bool validate_video_height(int video_h) {
  // return video_h >= 240 && video_h<=2160;
  return video_h >= 0;
}
// min: 1 fps max: 240 fps
static bool validate_video_fps(int fps) {
  // return fps>=1 && fps <= 240;
  return fps >= 0;
}

static bool is_resolution_auto(int video_w, int video_h, int fps) {
  if (video_w == 0 && video_h == 0 && fps == 0) {
    return true;
  }
  return false;
}

static bool validate_video_width_height_fps(int video_w, int video_h, int fps) {
  if (is_resolution_auto(video_w, video_h, fps)) return true;
  return validate_video_with(video_w) && validate_video_height(video_h) &&
         validate_video_fps(fps);
}

// 0 or 1 (h264 or h265)
static bool validate_video_codec(int codec) { return codec == 0 || codec == 1; }

bool validate_bitrate_mbits(int bitrate_mbits);

bool validate_camera_rotation(int value);

static bool validate_openhd_brightness(int value) {
  return value >= 0 && value <= 200;
}
static bool validate_openhd_saturation(int value) {
  return value >= 0 && value <= 200;
}
static bool validate_openhd_contrast(int value) {
  return value >= 0 && value <= 200;
}
static bool validate_openhd_sharpness(int value) {
  return value >= 0 && value <= 200;
}

// from gst-rpicamsrc: keyframe-interval   : Interval (in frames) between I
// frames. -1 = automatic, 0 = single-keyframe
bool validate_rpi_keyframe_interval(int value);

// see gst-rpicamsrc documentation
bool validate_rpi_intra_refresh_type(int value);

static bool validate_rpi_libcamera_ev_value(int value) {
  return value >= -10 && value <= 10;
}
static bool validate_rpi_libcamera_doenise_index(int value) {
  return value >= 0 && value <= 4;
}
static bool validate_rpi_libcamera_awb_index(int value) {
  return value >= 0 && value <= 7;
}
// (centre, spot, average, custom)
static bool validate_rpi_libcamera_metering_index(int value) {
  return value >= 0 && value <= 2;  // metering mode 3 (custom) crashes
                                    // libcamera
}
// (normal, sport)
static bool validate_rpi_libcamera_exposure_index(int value) {
  return value >= 0 && value <= 1;
}
// cannot really be validated, depends on camera
static bool validate_rpi_libcamera_shutter_microseconds(int value) {
  return value >= 0 && value <= 1000 * 100;
}

std::string video_format_from_int_values(int width, int height, int framerate);

}  // namespace openhd
#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_V_VALIDATE_SETTINGS_H_
