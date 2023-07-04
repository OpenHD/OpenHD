//
// Created by consti10 on 24.10.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_HPP_

#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

#include "openhd_platform.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "openhd_reboot_util.h"

// Helper to reconfigure the rpi os for the different camera types
// This really is exhausting - some camera(s) are auto-detected, some are not,
// And also gst-rpicamsrc (mmal) and libcamera need different config.txt files.
// They are also slight differences between the RPI4/CM4 and RPI3 or older
// Currently we use Dynamic Config files, which only include the important lines, 
// the config.txt can be changed by the user again, just the dynamic part is overwritten by openhd
// Note that the action(s) here are required for the OS to detect and configure the camera -
// only a camera detected by the OS can then be detected by the OHD camera(s) discovery
namespace openhd::rpi::os{

enum class CamConfig {
  MMAL = 0, // raspivid / gst-rpicamsrc
  L_IMX462_LOWLIGHT_MINI, // pivariety libcamera for the imx462 mini
  L_SKYMASTERHDR, // "normal" libcamera, explicitly set to imx708 detection only
  L_SKYVISIONPRO, // "normal" libcamera, explicitly set to imx519 detection only
  L_IMX477M, // "normal" libcamera, explicitly set to imx477 detection only
  L_IMX477, // "normal" libcamera, explicitly set to imx477 detection only
  L_IMX462,// Arducam imx462
  L_IMX327,// Arducam imx327 
  L_IMX290,// Arducam imx290 
  LIBCAMERA_AUTO,  // "normal" libcamera (autodetect)
  L_ARDUCAM_AUTO, // Arducam libcamera (piviarity) (autodetect)
  L_VEYE_2MP, // Veye IMX290/IMX327 (older versions)
  L_VEYE_CSIMX307, // Veye IMX307
  L_VEYE_CSSC132, //Veye SC132
  L_VEYE_MVCAM, // Veye MV Cameras
};
static std::string cam_config_to_string(const CamConfig& cam_config){
  switch (cam_config) {
    case CamConfig::MMAL: return "mmal";
    case CamConfig::L_IMX462_LOWLIGHT_MINI: return "libcamera_arducam";
    case CamConfig::L_SKYMASTERHDR: return "libcamera_imx708";
    case CamConfig::L_SKYVISIONPRO: return "libcamera_imx519";
    case CamConfig::L_IMX477M: return "libcamera_imx477m";
    case CamConfig::L_IMX477: return "libcamera_imx477";
    case CamConfig::L_IMX462: return "libcamera_imx462";
    case CamConfig::L_IMX327: return "libcamera_imx327";
    case CamConfig::L_IMX290: return "libcamera_imx290";
    case CamConfig::LIBCAMERA_AUTO: return "libcamera";
    case CamConfig::L_ARDUCAM_AUTO: return "libcamera_arducam";
    case CamConfig::L_VEYE_2MP: return "veye_cam2m";
    case CamConfig::L_VEYE_CSIMX307: return "veye_csimx307";
    case CamConfig::L_VEYE_CSSC132: return "veye_cssc132";
    case CamConfig::L_VEYE_MVCAM: return "veye_mvcam";
    default:break;
  }
  openhd::log::get_default()->warn("Error cam_config_to_string");
  assert(true);
  return "mmal";
}

static CamConfig cam_config_from_int(int val){
  if(val==0)return CamConfig::MMAL;
  if(val==1)return CamConfig::L_IMX462_LOWLIGHT_MINI;
  if(val==2)return CamConfig::L_SKYMASTERHDR;
  if(val==3)return CamConfig::L_SKYVISIONPRO;
  if(val==4)return CamConfig::L_IMX477M;
  if(val==5)return CamConfig::L_IMX477;
  if(val==6)return CamConfig::L_IMX462;
  if(val==7)return CamConfig::L_IMX327;
  if(val==8)return CamConfig::L_IMX290;
  if(val==9)return CamConfig::LIBCAMERA_AUTO;
  if(val==10)return CamConfig::L_ARDUCAM_AUTO;
  if(val==11)return CamConfig::L_VEYE_2MP;
  if(val==12)return CamConfig::L_VEYE_CSIMX307;
  if(val==13)return CamConfig::L_VEYE_CSSC132;
  if(val==14)return CamConfig::L_VEYE_MVCAM;
  openhd::log::get_default()->warn("Error cam_config_from_int");
  assert(true);
  return CamConfig::MMAL;
}
static int cam_config_to_int(CamConfig cam_config){
  switch (cam_config) {
    case CamConfig::MMAL: return 0;
    case CamConfig::L_IMX462_LOWLIGHT_MINI: return 1;
    case CamConfig::L_SKYMASTERHDR: return 2;
    case CamConfig::L_SKYVISIONPRO: return 3;
    case CamConfig::L_IMX477M: return 4;
    case CamConfig::L_IMX477: return 5;
    case CamConfig::L_IMX462: return 6;
    case CamConfig::L_IMX327: return 7;
    case CamConfig::L_IMX290: return 8;
    case CamConfig::LIBCAMERA_AUTO: return 9;
    case CamConfig::L_ARDUCAM_AUTO: return 10;
    case CamConfig::L_VEYE_2MP: return 11;
    case CamConfig::L_VEYE_CSIMX307: return 12;
    case CamConfig::L_VEYE_CSSC132: return 13;
    case CamConfig::L_VEYE_MVCAM: return 14;
    default:break;
  }
  openhd::log::get_default()->warn("Error cam_config_to_int");
  assert(true);
  return 0;
}

static bool validate_cam_config_settings_int(int val){
  return val>=0 && val<=13;
}

static std::string get_curr_cam_config_filename(){
  return openhd::get_video_settings_directory()+"curr_rpi_cam_config.txt";
}
static CamConfig get_current_cam_config_from_file(){
  if(!OHDFilesystemUtil::exists(get_curr_cam_config_filename())){
    // The OHD image builder defaults to mmal, NOTE this is in contrast to the default rpi os release.
    OHDFilesystemUtil::write_file(get_curr_cam_config_filename(), std::to_string(cam_config_to_int(CamConfig::MMAL)));
    return CamConfig::MMAL;
  }
  auto content=OHDFilesystemUtil::read_file(get_curr_cam_config_filename());
  auto content_as_int=OHDUtil::string_to_int(content);
  if(!content_as_int.has_value()){
    openhd::log::get_default()->error("Invalid value inside curr_rpi_cam_config.txt [{}]",content);
    return CamConfig::MMAL;
  }
  return cam_config_from_int(content_as_int.value());
}

static void save_cam_config_to_file(CamConfig new_cam_config){
  OHDFilesystemUtil::write_file(get_curr_cam_config_filename(),std::to_string(cam_config_to_int(new_cam_config)));
}

static std::string get_file_name_for_cam_config(const BoardType& board_type,const CamConfig& cam_config){
  const bool is_rpi4=board_type==BoardType::RaspberryPi4B || board_type==BoardType::RaspberryPiCM4;
  std::string base_filename="/boot/openhd/rpi_camera_configs/";
  if(cam_config==CamConfig::MMAL){
    return base_filename+"rpi_"+cam_config_to_string(cam_config)+".txt";
  }else{
    if(is_rpi4){
      return base_filename+"rpi_4_"+cam_config_to_string(cam_config)+".txt";
    }else{
      return base_filename+"rpi_3_"+cam_config_to_string(cam_config)+".txt";
    }
  }
  assert(true);
  return "";
}

// This has happened too often now - print a warning if any cam config is missing
static void runtime_check_if_all_cam_configs_exist(){
  for(int platform_idx=0;platform_idx<2;platform_idx++){
    const BoardType board_type=(platform_idx==0) ? BoardType::RaspberryPi3B : BoardType::RaspberryPi4B;
    for(int cam_config_idx=0;cam_config_idx<7;cam_config_idx++){
      const auto cam_config= cam_config_from_int(cam_config_idx);
      const auto filename= get_file_name_for_cam_config(board_type,cam_config);
      if(!OHDFilesystemUtil::exists(filename)){
        openhd::log::get_default()->warn("Cam config [{}] is missing !",filename);
      }else{
        //openhd::log::get_default()->debug("Cam config [{}] is available !",filename);
      }
    }
  }
}

// find the line that contains the dynamic content begin identifier
// after this identifier, we can modify things in the config file as we like.
// returns -1 on failure, a positive integer >=0 otherwise
// NOTE: line[index] contains the identifier, the dynamic content should then be written on line[index+1]
static int find_index_dynamic_content_begin(const std::vector<std::string>& lines){
  for(int i=0;i<lines.size();i++){
    if(OHDUtil::contains(lines[i],"#OPENHD_DYNAMIC_CONTENT_BEGIN#")){
      return i;
    }
  }
  return -1;
}

static constexpr auto rpi_config_file_path="/boot/config.txt";

// Applies the new cam config (rewrites the /boot/config.txt file)
// Then writes the type corresponding to the current configuration into the settings file.
// Returns true on success
// Returns false otherwise, the original state is then left untouched
static bool apply_new_cam_config_and_save(const BoardType& board_type,const CamConfig& new_cam_config){
  openhd::log::get_default()->debug("Begin apply cam config {}",cam_config_to_string(new_cam_config));
  const auto cam_config_filename= get_file_name_for_cam_config(board_type,new_cam_config);
  const auto cam_config_file_content_opt=OHDFilesystemUtil::opt_read_file(cam_config_filename);
if (!cam_config_file_content_opt.has_value()) {
    openhd::log::get_default()->warn("Cannot apply new cam config, corresponding *.txt [{}] not found", cam_config_filename);
    return false;
} else {
    if (cam_config_filename.find("imx477m") != std::string::npos) {
    openhd::log::get_default()->warn("Custom Tuning file is about to be installed!");
    if (OHDFilesystemUtil::exists("/usr/share/libcamera/ipa/raspberrypi/arducam-477m.json")) {
        openhd::log::get_default()->warn("Custom Tuning file found, now it'll be enabled");
        OHDUtil::run_command("mv", {"/usr/share/libcamera/ipa/raspberrypi/imx477.json", "/usr/share/libcamera/ipa/raspberrypi/imx477_old.json"});
        OHDUtil::run_command("cp", {"/usr/share/libcamera/ipa/raspberrypi/arducam-477m.json", "/usr/share/libcamera/ipa/raspberrypi/imx477.json"});
        openhd::log::get_default()->warn("Custom Tuning file written");
    } else {
        openhd::log::get_default()->warn("No Custom Tuning found!");
    }
} else if (cam_config_filename.find("imx477") != std::string::npos) {
    if (OHDFilesystemUtil::exists("/usr/share/libcamera/ipa/raspberrypi/imx477_old.json")) {
        openhd::log::get_default()->warn("Custom Tuning file found, Standart is about to be installed");
        OHDUtil::run_command("mv -f", {"/usr/share/libcamera/ipa/raspberrypi/imx477_old.json", "/usr/share/libcamera/ipa/raspberrypi/imx477.json"});
        openhd::log::get_default()->warn("Tuning file written");
    } else {
        openhd::log::get_default()->warn("No Tuning needed!");
    }
 }
}


  // Be more verbose why this stuff stands there - this is a comment added on top
  auto cam_config_file_content=fmt::format("#Curr cam config:{}\n", cam_config_to_string(new_cam_config))
	  +cam_config_file_content_opt.value();
  const auto cam_config_file_lines=OHDUtil::split_string_by_newline(cam_config_file_content);

  // read the content of the current config.txt file
  const auto config_file_content=OHDFilesystemUtil::opt_read_file(rpi_config_file_path);
  if(!config_file_content.has_value()){
    openhd::log::get_default()->warn("Cannot apply new cam config, original config.txt [{}] not found",rpi_config_file_path);
    return false;
  }
  // split it into lines
  const auto config_file_lines=OHDUtil::split_string_by_newline(config_file_content.value());
  // find the index where the dynamic content begins
  const auto dynamic_begin= find_index_dynamic_content_begin(config_file_lines);
  if(dynamic_begin<0){
    openhd::log::get_default()->warn("Your config.txt is not compatible with openhd-camera-changes (identifier not found)");
    return false;
  }
  // write the stuff we don't modify
  std::vector<std::string> lines_new_config_file;
  for(int i=0;i<=dynamic_begin;i++){
    lines_new_config_file.push_back(config_file_lines[i]);
  }
  assert(OHDUtil::contains(lines_new_config_file.at(lines_new_config_file.size()-1),"#OPENHD_DYNAMIC_CONTENT_BEGIN#"));
  // then add the stuff we modify
  for(const auto& line: cam_config_file_lines){
    lines_new_config_file.push_back(line);
  }
  // Now we are finished
  // Write a backup file of the original previous content for debugging
  OHDFilesystemUtil::write_file("/boot/config.txt.old",config_file_content.value());
  // and overwrite the old config file
  const auto new_config_file_content=OHDUtil::create_string_from_lines(lines_new_config_file);
  OHDFilesystemUtil::write_file(rpi_config_file_path,new_config_file_content);

  // save the current selection (persistent setting)
  save_cam_config_to_file(new_cam_config);

  // Now we just need to reboot
  openhd::log::get_default()->debug("End apply cam config {}",cam_config_to_string(new_cam_config));
  return true;
}

// Unfortunately complicated, since we need to perform the action asynchronously and then reboot
// but also have to make sure a eager user doesn't change the config multiple times and then the pi
// "reboots in between" a change
class ConfigChangeHandler{
 public:
  explicit ConfigChangeHandler(OHDPlatform platform): m_platform(platform){
    assert(m_platform.platform_type==PlatformType::RaspberryPi);
    runtime_check_if_all_cam_configs_exist();
  }
  // Returns true if checks passed, false otherwise (param rejected)
  bool change_rpi_os_camera_configuration(int new_value_as_int){
    std::lock_guard<std::mutex> lock(m_mutex);
    if(!validate_cam_config_settings_int(new_value_as_int)){
      // reject, not a valid value
      return false;
    }
    if(m_changed_once)return false;
    const auto current_configuration=get_current_cam_config_from_file();
    const auto new_configuration=cam_config_from_int(new_value_as_int);
    if(current_configuration==new_configuration){
      openhd::log::get_default()->warn("Not changing cam config,already at {}",cam_config_to_string(current_configuration));
      return true;
    }
    const bool success= apply_new_cam_config_and_save(m_platform.board_type,new_configuration);
    if(success){
      m_changed_once= true;
      // this change requires a reboot
      openhd::reboot::handle_power_command_async(std::chrono::seconds(3), false);
    }
    return success;
  }
 private:
  std::mutex m_mutex;
  std::unique_ptr<std::thread> m_handle_thread;
  const OHDPlatform m_platform;
  bool m_changed_once= false;
};


}
#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_RPI_OS_CONFIGURE_VENDOR_CAM_HPP_
