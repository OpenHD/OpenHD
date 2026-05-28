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
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "camera_holder.h"
#include "openhd_bitrate.h"
#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

// #define EXPERIMENTAL_USE_OPENH264_ENCODER

// Bitrate is one of the few params we want to support changing dynamically at
// run time without the need for a pipeline restart. This just wraps the
// differences for those pipelines nicely
struct GstBitrateControlElement {
  // Some elements take kbit/s, some take bit/s
  bool takes_kbit = false;
  bool uses_extra_controls_video_bitrate = false;
  // the encoder (or similar) element, must not be null
  GstElement* encoder;
  // Not all encoders / elements call the bitrate property "bitrate"
  std::string property_name = "bitrate";
  // Some encoders need related rate-control bounds updated with the target.
  // The percentage is relative to the raw property value, not kbit/s.
  std::vector<std::pair<std::string, int>> extra_properties_percent;
  int64_t max_raw_property_value = 0;
};

struct GstBitrateReadback {
  int raw_property_value = -1;
  int interpreted_kbits = -1;
};

struct GstQpControlElement {
  // the encoder (or similar) element, must not be null
  GstElement* encoder;
  std::string min_property_name = "qp-min";
  std::string max_property_name = "qp-max";
  std::string extra_controls_property_name = "extra-controls";
  bool uses_extra_controls_h264_qp = false;
};

struct GstQpReadback {
  int qp_min = -1;
  int qp_max = -1;
};

static std::optional<int64_t> read_integer_property(GObject* object,
                                                    const std::string& name) {
  auto* pspec =
      g_object_class_find_property(G_OBJECT_GET_CLASS(object), name.c_str());
  if (pspec == nullptr) {
    openhd::log::get_default()->warn("Cannot find gst property {}", name);
    return std::nullopt;
  }
  GValue value = G_VALUE_INIT;
  g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
  g_object_get_property(object, name.c_str(), &value);
  std::optional<int64_t> ret = std::nullopt;
  if (G_VALUE_HOLDS_INT(&value)) {
    ret = g_value_get_int(&value);
  } else if (G_VALUE_HOLDS_UINT(&value)) {
    ret = static_cast<int64_t>(g_value_get_uint(&value));
  } else if (G_VALUE_HOLDS_LONG(&value)) {
    ret = static_cast<int64_t>(g_value_get_long(&value));
  } else if (G_VALUE_HOLDS_ULONG(&value)) {
    const auto raw = g_value_get_ulong(&value);
    ret = raw > static_cast<gulong>(std::numeric_limits<int64_t>::max())
              ? std::numeric_limits<int64_t>::max()
              : static_cast<int64_t>(raw);
  } else if (G_VALUE_HOLDS_INT64(&value)) {
    ret = g_value_get_int64(&value);
  } else if (G_VALUE_HOLDS_UINT64(&value)) {
    const auto raw = g_value_get_uint64(&value);
    ret = raw > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())
              ? std::numeric_limits<int64_t>::max()
              : static_cast<int64_t>(raw);
  } else {
    openhd::log::get_default()->warn("gst property {} is not integer-like",
                                     name);
  }
  g_value_unset(&value);
  return ret;
}

static bool set_integer_property(GObject* object, const std::string& name,
                                 int64_t raw_value) {
  auto* pspec =
      g_object_class_find_property(G_OBJECT_GET_CLASS(object), name.c_str());
  if (pspec == nullptr) {
    openhd::log::get_default()->warn("Cannot find gst property {}", name);
    return false;
  }
  GValue value = G_VALUE_INIT;
  g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
  if (G_VALUE_HOLDS_INT(&value)) {
    g_value_set_int(&value, static_cast<gint>(std::clamp<int64_t>(
                                raw_value, std::numeric_limits<gint>::min(),
                                std::numeric_limits<gint>::max())));
  } else if (G_VALUE_HOLDS_UINT(&value)) {
    const auto unsigned_value =
        static_cast<uint64_t>(std::max<int64_t>(raw_value, 0));
    g_value_set_uint(&value,
                     static_cast<guint>(std::min<uint64_t>(
                         unsigned_value, std::numeric_limits<guint>::max())));
  } else if (G_VALUE_HOLDS_LONG(&value)) {
    g_value_set_long(&value, static_cast<glong>(std::clamp<int64_t>(
                                 raw_value, std::numeric_limits<glong>::min(),
                                 std::numeric_limits<glong>::max())));
  } else if (G_VALUE_HOLDS_ULONG(&value)) {
    const auto unsigned_value =
        static_cast<uint64_t>(std::max<int64_t>(raw_value, 0));
    g_value_set_ulong(&value,
                      static_cast<gulong>(std::min<uint64_t>(
                          unsigned_value, std::numeric_limits<gulong>::max())));
  } else if (G_VALUE_HOLDS_INT64(&value)) {
    g_value_set_int64(&value, raw_value);
  } else if (G_VALUE_HOLDS_UINT64(&value)) {
    g_value_set_uint64(&value,
                       static_cast<guint64>(std::max<int64_t>(raw_value, 0)));
  } else {
    openhd::log::get_default()->warn("gst property {} is not integer-like",
                                     name);
    g_value_unset(&value);
    return false;
  }
  g_object_set_property(object, name.c_str(), &value);
  g_value_unset(&value);
  return true;
}

