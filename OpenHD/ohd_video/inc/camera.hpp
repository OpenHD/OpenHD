//
// Created by consti10 on 12.01.24.
//

#ifndef OPENHD_CAMERA_HPP
#define OPENHD_CAMERA_HPP

#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

/**
 * NOTE: This file is copied into QOpenHD to populate the UI.
 */

// For development, always 'works' since fully emulated in SW.
static constexpr int X_CAM_TYPE_DUMMY_SW = 0;  // Dummy sw picture
// Manually feed camera data (encoded,rtp) to openhd. Bitrate control and more
// is not working in this mode, making it only valid for development and in
// extreme cases valid for users that want to use a specific ip camera.
static constexpr int X_CAM_TYPE_EXTERNAL = 2;
// For openhd, this is exactly the same as X_CAM_TYPE_EXTERNAL - only file
// start_ip_cam.txt is created Such that the ip cam service can start forwarding
// data to openhd core.
static constexpr int X_CAM_TYPE_EXTERNAL_IP = 3;
// For development, camera that reads input from a file, and then re-encodes it
// using the platform encoder
static constexpr int X_CAM_TYPE_DEVELOPMENT_FILESRC = 4;
// ... reserved for development / custom cameras

// OpenHD supports any usb camera outputting raw video (with sw encoding).
// H264 usb cameras are not supported, since in general, they do not support
// changing bitrate/ encoding parameters.
static constexpr int X_CAM_TYPE_USB_GENERIC = 10;
// 384x292@25 cam
static constexpr int X_CAM_TYPE_USB_INFIRAY = 11;
// 256x192@25 but only 0x0@0 works (urghs)
static constexpr int X_CAM_TYPE_USB_INFIRAY_T2 = 12;
// ... reserved for future (Thermal) USB cameras

//
// RPI Specific starts here
//
// As of now, we have mmal only for the geekworm hdmi to csi adapter
static constexpr int X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI = 20;
// ... 9 reserved for future use
// ...
// RPIF stands for RPI Foundation (aka original rpi foundation cameras)
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V1_OV5647 = 30;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V2_IMX219 = 31;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708 = 32;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RPIF_HQ_IMX477 = 33;
// .... 5 reserved for future use
// Now to all the rpi libcamera arducam cameras
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYMASTERHDR_IMX708 = 40;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYVISIONPRO_IMX519 = 41;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX477M = 42;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462 = 43;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX327 = 44;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX290 = 45;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462_LOWLIGHT_MINI = 46;
// ... 13 reserved for future use
static constexpr int X_CAM_TYPE_RPI_V4L2_VEYE_2MP = 60;
static constexpr int X_CAM_TYPE_RPI_V4L2_VEYE_CSIMX307 = 61;
static constexpr int X_CAM_TYPE_RPI_V4L2_VEYE_CSSC132 = 62;
static constexpr int X_CAM_TYPE_RPI_V4L2_VEYE_MVCAM = 63;
// ... 6 reserved for future use
//
// X20 Specific starts here
//
// Right now we only have one camera, but more (might) follow.
static constexpr int X_CAM_TYPE_X20_RUNCAM_NANO = 70;
// ... 9 reserved for future use
//
// ROCK Specific starts here
//
static constexpr int X_CAM_TYPE_ROCK_HDMI_IN = 80;
static constexpr int X_CAM_TYPE_ROCK_IMX219 = 81;
//
// OpenIPC specific starts here
static constexpr int X_CAM_TYPE_OPENIPC_SOMETHING = 90;
//

// ... rest is reserved for future use
// no camera, only exists to have a default value for secondary camera (which is
// disabled by default). NOTE: The primary camera cannot be disabled !
static constexpr int X_CAM_TYPE_DISABLED = 255;  // Max for uint8_t

