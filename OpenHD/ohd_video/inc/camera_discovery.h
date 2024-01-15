#ifndef CAMERA_H
#define CAMERA_H

#include <array>
#include <chrono>
#include <vector>

#include "camera_holder.h"
#include "openhd_platform.h"
#include "openhd_spdlog.h"

/**
 * Discover all connected cameras and for some camera(s) (E.g. USB cameras and/or cameras that use v4l2)
 * Figure out their capabilities via V4l2. Written as a class r.n but actually should only be a namespace.
 * The interesting bit is just the discover() method below.
 */
class DCameras {
 public:
  /**
   * Discover all cameras connected to this system.
   * @returns A list of detected cameras, or an empty vector if no cameras have been found.
   * Note that at this point, we haven't performed the settings lookup for the Camera(s) - this just exposes the available cameras
   * and their capabilities.
   * @param ohdPlatform the platform we are running on, detection depends on the platform type.
   */
  //static std::vector<Camera> discover(OHDPlatform ohdPlatform);
  //static std::vector<Camera> discover2(const OHDPlatform& platform);
  /**
   * NOTE: Some of the CSI camera(s) could also be accessed / detected via v4l2, but there are
   * some downsides to that, which is why for example we use libcamera directly to discover libcamera-CSI
   * cameras, and not v4l2.
   */
  /**
   * This is used when the gpu firmware is in charge of the camera, we have to
   * ask it.
   */
  //static std::vector<Camera> detect_raspberrypi_broadcom_csi(std::shared_ptr<spdlog::logger>& m_console);

  /*
   * NOTE: Even though libcamera's goal is to work on all linux device(s) with all camera(s) it is only usefully
   * on raspberry pi and for those rpi camera(s) that work on it - e.g. original rpi and arducam camera(s).
   */
  //7static std::vector<Camera> detect_raspberrypi_libcamera_csi(std::shared_ptr<spdlog::logger>& m_console);

  // hacky
  //static std::vector<Camera> detect_rapsberrypi_veye_v4l2_dirty(std::shared_ptr<spdlog::logger>& m_console);

  /**
   *  Detect allwinner CSI camera(s). Uses v4l2, but needs a few tweaks.
   */
  //static std::vector<Camera> detect_allwinner_csi(std::shared_ptr<spdlog::logger>& m_console);

  /**
   *  Detect rockchip CSI camera(s). Uses v4l2, but needs a few tweaks.
   */
  //static std::vector<Camera> detect_rockchip_csi(std::shared_ptr<spdlog::logger>& m_console);

  /**
   * Detect jetson CSI camera(s). Uses v4l2. dirty
   */
   //static std::vector<Camera> detect_jetson_csi(std::shared_ptr<spdlog::logger>& m_console);

   /**
    * NOTE: r.n only for USB UVC and UVCH264 camera(s)
    */
   //static std::vector<Camera> detect_usb_cameras(const OHDPlatform& platform,std::shared_ptr<spdlog::logger>& m_console);

   struct DiscoveredUSBCamera{
       std::string bus;
       std::string device_name;
   };
   static std::vector<DiscoveredUSBCamera> detect_usb_cameras(const OHDPlatform& platform,std::shared_ptr<spdlog::logger>& m_console);

  // NOTE: IP cameras cannot be auto detected !
};

#endif