static bool set_extra_controls_ints(
    GstElement* encoder, const std::string& property_name,
    const std::vector<std::pair<std::string, int>>& controls) {
  GstStructure* extra_controls = nullptr;
  g_object_get(encoder, property_name.c_str(), &extra_controls, NULL);
  if (extra_controls == nullptr) {
    extra_controls = gst_structure_new_empty("controls");
  }
  for (const auto& control : controls) {
    gst_structure_set(extra_controls, control.first.c_str(), G_TYPE_INT,
                      control.second, NULL);
  }
  g_object_set(encoder, property_name.c_str(), extra_controls, NULL);
  gst_structure_free(extra_controls);
  return true;
}

static std::optional<GstBitrateReadback> read_bitrate_readback(
    const GstBitrateControlElement& ctrl_el) {
  if (ctrl_el.uses_extra_controls_video_bitrate) {
    GstStructure* extra_controls = nullptr;
    g_object_get(ctrl_el.encoder, ctrl_el.property_name.c_str(),
                 &extra_controls, NULL);
    if (extra_controls == nullptr) {
      return std::nullopt;
    }
    gint raw_property_value = -1;
    const bool ok = gst_structure_get_int(extra_controls, "video_bitrate",
                                          &raw_property_value);
    gst_structure_free(extra_controls);
    if (!ok || raw_property_value < 0) {
      return std::nullopt;
    }
    GstBitrateReadback ret{};
    ret.raw_property_value = raw_property_value;
    ret.interpreted_kbits =
        openhd::bits_per_second_to_kbits_per_second(raw_property_value);
    return ret;
  }
  const auto raw_property_value_opt =
      read_integer_property(G_OBJECT(ctrl_el.encoder), ctrl_el.property_name);
  if (!raw_property_value_opt.has_value() ||
      raw_property_value_opt.value() < 0) {
    return std::nullopt;
  }
  const auto raw_property_value = raw_property_value_opt.value();
  GstBitrateReadback ret{};
  ret.raw_property_value = raw_property_value > std::numeric_limits<int>::max()
                               ? std::numeric_limits<int>::max()
                               : static_cast<int>(raw_property_value);
  ret.interpreted_kbits =
      ctrl_el.takes_kbit
          ? ret.raw_property_value
          : openhd::bits_per_second_to_kbits_per_second(ret.raw_property_value);
  return ret;
}

