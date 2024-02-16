//
// Created by consti10 on 13.02.24.
//

#ifndef OPENHD_LIBCAMERA_IQ_HELPER_H
#define OPENHD_LIBCAMERA_IQ_HELPER_H

#include "camera_settings.hpp"
#include "openhd_util.h"

// Maps openhd camera settings IQ value(s) to libcamera IQ values
// - openhd strictly doesn't use floats, but easier to understand values
// instead. or invalid.
namespace openhd::libcamera {

// Specify a fixed brightness parameter. Positive values (up to 1.0) produce
// brighter images; negative values (up to -1.0) produce darker images and 0.0
// leaves pixels unchanged.
static std::optional<float> get_brightness(const CameraSettings& settings) {
  if (!openhd::validate_openhd_brightness(settings.openhd_brightness))
    return std::nullopt;
  if (settings.openhd_brightness == OPENHD_BRIGHTNESS_DEFAULT)
    return std::nullopt;
  float brightness_minus1_to_1 =
      OHDUtil::map_int_percentage_0_200_to_minus1_to_1(
          settings.openhd_brightness);
  return brightness_minus1_to_1;
}

static float remap_libcamera_openhd_int_to_libcamera_float(int value) {
  auto tmp = static_cast<float>(value);
  return tmp / 100.0f;
}

// Specify a fixed contrast parameter. Normal contrast is given by the
// value 1.0; larger values produce images with more contrast.
static std::optional<float> get_contrast(const CameraSettings& settings) {
  if (!openhd::validate_openhd_contrast(settings.openhd_contrast))
    return std::nullopt;
  if (settings.openhd_contrast == OPENHD_CONTRAST_DEFAULT) return std::nullopt;
  return remap_libcamera_openhd_int_to_libcamera_float(
      settings.openhd_contrast);
}

// Specify a fixed saturation parameter. Normal saturation is given by the
// value 1.0; larger values produce more saturated colours; 0.0 produces a
// greyscale image.
static std::optional<float> get_saturation(const CameraSettings& settings) {
  if (!openhd::validate_openhd_saturation(settings.openhd_saturation))
    return std::nullopt;
  if (settings.openhd_saturation == OPENHD_SATURATION_DEFAULT)
    return std::nullopt;
  return remap_libcamera_openhd_int_to_libcamera_float(
      settings.openhd_saturation);
}

// A value of 0.0 means no sharpening. The minimum value means minimal
// sharpening, and shall be 0.0 unless the camera can't disable sharpening
// completely. The default value shall give a "reasonable" level of sharpening,
// suitable for most use cases. The maximum value may apply extremely high
// levels of sharpening, higher than anyone could reasonably want. Negative
// values are not allowed. Note also that sharpening is not applied to raw
// streams.
static std::optional<float> get_sharpness(const CameraSettings& settings) {
  if (!openhd::validate_openhd_sharpness(settings.openhd_sharpness))
    return std::nullopt;
  if (settings.openhd_sharpness == OPENHD_SHARPNESS_DEFAULT)
    return std::nullopt;
  return remap_libcamera_openhd_int_to_libcamera_float(
      settings.openhd_sharpness);
}

static std::optional<int> get_rotation_degree(const CameraSettings& settings) {
  // Identity 0° is default
  if (settings.camera_rotation_degree == 0) return std::nullopt;
  if (settings.camera_rotation_degree == 180) return 180;
  // anything other than 0 and 180° are not supported
  return std::nullopt;
}
}  // namespace openhd::libcamera

#endif  // OPENHD_LIBCAMERA_IQ_HELPER_H
