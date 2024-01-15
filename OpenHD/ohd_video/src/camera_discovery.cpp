#include "camera_discovery.h"

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <libusb.h>

#include <iostream>
#include <regex>

#include "camera2.h"
#include "libcamera_detect.hpp"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

// annoying linux platform specifics
#ifndef V4L2_PIX_FMT_H265
#define V4L2_PIX_FMT_H265 V4L2_PIX_FMT_HEVC
#endif

/*std::vector<Camera> DCameras::discover(const OHDPlatform platform) {
  discover2(platform);
  auto m_console=openhd::log::create_or_get("v_dcameras");
  assert(m_console);
  auto m_enable_debug=OHDUtil::get_ohd_env_variable_bool("OHD_DISCOVER_CAMERAS_DEBUG");
  // always enabled for now
  // TODO fixme
  if(m_enable_debug){
    m_console->set_level(spdlog::level::debug);
    m_console->debug("m_enable_debug=true");
  }
  m_console->debug("discover_internal()");
  std::vector<Camera> cameras;
  // Only on raspberry pi with the old broadcom stack we need a special detection method for the rpi CSI camera.
  // On all other platforms (for example jetson) the CSI camera is exposed as a normal V4l2 linux device,and we cah
  // check the driver if it is actually a CSI camera handled by nvidia.
  // Note: With libcamera, also the rpi will do v4l2 for cameras.
  if(platform.platform_type==PlatformType::RaspberryPi){
    // Detect RPI CSI Camera(s).
    // We can do mmal, libcamera and veye v4l2
    const auto rpi_broadcom_csi_cams=detect_raspberrypi_broadcom_csi(m_console);
    m_console->debug("RPI MMAL CSI Cameras:{}",rpi_broadcom_csi_cams.size());
    OHDUtil::vec_append(cameras,rpi_broadcom_csi_cams);
    const auto rpi_veye_csi_cams= detect_rapsberrypi_veye_v4l2_dirty(m_console);
    m_console->debug("RPI Veye V4l2 CSI Cameras:{}",rpi_veye_csi_cams.size());
    OHDUtil::vec_append(cameras,rpi_veye_csi_cams);
    if(rpi_veye_csi_cams.empty()){
      // NOTE: See the log below why we only detect "libcamera camera(s)" if there are no veye cameras found.
      const auto rpi_libcamera_csi_cams=detect_raspberrypi_libcamera_csi(m_console);
      OHDUtil::vec_append(cameras,rpi_libcamera_csi_cams);
    }else{
      m_console->warn("Skipping libcamera detect, since it might pick up a veye cam by accident even though it cannot do it");
    }
  }else if(platform.platform_type == PlatformType::Allwinner){
    auto tmp=detect_allwinner_csi(m_console);
    OHDUtil::vec_append(cameras,tmp);
    }else if(platform.platform_type == PlatformType::Rockchip){
    auto tmp=detect_rockchip_csi(m_console);
    OHDUtil::vec_append(cameras,tmp);
  }else if(platform.platform_type == PlatformType::Jetson){
    auto tmp=detect_jetson_csi(m_console);
    OHDUtil::vec_append(cameras,tmp);
  }
  // Allwinner 3.4 kernel v4l2 implementation is so sketchy that probing it can stop it working.
  if(platform.platform_type != PlatformType::Allwinner){
    // I think these need to be run before the detectv4l2 ones, since they are then picked up just like a normal v4l2 camera ??!!
    // Will need custom debugging before anything here is usable again though.
    DThermalCamerasHelper::enableFlirIfFound();
    DThermalCamerasHelper::enableSeekIfFound();
    // NOTE: be carefully to not detect camera(s) twice
    auto usb_cameras= detect_usb_cameras(platform,m_console);
    m_console->debug("N USB Camera(s): {}",usb_cameras.size());
    OHDUtil::vec_append(cameras,usb_cameras);
  }
  for(int i=0;i<cameras.size();i++){
    cameras[i].index=i;
  }
  // write to json for debugging
  write_camera_manifest(cameras);
  return cameras;
}

std::vector<Camera> DCameras::detect_raspberrypi_broadcom_csi(std::shared_ptr<spdlog::logger>& m_console) {
  m_console->debug("detect_raspberrypi_broadcom_csi()");
  std::vector<Camera> ret;
  const auto vcgencmd_result=OHDUtil::run_command_out("vcgencmd get_camera");
  if(vcgencmd_result==std::nullopt){
    m_console->debug("detect_raspberrypi_broadcom_csi() vcgencmd not found");
    return {};
  }
  const auto& raw_value=vcgencmd_result.value();
  std::smatch result;
  // example "supported=2 detected=2"
  const std::regex r{R"(supported=([\d]+)\s+detected=([\d]+))"};
  if (!std::regex_search(raw_value, result, r)) {
    m_console->debug("detect_raspberrypi_broadcom_csi() no regex match");
    return {};
  }
  if (result.size() != 3) {
    m_console->debug("detect_raspberrypi_broadcom_csi() regex unexpected result");
    return {};
  }
  const std::string supported = result[1];
  const std::string detected = result[2];
  m_console->debug("detect_raspberrypi_broadcom_csi() supported={} detected={}",supported,detected);
  const auto camera_count=OHDUtil::string_to_int(detected);
  if (camera_count >= 1) {
    // dirty, check for CSI to HDMI adapter
    bool is_csi_to_hdmi=false;
    const auto csi_to_hdmi_check=OHDUtil::run_command_out("i2cdetect -y 10 0x0f 0x0f");
    if(csi_to_hdmi_check.has_value()){
      if(OHDUtil::contains(csi_to_hdmi_check.value(),"0f")){
        m_console->debug("detected hdmi to csi instead of rpi mmal csi cam");
        is_csi_to_hdmi= true;
      }
    }
    if(is_csi_to_hdmi){
      Camera camera;
      camera.name = "Pi_CSI_HDMI_0";
      camera.vendor = "RaspberryPi";
      camera.type = CameraType::RPI_CSI_MMAL;
      camera.bus = "0";
      camera.rpi_csi_mmal_is_csi_to_hdmi= true;
      ret.push_back(camera);
    }else{
      Camera camera;
      camera.name = "Pi_CSI_0";
      camera.vendor = "RaspberryPi";
      camera.type = CameraType::RPI_CSI_MMAL;
      camera.bus = "0";
      ret.push_back(camera);
    }
  }
  if (camera_count >= 2) {
    Camera camera;
    camera.name = "Pi_CSI_1";
    camera.vendor = "RaspberryPi";
    camera.type = CameraType::RPI_CSI_MMAL;
    camera.bus = "1";
    ret.push_back(camera);
  }
  return ret;
}

std::vector<Camera> DCameras::detect_allwinner_csi(std::shared_ptr<spdlog::logger>& m_console) {
  m_console->debug("detect_allwinner_csi(");
  if(OHDFilesystemUtil::exists("/dev/video0")){
    m_console->debug("Camera set as Allwinner_CSI_0");
    Camera camera;
    camera.name = "Allwinner_CSI_0";
    camera.vendor = "Allwinner";
    camera.type = CameraType::ALLWINNER_CSI;
    camera.bus = "0";
    return {camera};
  }
  return {};
}

std::vector<Camera> DCameras::detect_rockchip_csi(std::shared_ptr<spdlog::logger>& m_console) {
  m_console->debug("detect_rockchip_csi(");
  if(OHDFilesystemUtil::exists("/dev/video11")){
    m_console->debug("Camera set as Rockchip_CSI_IMX415");
    Camera camera;
    camera.name = "Rockchip_CSI_11";
    camera.vendor = "Rockchip";
    camera.type = CameraType::ROCKCHIP_CSI;
    camera.bus = "11";
    return {camera};
  }
  return {};
}

std::vector<Camera> DCameras::detect_rapsberrypi_veye_v4l2_dirty(std::shared_ptr<spdlog::logger>& m_console) {
  m_console->debug("detect_rapsberrypi_veye_v4l2_dirty");
  std::vector<Camera> ret{};
  const auto devices = openhd::v4l2::findV4l2VideoDevices();
  for (const auto &device: devices) {
    // dirty, but works
    // We are only interested in veye camera(s), so first get rid of anything not using rpi unicam
    auto v4l2_fp_holder=std::make_unique<openhd::v4l2::V4l2FPHolder>(device,PlatformType::RaspberryPi);
    if(!v4l2_fp_holder->opened_successfully()){
      continue;
    }
    const auto caps_opt=openhd::v4l2::get_capabilities(v4l2_fp_holder);
    if(!caps_opt){
      continue;
    }
    const auto caps=caps_opt.value();
    if(!OHDUtil::contains(std::string((char*)caps.driver),"unicam")){
      continue;
    }
    v4l2_fp_holder.reset();
    // now check if it is one of the known veye cameras
    const auto v4l2_info_video0_opt=OHDUtil::run_command_out(fmt::format("v4l2-ctl --info --device {}",device));
    if(!v4l2_info_video0_opt.has_value()){
      continue;
    }
    const auto& v4l2_info_video0=v4l2_info_video0_opt.value();
    const bool is_veye=OHDUtil::contains(v4l2_info_video0,"veye327") || OHDUtil::contains(v4l2_info_video0,"csimx307") || OHDUtil::contains(v4l2_info_video0,"veyecam2m");
    if(is_veye){
      Camera camera;
      camera.type=CameraType::RPI_CSI_VEYE_V4l2;
      camera.bus=device;
      camera.index=0;
      camera.name = fmt::format("Pi_VEYE_{}",ret.size());
      camera.vendor = "VEYE";
      ret.push_back(camera);
    }
  }
  return ret;
}

#ifdef OPENHD_LIBCAMERA_PRESENT
std::vector<Camera> DCameras::detect_raspberrypi_libcamera_csi(std::shared_ptr<spdlog::logger>& m_console) {
  std::vector<Camera> ret;
  m_console->debug("detect_raspberry_libcamera()");
  auto cameras = openhd::libcameradetect::get_csi_cameras();
  m_console->debug("Libcamera:discovered {} cameras",cameras.size());
  for (const auto& camera : cameras) {
    // TODO: filter out other cameras
    ret.push_back(camera);
  }
  return ret;
}
#else
std::vector<Camera> DCameras::detect_raspberrypi_libcamera_csi(std::shared_ptr<spdlog::logger>& m_console) {
  m_console->warn("detect_raspberry_libcamera - built without libcamera, libcamera features unavailable");
  return {};
}
#endif

std::vector<Camera> DCameras::detect_jetson_csi(std::shared_ptr<spdlog::logger> &m_console) {
  const auto devices=openhd::v4l2::findV4l2VideoDevices();
  for(const auto& device:devices){
    auto v4l2_fp_holder=std::make_unique<openhd::v4l2::V4l2FPHolder>(device,PlatformType::Jetson);
    if(!v4l2_fp_holder->opened_successfully()){
      continue;
    }
    const auto caps_opt= get_capabilities(v4l2_fp_holder);
    if(!caps_opt){
      continue;
    }
    const auto caps=caps_opt.value();
    const std::string driver((char *)caps.driver);
    if(driver=="tegra-video"){
      m_console->debug("Found Jetson CSI camera");
      Camera camera;
      camera.type=CameraType::JETSON_CSI;
      camera.bus="0";
      camera.index=0;
      camera.name = "JETSON_CSI_0";
      camera.vendor = "NVIDIA";
      return {camera};
    }
  }
  return {};
}

std::vector<Camera> DCameras::detect_usb_cameras(const OHDPlatform& platform,std::shared_ptr<spdlog::logger>& m_console) {
  std::vector<Camera> ret{};
  const auto devices = openhd::v4l2::findV4l2VideoDevices();
  for (const auto &device: devices) {
    const auto probed_opt= openhd::v4l2::probe_v4l2_device(platform.platform_type,m_console,device);
    if(!probed_opt.has_value()){
      continue;
    }
    const auto& probed=probed_opt.value();
    const std::string bus((char *)probed.caps.bus_info);
    const std::string driver((char *)probed.caps.driver);
    CameraType camera_type=CameraType::UNKNOWN;
    if(driver=="uvcvideo"){
      camera_type=CameraType::UVC;
    }else if (driver == "v4l2 loopback") {
      m_console->warn("V4l2-loopback - skipping,");
      continue;
    }else{
      m_console->debug("Unknown driver type {}",driver);
      continue;
    }
    CameraEndpointV4l2 endpoint;
    endpoint.v4l2_device_node=device;
    endpoint.bus=bus;
    endpoint.formats_h264=probed.formats.formats_h264;
    endpoint.formats_h265=probed.formats.formats_h265;
    endpoint.formats_mjpeg=probed.formats.formats_mjpeg;
    endpoint.formats_raw=probed.formats.formats_raw;
    // Find either an already existing cam with this bus or create a new one
    bool found=false;
    for(auto& camera:ret){
      if(camera.bus==bus){
        found= true;
        camera.v4l2_endpoints.push_back(endpoint);
        m_console->debug("Adding endpoint {} to already existing camera",endpoint.v4l2_device_node);
      }
    }
    if(!found){
      Camera camera{};
      camera.type=camera_type;
      camera.bus=endpoint.bus;
      const auto udevadm_info=openhd::v4l2::get_udev_adm_info(device,m_console);
      camera.name=udevadm_info.id_model;
      camera.vendor=udevadm_info.id_vendor;
      camera.v4l2_endpoints.push_back(endpoint);
      m_console->debug("Adding new camera {}:{} for endpoint {}", camera_type_to_string(camera_type),camera.name,endpoint.v4l2_device_node);
      ret.push_back(camera);
    }
  }
  return ret;
}

static constexpr auto PRIMARY_CAM_TYPE_FILENAME="usr/local/share/openhd/video/cam0_type.txt";
static constexpr auto SECONDARY_CAM_TYPE_FILENAME="usr/local/share/openhd/video/cam1_type.txt";
static int read_cam_type(bool secondary){
    auto filename= secondary ? SECONDARY_CAM_TYPE_FILENAME : PRIMARY_CAM_TYPE_FILENAME;
    auto cam_type_opt=OHDFilesystemUtil::read_int_from_file(filename);
    if(cam_type_opt.has_value() && x_is_valid_cam_type(cam_type_opt.value())){
        return cam_type_opt.value();
    }
    int cam_type=secondary ? X_CAM_TYPE_DISABLED : X_CAM_TYPE_DUMMY_SW;
    OHDFilesystemUtil::write_file(OHDUtil::int_as_string(cam_type),filename);
    return cam_type;
}

std::vector<Camera> DCameras::discover2(const OHDPlatform &platform) {
    const auto cam0_type= read_cam_type(false);
    const auto cam1_type= read_cam_type(true);
    return {};
}*/

