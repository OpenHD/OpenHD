//
// Created by consti10 on 24.10.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_HPP_

#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"

// Helper to reconfigure the rpi os for the different camera types
namespace openhd::rpi::os{

static void OHDRpiConfigClear(){
  OHDUtil::run_command("sed -i '/camera_auto_detect=1/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/camera_auto_detect=0/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/dtoverlay=vc4-kms-v3d,cma-128/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/dtoverlay=vc4-fkms-v3d,cma-128/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/dtoverlay=imx477/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/start_x=1/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/enable_uart=1/d' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '/dtoverlay=arducam-pivariety/d' /boot/config.txt",{});
};

static void OHDRpiConfigLibcamera(){
  OHDUtil::run_command("sed -i '$ a camera_auto_detect=1' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a dtoverlay=vc4-kms-v3d,cma-128' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a enable_uart=1' /boot/config.txt",{});
};

static void OHDRpiConfigRaspicam(){
  OHDUtil::run_command("sed -i '$ a start_x=1' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a dtoverlay=vc4-fkms-v3d,cma-128' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a enable_uart=1' /boot/config.txt",{});
};

static void OHDRpiConfigArducam(){
  OHDUtil::run_command("sed -i '$ a dtoverlay=arducam-pivariety' /boot/config.txt",{});
};
static void OHDRpiConfigArducamImx477(){
  OHDUtil::run_command("sed -i '$ a dtoverlay=vc4-kms-v3d,cma-128' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a camera_auto_detect=0' /boot/config.txt",{});
  OHDUtil::run_command("sed -i '$ a dtoverlay=imx477' /boot/config.txt",{});
};

enum class CamConfig {
  MMAL = 0, // raspivid / gst-rpicamsrc
  LIBCAMERA, // "normal" libcamera
  LIBCAMERA_ARDUCAM, // pivariety libcamera (arducam special)
};
static CamConfig cam_config_from_string(const std::string& config){
  if(OHDUtil::equal_after_uppercase(config,"mmal")){
    return CamConfig::MMAL;
  }else if(OHDUtil::equal_after_uppercase(config,"libcamera")){
    return CamConfig::LIBCAMERA;
  }else if(OHDUtil::equal_after_uppercase(config,"libcamera_arducam")){
    return CamConfig::LIBCAMERA_ARDUCAM;
  }
  openhd::loggers::get_default()->warn("cam_config_from_string error");
  // return default
  return CamConfig::MMAL;
}
static std::string cam_config_to_string(const CamConfig& cam_config){
  if(cam_config==CamConfig::MMAL){
    return "mmal";
  }else if(cam_config==CamConfig::LIBCAMERA){
    return "libcamera";
  }
  return "libcamera_arducam";
}
static CamConfig cam_config_from_int(int val){
  if(val==0)return CamConfig::MMAL;
  if(val==1)return CamConfig::LIBCAMERA;
  if(val==2)return CamConfig::LIBCAMERA_ARDUCAM;
  openhd::loggers::get_default()->warn("Error cam_config_from_int");
  return CamConfig::MMAL;
}
static int cam_config_to_int(CamConfig cam_config){
  if(cam_config==CamConfig::MMAL){
    return 0;
  }else if(cam_config==CamConfig::LIBCAMERA){
    return 1;
  }
  return 2;
}

static bool validate_cam_config_settings_int(int val){
  return val==0 || val==1 || val==2;
}

static constexpr auto CAM_CONFIG_FILENAME="/boot/openhd/rpi_cam_config.txt";

static CamConfig get_current_cam_config_from_file(){
  OHDFilesystemUtil::create_directories("/boot/openhd/");
  if(!OHDFilesystemUtil::exists(CAM_CONFIG_FILENAME)){
    // The OHD image builder defaults to mmal, NOTE this is in contrast to the default rpi os release.
    OHDFilesystemUtil::write_file(CAM_CONFIG_FILENAME, cam_config_to_string(CamConfig::MMAL));
    return CamConfig::MMAL;
  }
  auto content=OHDFilesystemUtil::read_file(CAM_CONFIG_FILENAME);
  return cam_config_from_string(content);
}

static void save_cam_config_to_file(CamConfig new_cam_config){
  OHDFilesystemUtil::create_directories("/boot/openhd/");
  OHDFilesystemUtil::write_file(CAM_CONFIG_FILENAME, cam_config_to_string(new_cam_config));
}

// Applies the new cam config (rewrites the /boot/config.txt file)
// Then writes the type corresponding to the current configuration into the settings file.
static void apply_new_cam_config_and_save(CamConfig new_cam_config){
  openhd::loggers::get_default()->warn("Begin apply cam config"+ cam_config_to_string(new_cam_config));
  if(new_cam_config==CamConfig::MMAL){
    OHDRpiConfigClear();
    OHDRpiConfigRaspicam();
  }else if(new_cam_config==CamConfig::LIBCAMERA){
    OHDRpiConfigClear();
    OHDRpiConfigLibcamera();
  }else{
    assert(new_cam_config==CamConfig::LIBCAMERA_ARDUCAM);
    OHDRpiConfigClear();
    OHDRpiConfigArducam();
  }
  save_cam_config_to_file(new_cam_config);
  openhd::loggers::get_default()->warn("End apply cam config"+ cam_config_to_string(new_cam_config));
}

// Unfortunately complicated, since we need to perform the action asynchronously and then reboot
// but also have to make sure a eager user doesn't change the config multiple times and then the pi
// "reboots in between" a change
class ConfigChangeHandler{
 public:
  // Returns true if checks passed, false otherise (param rejected)
  bool change_rpi_os_camera_configuration(int new_value_as_int){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(!validate_cam_config_settings_int(new_value_as_int)){
      // reject, not a valid value
      return false;
    }
    const auto current_configuration=get_current_cam_config_from_file();
    const auto new_configuration=cam_config_from_int(new_value_as_int);
    if(current_configuration==new_configuration){
      openhd::loggers::get_default()->warn("Not changing cam config,already at "+ cam_config_to_string(current_configuration));
      return true;
    }
    // this change requires a reboot, so only allow changing once at run time
    if(m_changed_once)return false;
    m_changed_once= true;
    // This will apply the changes asynchronous, even though we are "not done yet"
    // We assume nothing will fail on this command and return true already,such that we can
    // send the ack.
    apply_async(new_configuration);
    return true;
  }
 private:
  std::mutex m_mutex;
  bool m_changed_once=false;
  std::unique_ptr<std::thread> m_handle_thread;
  void apply_async(CamConfig new_value){
    // This is okay, since we will restart anyways
    m_handle_thread=std::make_unique<std::thread>([new_value]{
      apply_new_cam_config_and_save(new_value);
      std::this_thread::sleep_for(std::chrono::seconds(3));
      OHDUtil::run_command("systemctl",{"start", "reboot.target"});
    });
  }
};


}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_HPP_
