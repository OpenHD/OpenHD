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

#ifndef OPENHD_CAMERA_HPP
#define OPENHD_CAMERA_HPP

#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "camera_registry_generated.h"
#include "openhd_platform.h"

/**
 * NOTE: This file is copied into QOpenHD to populate the UI.
 */

static std::string x_cam_type_to_string(int camera_type) {
  return openhd::camera_registry::camera_type_to_string(camera_type);
};

struct ResolutionFramerate {
  int width_px;
  int height_px;
  int fps;
  std::string as_string() const {
    std::stringstream ss;
    ss << width_px << "x" << height_px << "@" << fps;
    return ss.str();
  }
};

struct XCamera {
  int camera_type = X_CAM_TYPE_DUMMY_SW;
  // 0 for primary camera, 1 for secondary camera
  int index;
  // Only valid if camera is of type USB
  // For CSI camera(s) we in general 'know' from platform and cam type how to
  // tell the pipeline which cam/source to use.
  int usb_v4l2_device_number;
  bool requires_rpi_mmal_pipeline() const {
    return camera_type == X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI;
  }
  bool requires_rpi_libcamera_pipeline() const {
    return camera_type >= 30 && camera_type < 60;
  }
  bool requires_x20_cedar_pipeline() const {
    return camera_type >= 70 && camera_type < 77;
  }
  bool requires_a733_pipeline() const {
    return camera_type >= 77 && camera_type < 80;
  }
  bool requires_rpi_veye_pipeline() const {
    return camera_type >= 60 && camera_type < 70;
  }
  bool x20_supports_basic_iq_params() const {
    return requires_x20_cedar_pipeline() &&
           camera_type != X_CAM_TYPE_X20_HDZERO_GENERIC;
  }
  bool requires_rockchip5_mpp_pipeline() const {
    return camera_type >= 80 && camera_type < 90;
  }
  bool requires_rockchip3_mpp_pipeline() const {
    return camera_type >= 90 && camera_type < 100;
  }
  bool requires_orqa_pipeline() const {
    return camera_type >= 122 && camera_type < 124;
  }
  bool requires_nxp_imx8_v4l2_pipeline() const {
    return camera_type >= 130 && camera_type < 140;
  }
  bool requires_rockchip_rv_pipeline() const {
    return camera_type == 140;
  }
  std::string cam_type_as_verbose_string() const {
    return x_cam_type_to_string(camera_type);
  }
  bool is_camera_type_usb_infiray() const {
    return camera_type == X_CAM_TYPE_USB_INFIRAY ||
           camera_type == X_CAM_TYPE_USB_INFIRAY_T2 ||
           camera_type == X_CAM_TYPE_USB_INFIRAY_P2_PRO ||
           camera_type == X_CAM_TYPE_USB_FLIR_VUE ||
           camera_type == X_CAM_TYPE_USB_FLIR_BOSON ||
           camera_type == X_CAM_TYPE_USB_INFIRAY_X2;
  };
  // Returns a list of known supported resolution(s).
  // They should be ordered in ascending resolution / framerate
  // Must always return at least one resolution
  // Might not return all resolutions a camera supports per HW
  // (In qopenhd, we have the experiment checkbox, where the user can enter
  // anything he likes)
  std::vector<ResolutionFramerate> get_supported_resolutions() const {
    const auto* entry =
        openhd::camera_registry::find_camera_resolutions(camera_type);
    if (entry && entry->resolution_count > 0) {
      std::vector<ResolutionFramerate> ret;
      ret.reserve(entry->resolution_count);
      for (std::size_t i = 0; i < entry->resolution_count; ++i) {
        const auto& resolution = entry->resolutions[i];
        ret.push_back(ResolutionFramerate{
            resolution.width_px, resolution.height_px, resolution.fps});
      }
      return ret;
    }
    if (requires_rpi_libcamera_pipeline()) {
      std::cerr << "Not yet mapped:" << camera_type << std::endl;
      return {ResolutionFramerate{1920, 1080, 30}};
    }
    return {ResolutionFramerate{640, 480, 30}};
  }
  // We default to the last supported resolution
  [[nodiscard]] ResolutionFramerate get_default_resolution_fps() const {
    auto supported_resolutions = get_supported_resolutions();
    return supported_resolutions.at(supported_resolutions.size() - 1);
  }
};

static bool is_rpi_csi_camera(int cam_type) {
  return cam_type >= 20 && cam_type <= 69;
}
static bool is_rock_csi_camera(int cam_type) {
  return cam_type >= 80 && cam_type <= 99;
}
static bool is_orqa_csi_camera(int cam_type) {
  return cam_type >= 122 && cam_type <= 124;
}
static bool is_usb_camera(int cam_type) {
  return cam_type >= 10 && cam_type < 19;
}