static std::optional<GstQpReadback> read_qp_readback(
    const GstQpControlElement& ctrl_el) {
  if (ctrl_el.uses_extra_controls_h264_qp) {
    GstStructure* extra_controls = nullptr;
    g_object_get(ctrl_el.encoder, ctrl_el.extra_controls_property_name.c_str(),
                 &extra_controls, NULL);
    if (extra_controls == nullptr) {
      return std::nullopt;
    }
    gint qp_min = -1;
    gint qp_max = -1;
    const bool ok_min = gst_structure_get_int(
        extra_controls, ctrl_el.min_property_name.c_str(), &qp_min);
    const bool ok_max = gst_structure_get_int(
        extra_controls, ctrl_el.max_property_name.c_str(), &qp_max);
    gst_structure_free(extra_controls);
    if (!ok_min || !ok_max || qp_min < 0 || qp_max < 0) {
      return std::nullopt;
    }
    return GstQpReadback{qp_min, qp_max};
  }
  const auto qp_min_opt = read_integer_property(G_OBJECT(ctrl_el.encoder),
                                                ctrl_el.min_property_name);
  const auto qp_max_opt = read_integer_property(G_OBJECT(ctrl_el.encoder),
                                                ctrl_el.max_property_name);
  if (!qp_min_opt.has_value() || !qp_max_opt.has_value() ||
      qp_min_opt.value() < 0 || qp_max_opt.value() < 0) {
    return std::nullopt;
  }
  const auto to_int = [](int64_t raw) {
    return raw > std::numeric_limits<int>::max()
               ? std::numeric_limits<int>::max()
               : static_cast<int>(raw);
  };
  return GstQpReadback{to_int(qp_min_opt.value()), to_int(qp_max_opt.value())};
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
  } else if (camera.requires_rpi_libcamera_pipeline() &&
             !settings.force_sw_encode &&
             settings.streamed_video_format.videoCodec == VideoCodec::H264) {
    ret.encoder =
        gst_bin_get_by_name(GST_BIN(gst_pipeline), "rpi_v4l2_encoder");
    ret.property_name = "extra-controls";
    ret.takes_kbit = false;
    ret.uses_extra_controls_video_bitrate = true;
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
  } else if (camera.requires_rockchip_rv_pipeline()) {
    // We can change bitrate dynamically
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "rkmpih264enc");
    ret.property_name = "bitrate";
    ret.takes_kbit = true;
  } else if (camera.requires_rockchip1126_mpp_pipeline()) {
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "mpp_encoder");
    ret.property_name = "bps";
    ret.takes_kbit = false;
    ret.extra_properties_percent = {{"bps-min", 90}, {"bps-max", 110}};
    ret.max_raw_property_value = 25000000;
  } else if (camera.requires_rockchip3_mpp_pipeline() ||
             camera.requires_rockchip5_mpp_pipeline()) {
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "mpp_encoder");
    ret.property_name = "bps";
    ret.takes_kbit = false;
    ret.extra_properties_percent = {{"bps-min", 90}, {"bps-max", 110}};
    ret.max_raw_property_value = 25000000;
  } else if (camera.requires_nxp_imx8_v4l2_pipeline()) {
    ret.encoder = gst_bin_get_by_name(GST_BIN(gst_pipeline), "nxp_encoder");
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

static std::optional<GstQpControlElement>
get_dynamic_qp_control_element_in_pipeline(GstElement* gst_pipeline,
                                           const CameraHolder& camera_holder) {
  auto camera = camera_holder.get_camera();
  auto settings = camera_holder.get_settings();

  const auto try_qp_control =
      [&](const char* element_name, bool uses_extra_controls,
          std::string min_property,
          std::string max_property) -> std::optional<GstQpControlElement> {
    GstElement* encoder =
        gst_bin_get_by_name(GST_BIN(gst_pipeline), element_name);
    if (encoder == nullptr) {
      return std::nullopt;
    }
    GstQpControlElement ret{};
    ret.encoder = encoder;
    ret.min_property_name = std::move(min_property);
    ret.max_property_name = std::move(max_property);
    ret.uses_extra_controls_h264_qp = uses_extra_controls;
    const auto readback_opt = read_qp_readback(ret);
    if (!readback_opt.has_value()) {
      gst_object_unref(encoder);
      return std::nullopt;
    }
    const auto readback = readback_opt.value();
    openhd::log::get_default()->info(
        "Got QP control for camera {} encoder:{} min_property:{} "
        "max_property:{} current_min:{} current_max:{}",
        camera.cam_type_as_verbose_string(), element_name,
        ret.min_property_name, ret.max_property_name, readback.qp_min,
        readback.qp_max);
    return ret;
  };

  if (camera.requires_rpi_mmal_pipeline()) {
    if (auto ret = try_qp_control("rpicamsrc", false, "qp-min", "qp-max")) {
      return ret;
    }
  }
  if ((camera.requires_rpi_libcamera_pipeline() ||
       camera.requires_rpi_veye_pipeline() ||
       OHDPlatform::instance().is_rpi()) &&
      !settings.force_sw_encode &&
      settings.streamed_video_format.videoCodec == VideoCodec::H264) {
    if (auto ret =
            try_qp_control("rpi_v4l2_encoder", true, "h264_minimum_qp_value",
                           "h264_maximum_qp_value")) {
      return ret;
    }
  }
  if (camera.requires_rockchip3_mpp_pipeline() ||
      camera.requires_rockchip5_mpp_pipeline() ||
      camera.requires_rockchip1126_mpp_pipeline()) {
    if (auto ret = try_qp_control("mpp_encoder", false, "qp-min", "qp-max")) {
      return ret;
    }
  }
  if (camera.requires_rockchip_rv_pipeline()) {
    if (auto ret = try_qp_control("rkmpih264enc", false, "qp-min", "qp-max")) {
      return ret;
    }
  }
  if (camera.camera_type == X_CAM_TYPE_DUMMY_SW ||
      is_usb_camera(camera.camera_type) || settings.force_sw_encode) {
    if (auto ret = try_qp_control("swencoder", false, "qp-min", "qp-max")) {
      return ret;
    }
  }
  openhd::log::get_default()->debug(
      "Cannot find dynamic QP control element for camera {}",
      camera.cam_type_as_verbose_string());
  return std::nullopt;
}

static bool change_bitrate(const GstBitrateControlElement& ctrl_el,
                           int bitrate_kbits) {
  auto target_raw_property_value =
      ctrl_el.takes_kbit ? bitrate_kbits
                         : openhd::kbits_to_bits_per_second(bitrate_kbits);
  if (ctrl_el.max_raw_property_value > 0) {
    target_raw_property_value =
        std::min<int64_t>(target_raw_property_value,
                          ctrl_el.max_raw_property_value);
  }
  if (ctrl_el.uses_extra_controls_video_bitrate) {
    set_extra_controls_ints(ctrl_el.encoder, ctrl_el.property_name,
                            {{"video_bitrate", target_raw_property_value}});
  } else {
    if (!set_integer_property(G_OBJECT(ctrl_el.encoder), ctrl_el.property_name,
                              target_raw_property_value)) {
      return false;
    }
  }
  for (const auto& extra : ctrl_el.extra_properties_percent) {
    int64_t bounded_value =
        static_cast<int64_t>(target_raw_property_value) * extra.second / 100;
    if (ctrl_el.max_raw_property_value > 0) {
      bounded_value =
          std::min<int64_t>(bounded_value, ctrl_el.max_raw_property_value);
    }
    if (!set_integer_property(G_OBJECT(ctrl_el.encoder), extra.first,
                              bounded_value)) {
      return false;
    }
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

static bool change_qp(const GstQpControlElement& ctrl_el, int qp_min,
                      int qp_max) {
  if (qp_min < 0 || qp_min > 51 || qp_max < 0 || qp_max > 51) {
    openhd::log::get_default()->warn("Cannot change QP: invalid min:{} max:{}",
                                     qp_min, qp_max);
    return false;
  }
  if (qp_min > qp_max) {
    openhd::log::get_default()->warn("Cannot change QP: min {} > max {}",
                                     qp_min, qp_max);
    return false;
  }
  if (ctrl_el.uses_extra_controls_h264_qp) {
    set_extra_controls_ints(ctrl_el.encoder,
                            ctrl_el.extra_controls_property_name,
                            {{ctrl_el.min_property_name, qp_min},
                             {ctrl_el.max_property_name, qp_max}});
  } else {
    const auto current_qp = read_qp_readback(ctrl_el);
    const bool lower_max_below_current_min =
        current_qp.has_value() && qp_max < current_qp->qp_min;
    const bool set_min_first = lower_max_below_current_min;
    const auto set_min = [&]() {
      return set_integer_property(G_OBJECT(ctrl_el.encoder),
                                  ctrl_el.min_property_name, qp_min);
    };
    const auto set_max = [&]() {
      return set_integer_property(G_OBJECT(ctrl_el.encoder),
                                  ctrl_el.max_property_name, qp_max);
    };
    const bool ok =
        set_min_first ? (set_min() && set_max()) : (set_max() && set_min());
    if (!ok) {
      return false;
    }
  }
  const auto readback_opt = read_qp_readback(ctrl_el);
  if (!readback_opt.has_value()) {
    openhd::log::get_default()->warn("Cannot read QP after set min:{} max:{}",
                                     qp_min, qp_max);
    return false;
  }
  const auto readback = readback_opt.value();
  if (readback.qp_min != qp_min || readback.qp_max != qp_max) {
    openhd::log::get_default()->warn(
        "Cannot change QP: target_min:{} target_max:{} readback_min:{} "
        "readback_max:{}",
        qp_min, qp_max, readback.qp_min, readback.qp_max);
    return false;
  }
  openhd::log::get_default()->debug("Changed QP min:{} max:{}", qp_min, qp_max);
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

static void unref_qp_element(GstQpControlElement& element) {
  if (element.encoder) {
    openhd::log::get_default()->debug("Unref QP control element begin");
    gst_object_unref(element.encoder);
    element.encoder = nullptr;
    openhd::log::get_default()->debug("Unref QP control element end");
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
