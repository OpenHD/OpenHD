/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_GST_BITRATE_CONTROLL_WRAPPER_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_GST_BITRATE_CONTROLL_WRAPPER_H_

#include <gst/gst.h>

#include <algorithm>
#include <optional>

#include "openhd_bitrate.h"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

// #define EXPERIMENTAL_USE_OPENH264_ENCODER

// Bitrate is one of the few params we want to support changing dynamically at
// run time without the need for a pipeline restart. This just wraps the
// differences for those pipelines nicely
struct GstBitrateControlElement {
  // Some elements take kbit/s, some take bit/s
  bool takes_kbit = false;
  // the encoder (or similar) element, must not be null
  GstElement* encoder;
  // Not all encoders / elements call the bitrate property "bitrate"
  std::string property_name = "bitrate";
  // For rockchip mpp encoders we also update min/max bounds with each bitrate
  // change to avoid stale clamps when the link adapts bitrate dynamically.
  bool update_rockchip_mpp_bounds = false;
};

struct GstBitrateReadback {
  int raw_property_value = -1;
  int interpreted_kbits = -1;
};

static std::optional<GstBitrateReadback> read_bitrate_readback(
    const GstBitrateControlElement& ctrl_el) {
  gint raw_property_value = -1;
  g_object_get(ctrl_el.encoder, ctrl_el.property_name.c_str(),
               &raw_property_value, NULL);
  if (raw_property_value < 0) {
    return std::nullopt;
  }
  GstBitrateReadback ret{};
  ret.raw_property_value = raw_property_value;
  ret.interpreted_kbits =
      ctrl_el.takes_kbit
          ? raw_property_value
          : openhd::bits_per_second_to_kbits_per_second(raw_property_value);
  return ret;
}

static std::optional<GstBitrateControlElement>
get_dynamic_bitrate_control_element_in_pipeline(
    GstElement* gst_pipeline, const CameraHolder& camera_holder) {
  auto camera = camera_holder.get_camera();
  auto settings = camera_holder.get_settings();
  GstBitrateControlElement ret{};
  ret.encoder = nullptr;
  if (camera.requires_rpi_mmal_pipeline()) {
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "rpicamsrc");
    ret.property_name = "bitrate";
    ret.takes_kbit = false;
  } else if (camera.camera_type == X_CAM_TYPE_DUMMY_SW ||
             is_usb_camera(camera.camera_type) || settings.force_sw_encode) {
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "swencoder");
    ret.property_name = "bitrate";
#ifdef EXPERIMENTAL_USE_OPENH264_ENCODER
    ret.takes_kbit = false;
#else
    ret.takes_kbit = true;
#endif
  } else if (camera.requires_x20_cedar_pipeline()) {
    // We can change bitrate dynamically
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "sunxisrc");
    ret.property_name = "bitrate";
    ret.takes_kbit = true;
  } else if (camera.requires_rockchip3_mpp_pipeline() ||
             camera.requires_rockchip5_mpp_pipeline()) {
    // Rockchip mpp encoders use bps (bit/s)
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "rk_mpp_encoder");
    ret.property_name = "bps";
    ret.takes_kbit = false;
    ret.update_rockchip_mpp_bounds = true;
  } else if (camera.requires_rockchip_rv_pipeline()) {
    // We can change bitrate dynamically
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "rkmpih264enc");
    ret.property_name = "bitrate";
    ret.takes_kbit = true;
  }
  if (ret.encoder == nullptr) {
    openhd::log::get_default()->debug(
        "Cannot find dynamic bitrate control element for camera {}",
        camera.cam_type_as_verbose_string());
    return std::nullopt;
  }
  // try fetching the value for testing if it actually works
  const auto readback_opt = read_bitrate_readback(ret);
  if (!readback_opt.has_value()) {
    openhd::log::get_default()->warn(
        "dynamic bitrate control element doesn't work");
    return std::nullopt;
  }
  const auto readback = readback_opt.value();
  const char* encoder_name = GST_OBJECT_NAME(ret.encoder);
  openhd::log::get_default()->info(
      "Got bitrate control for camera {} encoder:{} property:{} units:{} "
      "current_raw:{} current_kbits:{}",
      camera.cam_type_as_verbose_string(), encoder_name ? encoder_name : "n/a",
      ret.property_name, ret.takes_kbit ? "kbit/s" : "bit/s",
      readback.raw_property_value, readback.interpreted_kbits);
  return ret;
}

static bool change_bitrate(const GstBitrateControlElement& ctrl_el,
                           int bitrate_kbits) {
  const auto target_raw_property_value =
      ctrl_el.takes_kbit ? bitrate_kbits
                         : openhd::kbits_to_bits_per_second(bitrate_kbits);
  if (ctrl_el.update_rockchip_mpp_bounds) {
    static constexpr int RK_BPS_HARD_MIN = 500000;
    static constexpr int RK_BPS_HARD_MAX = 25000000;
    static constexpr int RK_BPS_MAX_PCT = 110;
    static constexpr int RK_BPS_MIN_PCT = 90;
    const int bps_target = std::clamp(target_raw_property_value, RK_BPS_HARD_MIN,
                                      RK_BPS_HARD_MAX);
    int bps_min = std::max((bps_target * RK_BPS_MIN_PCT) / 100, RK_BPS_HARD_MIN);
    int bps_max = std::min((bps_target * RK_BPS_MAX_PCT) / 100, RK_BPS_HARD_MAX);
    if (bps_min > bps_max) {
      bps_min = bps_max;
    }
    g_object_set(ctrl_el.encoder, "bps", bps_target, "bps-min", bps_min,
                 "bps-max", bps_max, NULL);
  } else {
    g_object_set(ctrl_el.encoder, ctrl_el.property_name.c_str(),
                 target_raw_property_value, NULL);
  }
  const auto readback_opt = read_bitrate_readback(ctrl_el);
  if (!readback_opt.has_value()) {
    openhd::log::get_default()->warn(
        "Cannot read bitrate property {} after bitrate set to {} kbit/s",
        ctrl_el.property_name, bitrate_kbits);
    return false;
  }
  const auto readback = readback_opt.value();
  if (readback.raw_property_value != target_raw_property_value) {
    openhd::log::get_default()->warn(
        "Cannot change bitrate: target_kbits:{} target_raw:{} property:{} "
        "units:{} readback_raw:{} readback_kbits:{}",
        bitrate_kbits, target_raw_property_value, ctrl_el.property_name,
        ctrl_el.takes_kbit ? "kbit/s" : "bit/s", readback.raw_property_value,
        readback.interpreted_kbits);
    return false;
  }
  openhd::log::get_default()->debug(
      "Changed bitrate to {} kbit/s (property:{} raw:{})", bitrate_kbits,
      ctrl_el.property_name, target_raw_property_value);
  return true;
}

static void unref_bitrate_element(GstBitrateControlElement& element) {
  if (element.encoder) {
    openhd::log::get_default()->debug("Unref bitrate control element begin");
    gst_object_unref(element.encoder);
    element.encoder = nullptr;
    openhd::log::get_default()->debug("Unref bitrate control element end");
  }
}

static std::chrono::nanoseconds convert_ts(uint64_t dts) {
  return std::chrono::nanoseconds(dts);
}
static std::chrono::nanoseconds calculate_delta(uint64_t dts) {
  return convert_ts(
      std::chrono::steady_clock::now().time_since_epoch().count() - dts);
}

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_GST_BITRATE_CONTROLL_WRAPPER_H_
