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

#ifndef OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_
#define OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_

#include <spdlog/spdlog.h>

#include <algorithm>
#include <functional>
#include <sstream>

#include "camera.hpp"
#include "camera_settings.hpp"
#include "openhd_action_handler.h"
#include "openhd_bitrate.h"
#include "openhd_plugin_manager.h"
#include "openhd_settings_directories.h"
#include "openhd_settings_imp.h"
#include "openhd_settings_persistent.h"
#include "openhd_util.h"
#include "openhd_sock.h"
#include "usb_thermal_cam_helper.h"

// Holds the immutable (camera) and mutable (camera_settings) information about
// a camera. Changes in the camera
// settings are propagated through this class.
class CameraHolder :
    // persistence via JSON
    public openhd::PersistentSettings<CameraSettings> {
 public:
  explicit CameraHolder(XCamera camera)
      : m_camera(std::move(camera)),
        openhd::PersistentSettings<CameraSettings>(
            openhd::get_video_settings_directory()) {
    // read previous settings or create default ones
    init();
    // Hornet and Rekindle expose fixed CSI modes. Migrate values persisted by
    // older releases so CamInfo and RESOLUTION_FPS match the native capture.
    if (m_camera.camera_type == X_CAM_TYPE_ORQA_HORNET ||
        m_camera.camera_type == X_CAM_TYPE_ORQA_REKINDLE) {
      const auto native_format = m_camera.get_default_resolution_fps();
      auto& configured_format = unsafe_get_settings().streamed_video_format;
      if (configured_format.width != native_format.width_px ||
          configured_format.height != native_format.height_px ||
          configured_format.framerate != native_format.fps) {
        configured_format.width = native_format.width_px;
        configured_format.height = native_format.height_px;
        configured_format.framerate = native_format.fps;
        persist(false);
      }
    }
  }
  [[nodiscard]] const XCamera& get_camera() const { return m_camera; }
  using VIDEO_BITRATE_CHANGED_CALLBACK = std::function<void(int bitrate_kbits)>;
  using VIDEO_QP_CHANGED_CALLBACK = std::function<void(int qp_min, int qp_max)>;
  using VIDEO_ROI_CHANGED_CALLBACK = std::function<void()>;
  using VIDEO_RECORDING_CHANGED_CALLBACK = std::function<void()>;
  using VIDEO_DEBUG_CHANGED_CALLBACK = std::function<void()>;
  using VIDEO_FORCE_KEYFRAME_CALLBACK = std::function<void()>;
  void register_video_bitrate_listener(VIDEO_BITRATE_CHANGED_CALLBACK cb) {
    m_video_bitrate_changed_callback = std::move(cb);
  }
  void register_video_qp_listener(VIDEO_QP_CHANGED_CALLBACK cb) {
    m_video_qp_changed_callback = std::move(cb);
  }
  void register_video_roi_listener(VIDEO_ROI_CHANGED_CALLBACK cb) {
    m_video_roi_changed_callback = std::move(cb);
  }
  void register_video_recording_listener(VIDEO_RECORDING_CHANGED_CALLBACK cb) {
    m_video_recording_changed_callback = std::move(cb);
  }
  void register_video_debug_listener(VIDEO_DEBUG_CHANGED_CALLBACK cb) {
    m_video_debug_changed_callback = std::move(cb);
  }
  void register_video_force_keyframe_listener(VIDEO_FORCE_KEYFRAME_CALLBACK cb) {
    m_video_force_keyframe_callback = std::move(cb);
  }
  // Settings hacky begin
  std::vector<openhd::Setting> get_all_settings();
  bool set_enable_streaming(int enable) {
    if (!openhd::validate_yes_or_no(enable)) return false;
    unsafe_get_settings().enable_streaming = static_cast<bool>(enable);
    persist();
    return true;
  }
  bool set_ip_camera_pipeline(const std::string& pipeline) {
    // MAVLink PARAM_EXT_VALUE has a 128-byte payload. Keep one byte available
    // for a terminator so every MAVLink implementation can handle the value.
    if (pipeline.size() > 127 || pipeline.find('\n') != std::string::npos ||
        pipeline.find('\r') != std::string::npos) {
      return false;
    }
    unsafe_get_settings().ip_camera_pipeline = pipeline;
    persist();
    return true;
  }
  bool set_ip_camera_address(const std::string& address) {
    if (!OHDUtil::is_valid_ip(address)) return false;
    unsafe_get_settings().ip_camera_address = address;
    persist();
    notify_plugin_video_settings_changed();
    return true;
  }
  // it is only possible to validate setting the video width,height and fps
  // if we do them together
  bool set_video_width_height_framerate(int width, int height, int framerate) {
    if (!openhd::validate_video_width_height_fps(width, height, framerate)) {
      return false;
    }
    if (m_camera.camera_type == X_CAM_TYPE_ORQA_HORNET ||
        m_camera.camera_type == X_CAM_TYPE_ORQA_REKINDLE) {
      const auto native_format = m_camera.get_default_resolution_fps();
      if (width != native_format.width_px ||
          height != native_format.height_px ||
          framerate != native_format.fps) {
        return false;
      }
    }
    unsafe_get_settings().streamed_video_format.width = width;
    unsafe_get_settings().streamed_video_format.height = height;
    unsafe_get_settings().streamed_video_format.framerate = framerate;
    persist();
    notify_plugin_video_settings_changed();
    return true;
  }
  bool set_video_codec(int codec) {
    if (!openhd::validate_video_codec(codec)) {
      return false;
    }
    unsafe_get_settings().streamed_video_format.videoCodec =
        video_codec_from_int(codec);
    persist();
    notify_plugin_video_settings_changed();
    return true;
  }
  bool set_video_bitrate(int bitrate_mbits) {
    if (!openhd::validate_bitrate_mbits(bitrate_mbits)) {
      return false;
    }
    const auto previous_kbits = get_settings().h26x_bitrate_kbits;
    const auto requested_kbits =
        openhd::mbits_to_kbits_per_second(bitrate_mbits);
    unsafe_get_settings().h26x_bitrate_kbits =
        clamp_video_bitrate_kbits(requested_kbits);
    if (requested_kbits != unsafe_get_settings().h26x_bitrate_kbits) {
      openhd::log::get_default()->warn(
          "Camera{} bitrate request {} kbit/s limited to platform/camera "
          "maximum "
          "{} kbit/s",
          m_camera.index, requested_kbits,
          get_max_video_bitrate_kbits());
    }
    openhd::log::get_default()->debug(
        "Camera{} BITRATE_MBITS request:{} old_kbits:{} new_kbits:{}",
        m_camera.index, bitrate_mbits, previous_kbits,
        unsafe_get_settings().h26x_bitrate_kbits);
    persist(false);
    if (m_video_bitrate_changed_callback &&
        unsafe_get_settings().h26x_bitrate_kbits != previous_kbits) {
      m_video_bitrate_changed_callback(
          unsafe_get_settings().h26x_bitrate_kbits);
    }
    if (unsafe_get_settings().h26x_bitrate_kbits != previous_kbits) {
      notify_plugin_video_settings_changed();
    }
    return true;
  }
  void notify_plugin_video_settings_changed() const {
    const auto& settings = get_settings();
    const auto& format = settings.streamed_video_format;
    const openhd_plugin_video_settings_event event{
        sizeof(openhd_plugin_video_settings_event),
        static_cast<uint32_t>(m_camera.index),
        m_camera.camera_type,
        settings.h26x_bitrate_kbits,
        static_cast<int32_t>(format.videoCodec),
        static_cast<uint16_t>(format.width),
        static_cast<uint16_t>(format.height),
        static_cast<uint16_t>(format.framerate),
        settings.ip_camera_address.c_str()};
    openhd::PluginManager::instance().notify_video_settings_changed(event);
  }
  [[nodiscard]] int get_max_video_bitrate_kbits() const {
    return m_camera.get_max_video_bitrate_kbits(
        OHDPlatform::instance().platform_type);
  }
  [[nodiscard]] int clamp_video_bitrate_kbits(int bitrate_kbits) const {
    return std::clamp(bitrate_kbits, 1000,
                      get_max_video_bitrate_kbits());
  }
  bool set_qp_min(int value) {
    if (!openhd::validate_h26x_qp(value)) {
      return false;
    }
    if (value > get_settings().qp_max) {
      openhd::log::get_default()->warn(
          "Invalid Camera{} QP_MIN {}, current QP_MAX is {}", m_camera.index,
          value, get_settings().qp_max);
      return false;
    }
    const auto previous_qp_min = get_settings().qp_min;
    unsafe_get_settings().qp_min = value;
    openhd::log::get_default()->debug(
        "Camera{} QP_MIN request:{} old:{} new:{}", m_camera.index, value,
        previous_qp_min, unsafe_get_settings().qp_min);
    persist(false);
    if (m_video_qp_changed_callback &&
        unsafe_get_settings().qp_min != previous_qp_min) {
      m_video_qp_changed_callback(unsafe_get_settings().qp_min,
                                  unsafe_get_settings().qp_max);
    }
    return true;
  }
  bool set_qp_max(int value) {
    if (!openhd::validate_h26x_qp(value)) {
      return false;
    }
    if (value < get_settings().qp_min) {
      openhd::log::get_default()->warn(
          "Invalid Camera{} QP_MAX {}, current QP_MIN is {}", m_camera.index,
          value, get_settings().qp_min);
      return false;
    }
    const auto previous_qp_max = get_settings().qp_max;
    unsafe_get_settings().qp_max = value;
    openhd::log::get_default()->debug(
        "Camera{} QP_MAX request:{} old:{} new:{}", m_camera.index, value,
        previous_qp_max, unsafe_get_settings().qp_max);
    persist(false);
    if (m_video_qp_changed_callback &&
        unsafe_get_settings().qp_max != previous_qp_max) {
      m_video_qp_changed_callback(unsafe_get_settings().qp_min,
                                  unsafe_get_settings().qp_max);
    }
    return true;
  }
  bool set_qp_pid_enable(int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    unsafe_get_settings().qp_pid_enable = static_cast<bool>(value);
    openhd::log::get_default()->debug("Camera{} QP_PID_ENABLE request:{}",
                                      m_camera.index, value);
    persist(false);
    return true;
  }
  bool set_rk_bitrate_pid_enable(int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    unsafe_get_settings().rk_bitrate_pid_enable = static_cast<bool>(value);
    openhd::log::get_default()->debug("Camera{} RK_BITRATE_PID request:{}",
                                      m_camera.index, value);
    persist(false);
    return true;
  }
  bool set_mpp_roi_enable(int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    unsafe_get_settings().mpp_roi_enable = value != 0;
    persist(false);
    if (m_video_roi_changed_callback) m_video_roi_changed_callback();
    return true;
  }
  bool set_mpp_roi_geometry(int component, int value) {
    if (value < 0 || value > 100 || component < 0 || component > 3) {
      return false;
    }
    const auto& current = get_settings();
    int x = current.mpp_roi_x_percent;
    int y = current.mpp_roi_y_percent;
    int w = current.mpp_roi_width_percent;
    int h = current.mpp_roi_height_percent;
    if (component == 0) x = value;
    if (component == 1) y = value;
    if (component == 2) w = value;
    if (component == 3) h = value;
    if (w <= 0 || h <= 0 || x + w > 100 || y + h > 100) return false;
    auto& settings = unsafe_get_settings();
    settings.mpp_roi_x_percent = x;
    settings.mpp_roi_y_percent = y;
    settings.mpp_roi_width_percent = w;
    settings.mpp_roi_height_percent = h;
    persist(false);
    if (m_video_roi_changed_callback) m_video_roi_changed_callback();
    return true;
  }
  bool set_mpp_roi_quality(int value) {
    if (value < -51 || value > 51) return false;
    unsafe_get_settings().mpp_roi_quality = value;
    persist(false);
    if (m_video_roi_changed_callback) m_video_roi_changed_callback();
    return true;
  }
  bool set_mpp_intra_refresh_enable(int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    unsafe_get_settings().mpp_intra_refresh_enable = value != 0;
    persist(false);
    if (m_video_roi_changed_callback) m_video_roi_changed_callback();
    return true;
  }
  bool set_mpp_intra_refresh_mode(int value) {
    // Rockchip MPP exposes only row (0) and column (1) refresh. Its enum value
    // 2 is the end sentinel, not a sparse-block mode.
    if (value < 0 || value > 1) return false;
    unsafe_get_settings().mpp_intra_refresh_mode = value;
    persist(false);
    if (m_video_roi_changed_callback) m_video_roi_changed_callback();
    return true;
  }
  bool set_mpp_intra_refresh_num(int value) {
    if (value < 1 || value > 32) return false;
    unsafe_get_settings().mpp_intra_refresh_num = value;
    persist(false);
    if (m_video_roi_changed_callback) m_video_roi_changed_callback();
    return true;
  }
  bool set_mpp_record_bitrate(int value_mbits) {
    if (value_mbits < 5 || value_mbits > 200) return false;
    unsafe_get_settings().mpp_record_bitrate_kbits = value_mbits * 1000;
    persist(false);
    if (m_video_recording_changed_callback)
      m_video_recording_changed_callback();
    return true;
  }
  bool set_mpp_record_qp_min(int value) {
    if (!openhd::validate_h26x_qp(value) ||
        value > get_settings().mpp_record_qp_max) return false;
    unsafe_get_settings().mpp_record_qp_min = value;
    persist(false);
    if (m_video_recording_changed_callback)
      m_video_recording_changed_callback();
    return true;
  }
  bool set_mpp_record_qp_max(int value) {
    if (!openhd::validate_h26x_qp(value) ||
        value < get_settings().mpp_record_qp_min) return false;
    unsafe_get_settings().mpp_record_qp_max = value;
    persist(false);
    if (m_video_recording_changed_callback)
      m_video_recording_changed_callback();
    return true;
  }
  bool set_mpp_debug_noise(int value) {
    if (value < 0 || value > 100) return false;
    unsafe_get_settings().mpp_debug_noise_percent = value;
    persist(false);
    notify_video_debug_changed();
    return true;
  }
  bool set_mpp_debug_packet_loss(int value) {
    if (value < 0 || value > 95) return false;
    unsafe_get_settings().mpp_debug_packet_loss_percent = value;
    persist(false);
    notify_video_debug_changed();
    return true;
  }
  bool set_mpp_debug_keyframe_loss(int value) {
    if (value < 0 || value > 95) return false;
    unsafe_get_settings().mpp_debug_keyframe_loss_percent = value;
    persist(false);
    notify_video_debug_changed();
    return true;
  }
  bool force_mpp_keyframe(int value) {
    if (value != 1 || !m_video_force_keyframe_callback) return false;
    m_video_force_keyframe_callback();
    return true;
  }
  bool set_mpp_debug_bitrate_sweep(int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    unsafe_get_settings().mpp_debug_bitrate_sweep = value != 0;
    persist(false);
    notify_video_debug_changed();
    return true;
  }
  bool set_mpp_debug_bitrate_min(int value_mbits) {
    if (value_mbits < 1 || value_mbits > 200 ||
        value_mbits * 1000 > get_settings().mpp_debug_bitrate_max_kbits)
      return false;
    unsafe_get_settings().mpp_debug_bitrate_min_kbits =
        clamp_video_bitrate_kbits(value_mbits * 1000);
    persist(false);
    notify_video_debug_changed();
    return true;
  }
  bool set_mpp_debug_bitrate_max(int value_mbits) {
    if (value_mbits < 1 || value_mbits > 200 ||
        value_mbits * 1000 < get_settings().mpp_debug_bitrate_min_kbits)
      return false;
    unsafe_get_settings().mpp_debug_bitrate_max_kbits =
        clamp_video_bitrate_kbits(value_mbits * 1000);
    persist(false);
    notify_video_debug_changed();
    return true;
  }
  bool set_mpp_debug_bitrate_period(int seconds) {
    if (seconds < 2 || seconds > 120) return false;
    unsafe_get_settings().mpp_debug_bitrate_period_seconds = seconds;
    persist(false);
    notify_video_debug_changed();
    return true;
  }
  bool set_air_recording(int recording_enable);
  // EXTRA - sets the air recording param to disabled when we run out of space -
  // this should be called in regular intervals
  void check_remaining_space_air_recording(bool call_callback);

  bool set_camera_rotation(int value) {
    if (!openhd::validate_camera_rotation(value)) {
      return false;
    }
    unsafe_get_settings().camera_rotation_degree = value;
    persist();
    return true;
  }
  bool set_keyframe_interval(int value) {
    if (!openhd::validate_rpi_keyframe_interval(value)) return false;
    unsafe_get_settings().h26x_keyframe_interval = value;
    persist();
    return true;
  }
  bool set_intra_refresh_type(int value) {
    if (!openhd::validate_rpi_intra_refresh_type(value)) return false;
    unsafe_get_settings().h26x_intra_refresh_type = value;
    persist();
    return true;
  }
  bool set_h26x_num_slices(int value) {
    unsafe_get_settings().h26x_num_slices = value;
    persist();
    return true;
  }
  bool set_nxp_enable_aud(int value) {
    if (!openhd::validate_yes_or_no(value)) return false;
    unsafe_get_settings().nxp_enable_aud = static_cast<bool>(value);
    persist();
    return true;
  }
  bool set_openhd_flip(int value) {
    if (!(value >= OPENHD_FLIP_NONE &&
          value <= OPENHD_FLIP_VERTICAL_AND_HORIZONTAL))
      return false;
    unsafe_get_settings().openhd_flip = value;
    persist();
    return true;
  }
  bool set_openhd_brightness(int value) {
    if (!openhd::validate_openhd_brightness(value)) return false;
    unsafe_get_settings().openhd_brightness = value;
    persist();
    return true;
  }
  bool set_openhd_sharpness(int value) {
    if (!openhd::validate_openhd_sharpness(value)) return false;
    unsafe_get_settings().openhd_sharpness = value;
    persist(true);
    return true;
  }
  bool set_openhd_contrast(int value) {
    if (!openhd::validate_openhd_contrast(value)) return false;
    unsafe_get_settings().openhd_contrast = value;
    persist(true);
    return true;
  }
  bool set_openhd_saturation(int value) {
    if (!openhd::validate_openhd_saturation(value)) return false;
    unsafe_get_settings().openhd_saturation = value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_ev_value(int value) {
    if (!openhd::validate_rpi_libcamera_ev_value(value)) return false;
    unsafe_get_settings().rpi_libcamera_ev_value = value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_denoise_index(int value) {
    if (!openhd::validate_rpi_libcamera_doenise_index(value)) return false;
    unsafe_get_settings().rpi_libcamera_denoise_index = value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_awb_index(int value) {
    if (!openhd::validate_rpi_libcamera_awb_index(value)) return false;
    unsafe_get_settings().rpi_libcamera_awb_index = value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_metering_index(int value) {
    if (!openhd::validate_rpi_libcamera_metering_index(value)) return false;
    unsafe_get_settings().rpi_libcamera_metering_index = value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_exposure_index(int value) {
    if (!openhd::validate_rpi_libcamera_exposure_index(value)) return false;
    unsafe_get_settings().rpi_libcamera_exposure_index = value;
    persist(true);
    return true;
  }
  bool set_rpi_libcamera_shutter_microseconds(int value) {
    if (!openhd::validate_rpi_libcamera_shutter_microseconds(value))
      return false;
    unsafe_get_settings().rpi_libcamera_shutter_microseconds = value;
    persist(true);
    return true;
  }
  bool set_encryption_enable(int enable) {
    if (!openhd::validate_yes_or_no(enable)) return false;
    unsafe_get_settings().enable_ultra_secure_encryption = enable;
    // Doesn't need restart of the camera pipeline, weather to encrypt or not is
    // passed per frame to wb
    persist(false);
    return true;
  }
  bool set_infiray_custom_control_zoom_absolute_colorpalete(int value) {
    if (!openhd::is_valid_infiray_custom_control_zoom_absolute_value(value))
      return false;
    unsafe_get_settings().infiray_custom_control_zoom_absolute_colorpalete =
        value;
    persist(false);  // No restart required
    openhd::set_infiray_custom_control_zoom_absolute_async(
        value, m_camera.usb_v4l2_device_number);
    return true;
  }
  // The CSI to HDMI adapter has an annoying bug where it actually doesn't allow
  // changing the framerate but takes whatever the host provides (e.g. the hdmi
  // card). Util to check if we need to apply the "reduce bitrate by half" NOTE:
  // This is not completely correct - it assumes the provider (e.g. gopro)
  // always gives 60fps and in case the user selects 720p@49fps for example, the
  // bitrate is too low. However, rather be too low than too high - the user can
  // always go higher if he needs to.
  bool requires_half_bitrate_workaround() const {
    if (m_camera.camera_type == X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI &&
        get_settings().streamed_video_format.framerate != 60)
      return true;
    return false;
  }
  // Settings hacky end
 private:
  void notify_video_debug_changed() {
    if (m_video_debug_changed_callback) m_video_debug_changed_callback();
  }
  // Camera info is immutable
  const XCamera m_camera;
  VIDEO_BITRATE_CHANGED_CALLBACK m_video_bitrate_changed_callback = nullptr;
  VIDEO_QP_CHANGED_CALLBACK m_video_qp_changed_callback = nullptr;
  VIDEO_ROI_CHANGED_CALLBACK m_video_roi_changed_callback = nullptr;
  VIDEO_RECORDING_CHANGED_CALLBACK m_video_recording_changed_callback = nullptr;
  VIDEO_DEBUG_CHANGED_CALLBACK m_video_debug_changed_callback = nullptr;
  VIDEO_FORCE_KEYFRAME_CALLBACK m_video_force_keyframe_callback = nullptr;

 private:
  [[nodiscard]] std::string get_unique_filename() const override {
    std::stringstream ss;
    ss << m_camera.cam_type_as_verbose_string() << "_" << m_camera.index
       << ".json";
    return ss.str();
  }
  std::optional<CameraSettings> impl_deserialize(
      const std::string& file_as_string) const override;
  std::string imp_serialize(const CameraSettings& data) const override;
  [[nodiscard]] CameraSettings create_default() const override {
    auto ret = CameraSettings{};
    if (m_camera.camera_type == X_CAM_TYPE_EXTERNAL_IP) {
      ret.ip_camera_pipeline = DEFAULT_IP_CAMERA_PIPELINE;
      ret.ip_camera_address = DEFAULT_IP_CAMERA_ADDRESS;
    }
    if (OHDPlatform::instance().is_rpi5()) {
      ret.force_sw_encode = true;
    }
    auto default_resolution = m_camera.get_default_resolution_fps();
    ret.streamed_video_format.width = default_resolution.width_px;
    ret.streamed_video_format.height = default_resolution.height_px;
    ret.streamed_video_format.framerate = default_resolution.fps;

    const auto& sysutil_settings =
        []() -> const std::optional<openhd::SysutilSettings>& {
      static const std::optional<openhd::SysutilSettings> cached =
          openhd::request_sysutil_settings();
      return cached;
    }();
    if (sysutil_settings.has_value()) {
      if (m_camera.index == 1 &&
          m_camera.camera_type == X_CAM_TYPE_EXTERNAL_IP) {
        if (sysutil_settings->has_camera2_ip_camera_address &&
            !sysutil_settings->camera2_ip_camera_address.empty()) {
          ret.ip_camera_address =
              sysutil_settings->camera2_ip_camera_address;
        }
        if (sysutil_settings->has_camera2_ip_camera_pipeline &&
            !sysutil_settings->camera2_ip_camera_pipeline.empty()) {
          ret.ip_camera_pipeline =
              sysutil_settings->camera2_ip_camera_pipeline;
        }
      }
      std::string requested_resolution_fps;
      if (m_camera.index == 0 && sysutil_settings->has_camera_resolution_fps) {
        requested_resolution_fps = sysutil_settings->camera_resolution_fps;
      } else if (m_camera.index == 1 &&
                 sysutil_settings->has_camera2_resolution_fps) {
        requested_resolution_fps = sysutil_settings->camera2_resolution_fps;
      }

      if (!requested_resolution_fps.empty()) {
        const auto requested_format =
            parse_video_format(requested_resolution_fps);
        if (requested_format.has_value()) {
          const auto supported_formats = m_camera.get_supported_resolutions();
          const auto supported_it = std::find_if(
              supported_formats.begin(), supported_formats.end(),
              [&](const ResolutionFramerate& candidate) {
                return candidate.width_px == requested_format->width_px &&
                       candidate.height_px == requested_format->height_px &&
                       candidate.fps == requested_format->fps;
              });
          if (supported_it != supported_formats.end()) {
            ret.streamed_video_format.width = requested_format->width_px;
            ret.streamed_video_format.height = requested_format->height_px;
            ret.streamed_video_format.framerate = requested_format->fps;
            openhd::log::get_default()->debug(
                "Using sysutils camera{} resolution override {}",
                m_camera.index + 1, requested_resolution_fps);
          } else {
            openhd::log::get_default()->warn(
                "Ignoring unsupported sysutils camera{} resolution override {}",
                m_camera.index + 1, requested_resolution_fps);
          }
        } else {
          openhd::log::get_default()->warn(
              "Ignoring invalid sysutils camera{} resolution override {}",
              m_camera.index + 1, requested_resolution_fps);
        }
      }
    }

    if (OHDPlatform::instance().is_x20()) {
      // Better choice for the x20
      ret.h26x_keyframe_interval = 8;
    }
    if (m_camera.requires_rockchip3_mpp_pipeline() ||
        m_camera.requires_rockchip5_mpp_pipeline()) {
      ret.rk_bitrate_pid_enable = true;
    }
    return ret;
  }
};

#endif  // OPENHD_OPENHD_OHD_VIDEO_INC_CAMERA_HOLDER_H_