static bool is_valid_primary_cam_type(int cam_type) {
  if (cam_type >= 0 && cam_type < X_CAM_TYPE_DISABLED) return true;
  return false;
}
static bool is_valid_secondary_cam_type(int cam_type) {
  if (is_usb_camera(cam_type)) return true;
  if (cam_type == X_CAM_TYPE_DUMMY_SW || cam_type == X_CAM_TYPE_EXTERNAL ||
      cam_type == X_CAM_TYPE_EXTERNAL_IP || cam_type == X_CAM_TYPE_DISABLED) {
    return true;
  }
  return false;
}
// Takes a string in the from {width}x{height}@{framerate}
// e.g. 1280x720@30
static std::optional<ResolutionFramerate> parse_video_format(
    const std::string& videoFormat) {
  // 0x0@0 is a valid resolution (omit resolution / fps in the pipeline)
  if (videoFormat == "0x0@0") return ResolutionFramerate{0, 0, 0};
  // Otherwise, we need at least 6 characters (0x0@0 is 5 characters)
  if (videoFormat.size() <= 5) {
    return std::nullopt;
  }
  ResolutionFramerate tmp_video_format{0, 0, 0};
  const std::regex reg{R"((\d*)x(\d*)\@(\d*))"};
  std::smatch result;
  if (std::regex_search(videoFormat, result, reg)) {
    if (result.size() == 4) {
      // openhd::log::get_default()->debug("result[0]=["+result[0].str()+"]");
      tmp_video_format.width_px = atoi(result[1].str().c_str());
      tmp_video_format.height_px = atoi(result[2].str().c_str());
      tmp_video_format.fps = atoi(result[3].str().c_str());
      return tmp_video_format;
    }
  }
  return std::nullopt;
}

//
// Used in QOpenHD UI
//
static std::string get_verbose_string_of_resolution(
    const ResolutionFramerate& resolution_framerate) {
  if (resolution_framerate.width_px == 0 &&
      resolution_framerate.height_px == 0 && resolution_framerate.fps == 0) {
    return "AUTO";
  }
  std::stringstream ss;
  if (resolution_framerate.width_px == 640 &&
      resolution_framerate.height_px == 480) {
    ss << "VGA 4:3";
  } else if (resolution_framerate.width_px == 848 &&
             resolution_framerate.height_px == 480) {
    ss << "VGA 16:9";
  } else if (resolution_framerate.width_px == 896 &&
             resolution_framerate.height_px == 504) {
    ss << "SD 16:9";
  } else if (resolution_framerate.width_px == 1280 &&
             resolution_framerate.height_px == 720) {
    ss << "HD 16:9";
  } else if (resolution_framerate.width_px == 1920 &&
             resolution_framerate.height_px == 1080) {
    ss << "FHD 16:9";
  } else if (resolution_framerate.width_px == 2560 &&
             resolution_framerate.height_px == 1440) {
    ss << "2K 16:9";
  } else {
    ss << resolution_framerate.width_px << "x"
       << resolution_framerate.height_px;
  }
  ss << "\n" << resolution_framerate.fps << "fps";
  return ss.str();
}

static std::string get_v4l2_device_name_string(int value) {
  std::stringstream ss;
  ss << "/dev/video" << value;
  return ss.str();
}

/**
 * On platforms with many cameras (e.g. rpi) we need a differentiation by
 * manufacturer to make a nice UI - otherwise, the choices are overwhelming.
 * Manufacturer is not really the right name for all categories that result
 * here, but it is 'okay' for the UI in qopenhd.
 */
struct CameraNameAndType {
  std::string name;
  int type;
};
struct ManufacturerForPlatform {
  std::string manufacturer_name;
  std::vector<CameraNameAndType> cameras;
};
/**
 * Return: a list of categories for this platform.
 * Each category has a list of valid camera types (for this platform).
 * @param platform_type unique platform type
 * @param is_secondary selection is different for secondary cam,most notably, we
 * only support usb, develop and a 'disabled' type.
 */
static std::vector<ManufacturerForPlatform> get_camera_choices_for_platform(
    int platform_type, bool is_secondary) {
  const auto to_camera_list = [](const openhd::camera_registry::CameraUiEntry*
                                     entries,
                                 std::size_t entry_count) {
    std::vector<CameraNameAndType> cameras;
    cameras.reserve(entry_count);
    for (std::size_t i = 0; i < entry_count; ++i) {
      cameras.push_back(CameraNameAndType{entries[i].label, entries[i].type});
    }
    return cameras;
  };
  const auto to_manufacturers =
      [&](const openhd::camera_registry::ManufacturerEntry* entries,
          std::size_t entry_count) {
        std::vector<ManufacturerForPlatform> manufacturers;
        manufacturers.reserve(entry_count);
        for (std::size_t i = 0; i < entry_count; ++i) {
          manufacturers.push_back(ManufacturerForPlatform{
              entries[i].name,
              to_camera_list(entries[i].cameras, entries[i].camera_count)});
        }
        return manufacturers;
      };

  if (is_secondary) {
    return to_manufacturers(
        openhd::camera_registry::kSecondaryManufacturers,
        openhd::camera_registry::kSecondaryManufacturerCount);
  }

  const auto* platform_entry =
      openhd::camera_registry::find_platform_camera_choices(platform_type);
  if (platform_entry) {
    return to_manufacturers(platform_entry->manufacturers,
                            platform_entry->manufacturer_count);
  }
  return to_manufacturers(openhd::camera_registry::kFallbackManufacturers,
                          openhd::camera_registry::kFallbackManufacturerCount);
}

#endif  // OPENHD_CAMERA_HPP
