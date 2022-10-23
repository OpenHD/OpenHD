//
// Created by consti10 on 24.10.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_H_

#include "openhd-util.hpp"

// Helper to reconfigure the rpi os for the different camera types
namespace openhd::rpi::os{
static void OHDRpiConfigClear()
{
  OHDUtil::run_command("sed -i '/camera_auto_detect=1/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/dtoverlay=vc4-kms-v3d/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/dtoverlay=vc4-fkms-v3d/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/start_x=1/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/enable_uart=1/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/dtoverlay=arducam-pivariety/d' /boot/config.txt",{});
};

static void OHDRpiConfigExecute()
{
  OHDUtil::run_command("rm -Rf /boot/OpenHD/libcamera.txt",{});
  OHDUtil::run_command("rm -Rf /boot/OpenHD/raspicamsrc.txt",{});
  OHDUtil::run_command("rm -Rf /boot/OpenHD/arducam.txt",{});
  OHDUtil::run_command("echo",{"This device will now reboot to enable configs"});
  OHDUtil::run_command("reboot",{});
};

static void OHDRpiConfigLibcamera()
{
  OHDUtil::run_command("sed -i '$ a camera_auto_detect=1' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a dtoverlay=vc4-kms-v3d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a enable_uart=1' /boot/config.txt",{});
};

static void OHDRpiConfigRaspicam()
{
  OHDUtil::run_command("sed -i '$ a start_x=1' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a dtoverlay=vc4-fkms-v3d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a enable_uart=1' /boot/config.txt",{});
};

static void OHDRpiConfigArducam()
{
  OHDUtil::run_command("sed -i '$ a dtoverlay=arducam-pivariety' /boot/config.txt",{});
};
}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_H_
