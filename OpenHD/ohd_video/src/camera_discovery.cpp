#include "camera_discovery.h"

#include <fcntl.h>

#ifdef ENABLE_USB_CAMERAS
#include <libv4l2.h>
#include <linux/videodev2.h>
#endif

#include <sys/ioctl.h>
#include <sys/stat.h>
#ifdef OPENHD_LIBUSB_PRESENT
#include <libusb.h>
#endif

#include <iostream>
#include <regex>

#include "camera.hpp"
// #include "libcamera_detect.hpp"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

// annoying linux platform specifics
#ifndef V4L2_PIX_FMT_H265
#define V4L2_PIX_FMT_H265 V4L2_PIX_FMT_HEVC
#endif

/**
 * Helper for the discover thermal cameras step.
 * It is a bit more complicated, once we actually support them the code here
 * will probably blow a bit. Rn I just copy pasted stephens code for the flir
 * and seek here
 */
namespace DThermalCamerasHelper {
static constexpr auto FLIR_ONE_VENDOR_ID = 0x09cb;
static constexpr auto FLIR_ONE_PRODUCT_ID = 0x1996;

static constexpr auto SEEK_COMPACT_VENDOR_ID = 0x289d;
static constexpr auto SEEK_COMPACT_PRODUCT_ID = 0x0010;

static constexpr auto SEEK_COMPACT_PRO_VENDOR_ID = 0x289d;
static constexpr auto SEEK_COMPACT_PRO_PRODUCT_ID = 0x0011;
/*
 * What this is:
 *
 * We're detecting whether the flir one USB thermal camera is connected. We then
 * run the flir one driver with systemd.
 *
 * What happens after:
 *
 * The systemd service starts, finds the camera and begins running on the device
 * node we select. Then we will let it be found by the rest of this class just
 * like any other camera, so it gets recorded in the manifest and found by the
 * camera service.
 *
 *
 * todo: this should really be marking the camera as a thermal cam instead of
 * starting v4l2loopback and abstracting it away like this, but the camera
 * service doesn't yet have a thermal handling class
 */
static void enableFlirIfFound() {
#ifdef OPENHD_LIBUSB_PRESENT
  libusb_context *context = nullptr;
  int result = libusb_init(&context);
  if (result) {
    openhd::log::get_default()->warn("Failed to initialize libusb");
    return;
  }
  libusb_device_handle *handle = libusb_open_device_with_vid_pid(
      nullptr, FLIR_ONE_VENDOR_ID, FLIR_ONE_PRODUCT_ID);
  if (!handle) return;
  // Close libusb handles after we don't need them anymore
  libusb_close(handle);
  // TODO missing r.n
  OHDUtil::run_command("systemctl", {"start", "flirone"});
#endif  // OPENHD_LIBUSB_PRESENT
}

/*
 * What this is:
 *
 * We're detecting whether the 2 known Seek thermal USB cameras are connected,
 * then constructing arguments for the seekthermal driver depending on which
 * model it is. We then run the seek driver with systemd using the arguments
 * file we provided to it in seekthermal.service in the libseek-thermal package.
 *
 * What happens after:
 *
 * The systemd service starts, finds the camera and begins running on the device
 * node we select. Then we will let it be found by the rest of this class just
 * like any other camera, so it gets recorded in the manifest and found by the
 * camera service.
 *
 *
 * todo: this should pull the camera settings from the settings file if
 * available
 */
static void enableSeekIfFound() {
#ifdef OPENHD_LIBUSB_PRESENT
  libusb_context *context = nullptr;
  int result = libusb_init(&context);
  if (result) {
    openhd::log::get_default()->warn("Failed to initialize libusb");
    return;
  }
  libusb_device_handle *handle_compact = libusb_open_device_with_vid_pid(
      nullptr, SEEK_COMPACT_VENDOR_ID, SEEK_COMPACT_PRODUCT_ID);
  libusb_device_handle *handle_compact_pro = libusb_open_device_with_vid_pid(
      nullptr, SEEK_COMPACT_PRO_VENDOR_ID, SEEK_COMPACT_PRO_PRODUCT_ID);
  const bool has_seek_compact = handle_compact != nullptr;
  const bool has_seek_compact_pro = handle_compact_pro != nullptr;
  // Close libusb handles after we don't need them anymore
  if (handle_compact) libusb_close(handle_compact);
  if (handle_compact_pro) libusb_close(handle_compact_pro);

  // todo: this will need to be pulled from the config, we may end up running
  // these from the camera service so that
  //       it can see the camera settings, which are not visible to
  //       openhd-system early at boot
  std::string model;
  std::string fps;

  if (has_seek_compact) {
    openhd::log::get_default()->debug("Found seek_compact");
    model = "seek";
    fps = "7";
  }

  if (has_seek_compact_pro) {
    openhd::log::get_default()->debug("Found seek_compact_pro");
    model = "seekpro";
    // todo: this is not necessarily accurate, not all compact pro models are
    // 15hz
    fps = "15";
  }

  if (has_seek_compact || has_seek_compact_pro) {
    openhd::log::get_default()->debug("Found seek_compact / seek_compact_pro");
    std::stringstream ss;
    // todo: this should be more dynamic and allow for multiple cameras
    ss << "DeviceNode=/dev/video4";
    ss << std::endl;
    ss << "SeekModel=";
    ss << model;
    ss << std::endl;
    ss << "FPS=";
    ss << fps;
    ss << std::endl;
    ss << "SeekColormap=11";
    ss << std::endl;
    ss << "SeekRotate=11";
    ss << std::endl;
    OHDFilesystemUtil::create_directories("/etc/openhd");
    OHDFilesystemUtil::write_file("/etc/openhd/seekthermal.conf", ss.str());

    std::vector<std::string> ar{"start", "seekthermal"};
    OHDUtil::run_command("systemctl", ar);
  }
#endif  // OPENHD_LIBUSB_PRESENT
}
}  // namespace DThermalCamerasHelper