struct EndpointFormat{
    // pixel format as string, never empty
    std::string format;
    int width;
    int height;
    int fps;
    std::string debug()const{
        return fmt::format("{}|{}x{}@{}",format,width,height,fps);
    }
};
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
        libusb_context *context = nullptr;
        int result = libusb_init(&context);
        if (result) {
            openhd::log::get_default()->warn("Failed to initialize libusb");
            return;
        }
        libusb_device_handle *handle = libusb_open_device_with_vid_pid(
                nullptr, FLIR_ONE_VENDOR_ID, FLIR_ONE_PRODUCT_ID);
        if(!handle)return;
        // Close libusb handles after we don't need them anymore
        libusb_close(handle);
        // TODO missing r.n
        OHDUtil::run_command("systemctl", {"start", "flirone"});
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
        const bool has_seek_compact=handle_compact != nullptr;
        const bool has_seek_compact_pro=handle_compact_pro != nullptr;
        // Close libusb handles after we don't need them anymore
        if(handle_compact)libusb_close(handle_compact);
        if(handle_compact_pro) libusb_close(handle_compact_pro);

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
            OHDFilesystemUtil::write_file("/etc/openhd/seekthermal.conf",ss.str());

            std::vector<std::string> ar{"start", "seekthermal"};
            OHDUtil::run_command("systemctl", ar);
        }
    }
}  // namespace DThermalCamerasHelper


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
    static std::vector<std::string> findV4l2VideoDevices() {
        const auto paths =
                OHDFilesystemUtil::getAllEntriesFullPathInDirectory("/dev");
        std::vector<std::string> ret;
        const std::regex r{"/dev/video([\\d]+)"};
        for (const auto &path : paths) {
            std::smatch result;
            if (!std::regex_search(path, result, r)) {
                continue;
            }
            ret.push_back(path);
        }
        return ret;
    }