static std::string x_cam_type_to_string(int camera_type) {
  switch (camera_type) {
    case X_CAM_TYPE_DUMMY_SW:
      return "DUMMY";
    case X_CAM_TYPE_EXTERNAL:
      return "EXTERNAL";
    case X_CAM_TYPE_EXTERNAL_IP:
      return "EXTERNAL_IP";
    case X_CAM_TYPE_DEVELOPMENT_FILESRC:
      return "DEV_FILESRC";
    case X_CAM_TYPE_USB_GENERIC:
      return "USB";
    case X_CAM_TYPE_USB_INFIRAY:
      return "INFIRAY";
    case X_CAM_TYPE_USB_INFIRAY_T2:
      return "INFIRAY_T2";
    // All the rpi stuff begin
    case X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI:
      return "MMAL_HDMI";
    case X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V1_OV5647:
      return "RPIF_V1_OV5647";
    case X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V2_IMX219:
      return "RPIF_V2_IMX219";
    case X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708:
      return "RPIF_V3_IMX708";
    case X_CAM_TYPE_RPI_LIBCAMERA_RPIF_HQ_IMX477:
      return "RPIF_HQ_IMX477";
    case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYMASTERHDR_IMX708:
      return "ARDUCAM_SKYMASTERHDR";
    case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYVISIONPRO_IMX519:
      return "ARDUCAM_SKYVISIONPRO";
    case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX477M:
      return "ARDUCAM_IMX477M";
    case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462:
      return "ARDUCAM_IMX462";
    case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX327:
      return "ARDUCAM_IMX327";
    case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX290:
      return "ARDUCAM_IMX290";
    case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462_LOWLIGHT_MINI:
      return "ARDUCAM_IMX462_LOWLIGHT_MINI";
    case X_CAM_TYPE_RPI_V4L2_VEYE_2MP:
      return "VEYE_2MP";
    case X_CAM_TYPE_RPI_V4L2_VEYE_CSIMX307:
      return "VEYE_IMX307";
    case X_CAM_TYPE_RPI_V4L2_VEYE_CSSC132:
      return "VEYE_CSSC132";
    case X_CAM_TYPE_RPI_V4L2_VEYE_MVCAM:
      return "VEYE_MVCAM";
    // All the x20 begin
    case X_CAM_TYPE_X20_RUNCAM_NANO:
      return "X20_RUNCAM_NANO";
    // All the rock begin
    case X_CAM_TYPE_ROCK_HDMI_IN:
      return "ROCK_HDMI_IN";
    case X_CAM_TYPE_ROCK_IMX219:
      return "ROCK_IMX219";
    case X_CAM_TYPE_DISABLED:
      return "DISABLED";
    case X_CAM_TYPE_OPENIPC_SOMETHING:
      return "OPENIPC_X";
    default:
      break;
  }
  std::stringstream ss;
  ss << "UNKNOWN (" << camera_type << ")";
  return ss.str();
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
  bool requires_rpi_veye_pipeline() const {
    return camera_type >= 60 && camera_type < 70;
  }
  bool requires_x20_cedar_pipeline() const {
    return camera_type >= 70 && camera_type < 80;
  }
  bool requires_rockchip_mpp_pipeline() const {
    return camera_type >= 80 && camera_type < 90;
  }
  std::string cam_type_as_verbose_string() const {
    return x_cam_type_to_string(camera_type);
  }
  bool is_camera_type_usb_infiray() const {
    return camera_type == X_CAM_TYPE_USB_INFIRAY ||
           camera_type == X_CAM_TYPE_USB_INFIRAY_T2;
  };
  // Returns a list of known supported resolution(s).
  // They should be ordered in ascending resolution / framerate
  // Must always return at least one resolution
  // Might not return all resolutions a camera supports per HW
  // (In qopenhd, we have the experiment checkbox, where the user can enter
  // anything he likes)
  std::vector<ResolutionFramerate> get_supported_resolutions() const {
    if (requires_rpi_veye_pipeline()) {
      // Except one, all veye camera(s) only do 1080p30 -
      if (camera_type == X_CAM_TYPE_RPI_V4L2_VEYE_CSIMX307) {
        std::vector<ResolutionFramerate> ret;
        ret.push_back(ResolutionFramerate{1280, 720, 60});
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      } else {
        return {ResolutionFramerate{1920, 1080, 30}};
      }
      return {ResolutionFramerate{1920, 1080, 30}};
    } else if (requires_x20_cedar_pipeline()) {
      // also easy, 720p60 only (for now)
      return {ResolutionFramerate{1280, 720, 60}};
    } else if (camera_type == X_CAM_TYPE_USB_INFIRAY) {
      return {ResolutionFramerate{384, 292, 25}};
    } else if (camera_type == X_CAM_TYPE_USB_INFIRAY_T2) {
      // return {ResolutionFramerate{256,192,25}}; for whatever reason doesn't
      // work ...
      return {ResolutionFramerate{0, 0, 0}};
    } else if (camera_type == X_CAM_TYPE_USB_GENERIC) {
      std::vector<ResolutionFramerate> ret;
      // most likely working resolution
      ret.push_back(ResolutionFramerate{640, 480, 30});
      // auto is also a good choice on usb
      ret.push_back(ResolutionFramerate{0, 0, 0});
      return ret;
    } else if (requires_rpi_libcamera_pipeline()) {
      std::vector<ResolutionFramerate> ret;
      if (camera_type == X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462 ||
          camera_type ==
              X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462_LOWLIGHT_MINI) {
        ret.push_back(ResolutionFramerate{640, 480, 60});
        ret.push_back(ResolutionFramerate{896, 504, 60});
        ret.push_back(ResolutionFramerate{1280, 720, 60});
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      } else if (camera_type == X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX477M ||
                 camera_type == X_CAM_TYPE_RPI_LIBCAMERA_RPIF_HQ_IMX477) {
        ret.push_back(ResolutionFramerate{640, 480, 50});
        ret.push_back(ResolutionFramerate{896, 504, 50});
        ret.push_back(ResolutionFramerate{1280, 720, 50});
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      } else if (camera_type == X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V2_IMX219) {
        ret.push_back(ResolutionFramerate{640, 480, 47});
        ret.push_back(ResolutionFramerate{896, 504, 47});
        ret.push_back(ResolutionFramerate{1280, 720, 47});
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      } else if (camera_type ==
                 X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYVISIONPRO_IMX519) {
        ret.push_back(ResolutionFramerate{640, 480, 60});
        ret.push_back(ResolutionFramerate{896, 504, 60});
        ret.push_back(ResolutionFramerate{1280, 720, 60});
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      } else if (camera_type ==
                     X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYMASTERHDR_IMX708 ||
                 camera_type == X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708) {
        ret.push_back(ResolutionFramerate{640, 480, 60});
        ret.push_back(ResolutionFramerate{896, 504, 60});
        ret.push_back(ResolutionFramerate{1280, 720, 60});
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      } else if (camera_type == X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX327) {
        ret.push_back(ResolutionFramerate{640, 480, 60});
        ret.push_back(ResolutionFramerate{896, 504, 60});
        ret.push_back(ResolutionFramerate{1280, 720, 60});
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      } else if (camera_type == X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V1_OV5647) {
        ret.push_back(ResolutionFramerate{1280, 720, 30});
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      } else {
        std::cerr << "Not yet mapped:" << camera_type << std::endl;
        ret.push_back(ResolutionFramerate{1920, 1080, 30});
      }
      return ret;
    } else if (camera_type == X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI) {
      std::vector<ResolutionFramerate> ret;
      // 720p60 is the most commonly used / works in a lot of scenarios
      ret.push_back(ResolutionFramerate{1280, 720, 30});
      ret.push_back(ResolutionFramerate{1920, 1080, 25});
      ret.push_back(ResolutionFramerate{1280, 720, 60});
      return ret;
    } else if (camera_type == X_CAM_TYPE_DUMMY_SW) {
      std::vector<ResolutionFramerate> ret;
      ret.push_back(ResolutionFramerate{640, 480, 30});
      ret.push_back(ResolutionFramerate{1280, 720, 30});
      ret.push_back(ResolutionFramerate{1280, 720, 60});
      return ret;
    } else if (camera_type == X_CAM_TYPE_DEVELOPMENT_FILESRC) {
      std::vector<ResolutionFramerate> ret;
      ret.push_back(ResolutionFramerate{848, 480, 60});
      ret.push_back(ResolutionFramerate{1280, 720, 60});
      ret.push_back(ResolutionFramerate{1920, 1080, 60});
      return ret;
    }
    // Not mapped yet
    // return something that might work or might not work
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
  return cam_type == X_CAM_TYPE_ROCK_IMX219;
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

#endif  // OPENHD_CAMERA_HPP