#ifdef ENABLE_USB_CAMERAS
/**
 * Try and break out some of the stuff from stephen.
 * Even though it mght not be re-used in multiple places, it makes the code more
 * readable in my opinion.
 */
namespace openhd::v4l2 {
/**
 * Search for all v4l2 video devices, that means devices named /dev/videoX where
 * X=0,1,...
 * @return list of all the devices that have the above name scheme.
 */
static std::vector<int> findV4l2VideoDevices() {
  const auto paths =
      OHDFilesystemUtil::getAllEntriesFullPathInDirectory("/dev");
  std::vector<int> ret;
  const std::regex r{"/dev/video([\\d]+)"};
  for (const auto &path : paths) {
    std::smatch result;
    if (!std::regex_search(path, result, r)) {
      continue;
    }
    // openhd::log::get_default()->debug("XX {}
    // {}",result[0].str(),result[1].str());
    if (result.size() >= 2) {
      const int v4l2_number = std::stoi(result[1].str());
      ret.push_back(v4l2_number);
    }
  }
  std::sort(ret.begin(), ret.end());
  return ret;
}

// Util so we can't forget to close the fd
class V4l2FPHolder {
 public:
  V4l2FPHolder(const std::string &node, const OHDPlatform &platform) {
    // fucking hell, on jetson v4l2_open seems to be bugged
    // https://forums.developer.nvidia.com/t/v4l2-open-create-core-with-jetpack-4-5-or-later/170624/6
    if (false) {  // platform_type==PlatformType::Jetson
      fd = open(node.c_str(), O_RDWR | O_NONBLOCK, 0);
    } else {
      fd = v4l2_open(node.c_str(), O_RDWR);
    }
  }
  ~V4l2FPHolder() {
    if (fd != -1) {
      v4l2_close(fd);
    }
  }
  [[nodiscard]] bool opened_successfully() const { return fd != -1; }
  int fd;
};

// Stephen already wrote the parsing for this info, even though it is not really
// needed I keep it anyways
struct Udevaddm_info {
  std::string id_model = "unknown";
  std::string id_vendor = "unknown";
};
Udevaddm_info get_udev_adm_info(const std::string &v4l2_device,
                                std::shared_ptr<spdlog::logger> &m_console) {
  Udevaddm_info ret{};
  const auto udev_info_opt =
      OHDUtil::run_command_out(fmt::format("udevadm info {}", v4l2_device));
  if (udev_info_opt == std::nullopt) {
    m_console->debug("udev_info no result");
    return {};
  }
  const auto &udev_info = udev_info_opt.value();
  // check for device name
  std::smatch model_result;
  const std::regex model_regex{"ID_MODEL=([\\w]+)"};
  if (std::regex_search(udev_info, model_result, model_regex)) {
    if (model_result.size() == 2) {
      ret.id_model = model_result[1];
    }
  }
  // check for device vendor
  std::smatch vendor_result;
  const std::regex vendor_regex{"ID_VENDOR=([\\w]+)"};
  if (std::regex_search(udev_info, vendor_result, vendor_regex)) {
    if (vendor_result.size() == 2) {
      ret.id_vendor = vendor_result[1];
    }
  }
  return ret;
}

static std::string v4l2_capability_to_string(const v4l2_capability caps) {
  return fmt::format("driver:{},bus_info:{}", (const char *)caps.driver,
                     (const char *)caps.bus_info);
}

static std::optional<v4l2_capability> get_capabilities(
    std::unique_ptr<openhd::v4l2::V4l2FPHolder> &v4l2_fp_holder) {
  struct v4l2_capability caps = {};
  if (ioctl(v4l2_fp_holder->fd, VIDIOC_QUERYCAP, &caps) == -1) {
    return std::nullopt;
  }
  return caps;
}

struct EndpointFormat {
  // pixel format as string, never empty
  std::string format;
  int width;
  int height;
  int fps;
  std::string debug() const {
    return fmt::format("{}|{}x{}@{}", format, width, height, fps);
  }
};
struct EndpointFormats {
  // These are the 3 (already encoded) formats openhd understands
  std::vector<EndpointFormat> formats_h264;
  std::vector<EndpointFormat> formats_h265;
  std::vector<EndpointFormat> formats_mjpeg;
  // anything other (raw) we pack into a generic bucket
  std::vector<EndpointFormat> formats_raw;
  bool has_any_valid_format = false;
};

// Enumerate all the ("pixel formats") we are after for a given v4l2 device
static EndpointFormats iterate_supported_outputs(
    std::unique_ptr<openhd::v4l2::V4l2FPHolder> &v4l2_fp_holder) {
  auto m_console = openhd::log::get_default();
  EndpointFormats ret{};

  struct v4l2_fmtdesc fmtdesc {};
  memset(&fmtdesc, 0, sizeof(fmtdesc));
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  while (ioctl(v4l2_fp_holder->fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
    struct v4l2_frmsizeenum frmsize {};
    frmsize.pixel_format = fmtdesc.pixelformat;
    frmsize.index = 0;
    while (ioctl(v4l2_fp_holder->fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
      struct v4l2_frmivalenum frmival {};
      if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
        frmival.index = 0;
        frmival.pixel_format = fmtdesc.pixelformat;
        frmival.width = frmsize.discrete.width;
        frmival.height = frmsize.discrete.height;
        while (ioctl(v4l2_fp_holder->fd, VIDIOC_ENUM_FRAMEINTERVALS,
                     &frmival) == 0) {
          if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            EndpointFormat endpoint_format;
            endpoint_format.format =
                fmt::format("{}", (const char *)fmtdesc.description);
            endpoint_format.width = frmsize.discrete.width;
            endpoint_format.height = frmsize.discrete.height;
            endpoint_format.fps = frmival.discrete.denominator;
            // m_console->debug("{}", endpoint_format.debug());
            ret.has_any_valid_format = true;
            if (fmtdesc.pixelformat == V4L2_PIX_FMT_H264) {
              ret.formats_h264.push_back(endpoint_format);
            }
#if defined V4L2_PIX_FMT_H265
            else if (fmtdesc.pixelformat == V4L2_PIX_FMT_H265) {
              ret.formats_h265.push_back(endpoint_format);
            }
#endif
            else if (fmtdesc.pixelformat == V4L2_PIX_FMT_MJPEG) {
              ret.formats_mjpeg.push_back(endpoint_format);
            } else {
              // if it supports something else we assume it's one of the raw
              // formats, being specific here is too complicated
              ret.formats_raw.push_back(endpoint_format);
            }
          }
          frmival.index++;
        }
      }
      frmsize.index++;
    }
    fmtdesc.index++;
  }
  return ret;
}

/**
 * Helper for checking if a v4l2 device can output any of the supported endpoint
 * format(s). Returns std::nullopt if this device cannot do h264,h265,mjpeg or
 * RAW out.
 */
struct XValidEndpoint {
  v4l2_capability caps;
  openhd::v4l2::EndpointFormats formats;
};
static std::optional<XValidEndpoint> probe_v4l2_device(
    const OHDPlatform platform_tpye, std::shared_ptr<spdlog::logger> &m_console,
    const std::string &device_node) {
  auto v4l2_fp_holder =
      std::make_unique<openhd::v4l2::V4l2FPHolder>(device_node, platform_tpye);
  if (!v4l2_fp_holder->opened_successfully()) {
    m_console->debug("Can't open {}", device_node);
    return std::nullopt;
  }
  const auto caps_opt = openhd::v4l2::get_capabilities(v4l2_fp_holder);
  if (!caps_opt) {
    m_console->debug("Can't get caps for {}", device_node);
    return std::nullopt;
  }
  const auto caps = caps_opt.value();
  const auto supported_formats =
      openhd::v4l2::iterate_supported_outputs(v4l2_fp_holder);
  if (supported_formats.has_any_valid_format) {
    return XValidEndpoint{caps, supported_formats};
  }
  return std::nullopt;
}

}  // namespace openhd::v4l2