// Util so we can't forget to close the fd
    class V4l2FPHolder{
    public:
        V4l2FPHolder(const std::string &node,const PlatformType& platform_type){
            // fucking hell, on jetson v4l2_open seems to be bugged
            // https://forums.developer.nvidia.com/t/v4l2-open-create-core-with-jetpack-4-5-or-later/170624/6
            if(platform_type==PlatformType::Jetson){
                fd = open(node.c_str(), O_RDWR | O_NONBLOCK, 0);
            }else{
                fd = v4l2_open(node.c_str(), O_RDWR);
            }
        }
        ~V4l2FPHolder(){
            if(fd!=-1){
                v4l2_close(fd);
            }
        }
        [[nodiscard]] bool opened_successfully() const{
            return fd!=-1;
        }
        int fd;
    };

// Stephen already wrote the parsing for this info, even though it is not really needed
// I keep it anyways
    struct Udevaddm_info {
        std::string id_model="unknown";
        std::string id_vendor="unknown";
    };
    Udevaddm_info get_udev_adm_info(const std::string& v4l2_device,std::shared_ptr<spdlog::logger>& m_console){
        Udevaddm_info ret{};
        const auto udev_info_opt=OHDUtil::run_command_out(fmt::format("udevadm info {}",v4l2_device));
        if(udev_info_opt==std::nullopt){
            m_console->debug("udev_info no result");
            return {};
        }
        const auto& udev_info=udev_info_opt.value();
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

    static std::string v4l2_capability_to_string(const v4l2_capability caps){
        return fmt::format("driver:{},bus_info:{}",(const char*)caps.driver,(const char*)caps.bus_info);
    }

    static std::optional<v4l2_capability> get_capabilities(std::unique_ptr<openhd::v4l2::V4l2FPHolder>& v4l2_fp_holder){
        struct v4l2_capability caps = {};
        if (ioctl(v4l2_fp_holder->fd, VIDIOC_QUERYCAP, &caps) == -1) {
            return std::nullopt;
        }
        return caps;
    }

    struct EndpointFormats{
        // These are the 3 (already encoded) formats openhd understands
        std::vector<EndpointFormat> formats_h264;
        std::vector<EndpointFormat> formats_h265;
        std::vector<EndpointFormat> formats_mjpeg;
        // anything other (raw) we pack into a generic bucket
        std::vector<EndpointFormat> formats_raw;
        bool has_any_valid_format=false;
    };
// Enumerate all the ("pixel formats") we are after for a given v4l2 device
    static EndpointFormats iterate_supported_outputs(std::unique_ptr<openhd::v4l2::V4l2FPHolder>& v4l2_fp_holder){
        auto m_console=openhd::log::get_default();
        EndpointFormats ret{};

        struct v4l2_fmtdesc fmtdesc{};
        memset(&fmtdesc, 0, sizeof(fmtdesc));
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while (ioctl(v4l2_fp_holder->fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
            struct v4l2_frmsizeenum frmsize{};
            frmsize.pixel_format = fmtdesc.pixelformat;
            frmsize.index = 0;
            while (ioctl(v4l2_fp_holder->fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
                struct v4l2_frmivalenum frmival{};
                if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                    frmival.index = 0;
                    frmival.pixel_format = fmtdesc.pixelformat;
                    frmival.width = frmsize.discrete.width;
                    frmival.height = frmsize.discrete.height;
                    while (ioctl(v4l2_fp_holder->fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
                        if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                            EndpointFormat endpoint_format;
                            endpoint_format.format = fmt::format("{}", (const char*)fmtdesc.description);
                            endpoint_format.width = frmsize.discrete.width;
                            endpoint_format.height = frmsize.discrete.height;
                            endpoint_format.fps = frmival.discrete.denominator;
                            //m_console->debug("{}", endpoint_format.debug());
                            ret.has_any_valid_format= true;
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
                                // if it supports something else we assume it's one of the raw formats, being specific here is too complicated
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
 * Helper for checking if a v4l2 device can output any of the supported endpoint format(s).
 * Returns std::nullopt if this device cannot do h264,h265,mjpeg or RAW out.
 */
    struct XValidEndpoint{
        v4l2_capability caps;
        openhd::v4l2::EndpointFormats formats;
    };
    static std::optional<XValidEndpoint> probe_v4l2_device(const PlatformType platform_tpye,std::shared_ptr<spdlog::logger>& m_console,const std::string& device_node){
        auto v4l2_fp_holder=std::make_unique<openhd::v4l2::V4l2FPHolder>(device_node,platform_tpye);
        if(!v4l2_fp_holder->opened_successfully()){
            m_console->debug("Can't open {}",device_node);
            return std::nullopt;
        }
        const auto caps_opt=openhd::v4l2::get_capabilities(v4l2_fp_holder);
        if(!caps_opt){
            m_console->debug("Can't get caps for {}",device_node);
            return std::nullopt;
        }
        const auto caps=caps_opt.value();
        const auto supported_formats=openhd::v4l2::iterate_supported_outputs(v4l2_fp_holder);
        if(supported_formats.has_any_valid_format){
            return XValidEndpoint{caps,supported_formats};
        }
        return std::nullopt;
    }

}  // namespace openhd::v4l2

std::vector<DCameras::DiscoveredUSBCamera>
DCameras::detect_usb_cameras(const OHDPlatform &platform, std::shared_ptr<spdlog::logger> &m_console) {
    if(platform.platform_type==PlatformType::RaspberryPi || platform.platform_type==PlatformType::PC){
        DThermalCamerasHelper::enableFlirIfFound();
        DThermalCamerasHelper::enableSeekIfFound();
    }
    std::vector<DCameras::DiscoveredUSBCamera> ret;
    const auto devices = openhd::v4l2::findV4l2VideoDevices();
    for (const auto &device: devices) {
        const auto probed_opt= openhd::v4l2::probe_v4l2_device(platform.platform_type,m_console,device);
        if(!probed_opt.has_value()){
            continue;
        }
        const auto& probed=probed_opt.value();
        const std::string bus((char *)probed.caps.bus_info);
        const std::string driver((char *)probed.caps.driver);
        if(probed.formats.formats_raw.size()>0){
            // (RAW) format usb camera candidate
            bool found= false;
            for(auto& tmp:ret){
                if(tmp.bus==bus){
                    found= true;
                }
            }
            if(!found){
                ret.push_back(DCameras::DiscoveredUSBCamera{.bus=bus,.device_name=device});
            }
        }
    }
    return ret;
}
