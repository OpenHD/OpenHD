//
// Created by consti10 on 10.02.23.
//

#include "openhd_platform.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <set>

#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "openhd_spdlog.h"


static constexpr auto JETSON_BOARDID_PATH = "/proc/device-tree/nvidia,boardids";
static constexpr auto DEVICE_TREE_COMPATIBLE_PATH = "/proc/device-tree/compatible";
static constexpr auto ALLWINNER_BOARDID_PATH = "/dev/cedar_dev";


static int internal_discover_platform(){
  //These are the 'easy ones'
  if(OHDFilesystemUtil::exists(ALLWINNER_BOARDID_PATH)){
    return X_PLATFORM_TYPE_ALWINNER_X20;
  }
  if (OHDFilesystemUtil::exists(DEVICE_TREE_COMPATIBLE_PATH)) {
    std::string compatible_content = OHDFilesystemUtil::read_file(DEVICE_TREE_COMPATIBLE_PATH);
    std::regex r("rockchip,(r[kv][0-9]+)");
    std::smatch sm;
    if (regex_search(compatible_content, sm, r)) {
      const std::string chip = sm[1];
      if(chip == "rk3588"){
        return X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5;
      }else if(chip=="rk3566"){
        return X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W;
      }else if(chip=="rv1126"){
        return X_PLATFORM_TYPE_ROCKCHIP_RV1126_UNDEFINED;
      }
    }
  }
  // If this file exists we can be sure we are on (any) RPI
  if(OHDFilesystemUtil::exists("/boot/config.txt")){
    const auto filename_proc_cpuinfo="/proc/cpuinfo";
    const auto proc_cpuinfo_opt=OHDFilesystemUtil::opt_read_file("/proc/cpuinfo");
    if(!proc_cpuinfo_opt.has_value()){
      openhd::log::get_default()->warn("File {} does not exist, rpi detection unavailable",filename_proc_cpuinfo);
      return X_PLATFORM_TYPE_RPI_OLD;
    }
    if(OHDUtil::contains(proc_cpuinfo_opt.value(),"BCM2711")){
      return X_PLATFORM_TYPE_RPI_4;
    }
    return X_PLATFORM_TYPE_RPI_OLD;
  }
  {
    // X86
    const auto arch_opt=OHDUtil::run_command_out("arch");
    if(arch_opt==std::nullopt){
      openhd::log::get_default()->warn("Arch not found");
      return X_PLATFORM_TYPE_UNKNOWN;
    }
    const auto arch=arch_opt.value();
    std::smatch result;
    std::regex r1{"x86_64"};
    auto res1 = std::regex_search(arch, result, r1);
    if(res1){
      return X_PLATFORM_TYPE_X86;
    }
  }
  openhd::log::get_default()->warn("Unknown platform");
  return X_PLATFORM_TYPE_UNKNOWN;
}


static void write_platform_manifest(const OHDPlatform& ohdPlatform) {
    static constexpr auto PLATFORM_MANIFEST_FILENAME = "/tmp/platform_manifest.txt";
    OHDFilesystemUtil::write_file(PLATFORM_MANIFEST_FILENAME,ohdPlatform.to_string());
}

static OHDPlatform discover_and_write_manifest() {
  auto platform_int=internal_discover_platform();
  auto platform=OHDPlatform(platform_int);
  write_platform_manifest(platform);
  openhd::log::get_default()->info("{}",platform.to_string());
  return platform;
}

std::string x_platform_type_to_string(int platform_type) {
  switch (platform_type) {
    case X_PLATFORM_TYPE_UNKNOWN:return "UNKNOWN";
    case X_PLATFORM_TYPE_X86:return "X86";
    case X_PLATFORM_TYPE_RPI_OLD: return "RPI <=3";
    case X_PLATFORM_TYPE_RPI_4: return "RPI 4";
    case X_PLATFORM_TYPE_RPI_5: return "RPI 5";
    // RPI END
    case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W: return "RADXA ZERO3W";
    case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5: return "RADXA ROCK5";
    case X_PLATFORM_TYPE_ROCKCHIP_RV1126_UNDEFINED: return "RV1126 UNDEFINED";
    // ROCK END
    case X_PLATFORM_TYPE_ALWINNER_X20: return "X20";
    case X_PLATFORM_TYPE_SIGMASTAR_UNDEFINED: return "SIGMASTAR UNDEFINED";
    default:
      break;
  }
  return "ERR-UNDEFINED";
}
int get_fec_max_block_size_for_platform(int platform_type) {
  if(platform_type==X_PLATFORM_TYPE_RPI_4 || platform_type==X_PLATFORM_TYPE_RPI_CM4){
    return 50;
  }
  if(platform_type==X_PLATFORM_TYPE_RPI_OLD){
    return 30;
  }
  if(platform_type==X_PLATFORM_TYPE_X86){
    return 50;
  }
  if(platform_type==X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W
      || platform_type==X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5){
    return 50;
  }
  // For now
  if(platform_type==X_PLATFORM_TYPE_ALWINNER_X20){
    return 50;
  }
  return 20;
}

const OHDPlatform &OHDPlatform::instance() {
    static OHDPlatform instance=discover_and_write_manifest();
    return instance;
}

std::string OHDPlatform::to_string() const {
  std::stringstream ss;
  ss<<"OHDPlatform:["<<x_platform_type_to_string(platform_type)<<"]";
  return ss.str();
}

bool OHDPlatform::is_rpi() const {
  return platform_type>=10 && platform_type<20;
}

bool OHDPlatform::is_rock() const {
  return platform_type>=20 && platform_type<30;
}

bool OHDPlatform::is_rpi_or_x86()const {
  return is_rpi() || platform_type==X_PLATFORM_TYPE_X86;
}

bool OHDPlatform::is_allwinner() const {
  return platform_type==X_PLATFORM_TYPE_ALWINNER_X20;
}