std::vector<DCameras::DiscoveredUSBCamera> DCameras::detect_usb_cameras(
    std::shared_ptr<spdlog::logger> &m_console, bool debug) {
  const auto platform = OHDPlatform::instance();
  if (platform.is_rpi_or_x86()) {
    DThermalCamerasHelper::enableFlirIfFound();
    DThermalCamerasHelper::enableSeekIfFound();
  }
  std::vector<DCameras::DiscoveredUSBCamera> ret;
  const auto devices = openhd::v4l2::findV4l2VideoDevices();
  if (debug) {
    m_console->debug("Found {} v4l2 devices", devices.size());
    for (auto &device : devices) m_console->debug("Device:{}", device);
  }
  for (const auto &device : devices) {
    const auto v4l2_device_name = get_v4l2_device_name_string(device);
    const auto probed_opt =
        openhd::v4l2::probe_v4l2_device(platform, m_console, v4l2_device_name);
    if (!probed_opt.has_value()) {
      continue;
    }
    const auto &probed = probed_opt.value();
    const std::string bus((char *)probed.caps.bus_info);
    const std::string driver((char *)probed.caps.driver);
    if (debug) {
      m_console->debug("V4l2 info {} {} {}", device, bus,
                       probed.formats.formats_raw.size());
    }
    if (!probed.formats.formats_raw.empty()) {
      // (RAW) format usb camera candidate
      bool found = false;
      for (auto &tmp : ret) {
        if (tmp.bus == bus) {
          found = true;
        }
      }
      if (!found) {
        ret.push_back(DCameras::DiscoveredUSBCamera{
            .bus = bus, .v4l2_device_number = device});
      } else {
      }
    }
  }
  if (debug) {
    for (const auto &usb_cam : ret) {
      m_console->debug("Found USB cam [{}]-[{}]", usb_cam.bus,
                       usb_cam.v4l2_device_number);
    }
  }
  return ret;
}

#endif