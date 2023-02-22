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

#include "include_json.hpp"

std::string platform_type_to_string(PlatformType platform_type) {
  switch (platform_type) {
    case PlatformType::Jetson: return "jetson";
    case PlatformType::RaspberryPi: return "raspberrypi";
    case PlatformType::Allwinner: return "allwinner";
    case PlatformType::iMX6: return "imx6";
    case PlatformType::Zynq: return "zynq";
    case PlatformType::PC: return "pc";
    case PlatformType::Rockchip: return "rockchip";
    default: return "unknown";
  }
}

std::string board_type_to_string(BoardType board_type) {
  switch (board_type) {
    case BoardType::GenericPC: return "generic-pc";
    case BoardType::RaspberryPiZero: return "pizero";
    case BoardType::RaspberryPiZeroW: return "pizerow";
    case BoardType::RaspberryPi2B: return "pi2b";
    case BoardType::RaspberryPi3A: return "pi3a";
    case BoardType::RaspberryPi3APlus: return "pi3aplus";
    case BoardType::RaspberryPi3B: return "pi3b";
    case BoardType::RaspberryPi3BPlus: return "pi3bplus";
    case BoardType::RaspberryPiCM: return "picm";
    case BoardType::RaspberryPiCM3: return "picm3";
    case BoardType::RaspberryPiCM3Plus: return "picm3plus";
    case BoardType::RaspberryPiCM4: return "picm4";
    case BoardType::RaspberryPi4B: return "pi4b";
    case BoardType::JetsonNano: return "jetson-nano";
    case BoardType::JetsonTX1: return "jetson-tx1";
    case BoardType::JetsonTX2: return "jetson-tx2";
    case BoardType::JetsonNX: return "jetson-nx";
    case BoardType::JetsonAGX: return "jetson-agx";
    case BoardType::NanoPiDuo2: return "nanopi-duo2";
    case BoardType::PynqZ1: return "pynqz1";
    case BoardType::PynqZ2: return "pynqz2";
    case BoardType::X3DRSolo: return "3dr-solo";
    case BoardType::RK3588: return "rk3588";
    case BoardType::RV1109: return "rv1109";
    case BoardType::RV1126: return "rv1126";
    default: return "unknown";
  }
}


static constexpr auto JETSON_BOARDID_PATH = "/proc/device-tree/nvidia,boardids";
static constexpr auto DEVICE_TREE_COMPATIBLE_PATH = "/proc/device-tree/compatible";
static constexpr auto ALLWINNER_BOARDID_PATH = "/sys/class/sunxi_info/sys_info";

static std::optional<std::pair<PlatformType,BoardType>> detect_raspberrypi(){
  const auto filename_proc_cpuinfo="/proc/cpuinfo";
  const auto proc_cpuinfo_opt=OHDFilesystemUtil::opt_read_file("/proc/cpuinfo");
  if(!proc_cpuinfo_opt.has_value()){
    openhd::log::get_default()->warn("File {} does not exist, rpi detection unavailable",filename_proc_cpuinfo);
    return {};
  }
  const auto& proc_cpuinfo=proc_cpuinfo_opt.value();

  std::smatch result;
  // example "Revision	: 2a020d3"
  const std::regex r{R"(Revision\t*:\s*([\w]+))"};

  if (!std::regex_search(proc_cpuinfo, result, r)) {
    openhd::log::get_default()->debug("Detect rpi no result");
    return {};
  }

  if (result.size() != 2) {
    openhd::log::get_default()->debug("Detect rpi result doesnt match");
    return {};
  }

  const PlatformType platform_type = PlatformType::RaspberryPi;
  BoardType board_type=BoardType::Unknown;

  const std::string raspberry_identifier = result[1];
  if(raspberry_identifier == "0000")
  {
    std::cout<<"identifier:{"<<raspberry_identifier<<"} is not raspberry pi\n";
    return {};
  }
  openhd::log::get_default()->debug("Pi identifier:[{}]",raspberry_identifier);

  const std::set<std::string> pi4b_identifiers = {"a03111", "b03111", "b03112", "c03111", "c03112", "d03114","b03115"};
  if (pi4b_identifiers.find(raspberry_identifier) != pi4b_identifiers.end()) {
    board_type = BoardType::RaspberryPi4B;
  }

  const std::set<std::string> pi3b_identifiers = {"2a02082", "2a22082", "2a32082", "2a52082"};
  if (pi3b_identifiers.find(raspberry_identifier) != pi3b_identifiers.end()) {
    board_type = BoardType::RaspberryPi3B;
  }

  const std::set<std::string> pizero_identifiers = {"2900092", "2900093", "2920092", "2920093"};
  if (pizero_identifiers.find(raspberry_identifier) != pizero_identifiers.end()) {
    board_type = BoardType::RaspberryPiZero;
  }

  const std::set<std::string> pi2b_identifiers = {"2a22042", "2a21041", "2a01041", "2a01040"};
  if (pi2b_identifiers.find(raspberry_identifier) != pi2b_identifiers.end()) {
    board_type = BoardType::RaspberryPi2B;
  }

  if (raspberry_identifier == "29020e0") {
    board_type = BoardType::RaspberryPi3APlus;
  }

  if (raspberry_identifier == "2a020d3") {
    board_type = BoardType::RaspberryPi3BPlus;
  }

  if (raspberry_identifier == "29000c1") {
    board_type = BoardType::RaspberryPiZeroW;
  }
  return std::make_pair(platform_type,board_type);
}
static std::optional<std::pair<PlatformType,BoardType>> detect_jetson(){
  if (OHDFilesystemUtil::exists(JETSON_BOARDID_PATH)) {
    return std::make_pair(PlatformType::Jetson,BoardType::JetsonNano);
  }
  return {};
}
static std::optional<std::pair<PlatformType,BoardType>> detect_rockchip(){
  if (OHDFilesystemUtil::exists(DEVICE_TREE_COMPATIBLE_PATH)) {
    std::string compatible_content = OHDFilesystemUtil::read_file(DEVICE_TREE_COMPATIBLE_PATH);

    std::regex r("rockchip,(r[kv][0-9]+)");
    std::smatch sm;

    if (regex_search(compatible_content, sm, r)) {
      const std::string chip = sm[1];
      if(chip == "rk3588"){
        return std::make_pair(PlatformType::Rockchip, BoardType::RK3588);
      }else if(chip == "rv1109"){
        return std::make_pair(PlatformType::Rockchip, BoardType::RV1109);
      }else if(chip == "rv1126"){
        return std::make_pair(PlatformType::Rockchip, BoardType::RV1126);
      }else{
        return std::make_pair(PlatformType::Rockchip, BoardType::Unknown);
      }
    }
  }
  return {};
}
static std::optional<std::pair<PlatformType,BoardType>> detect_allwinner(){
  if (OHDFilesystemUtil::exists(ALLWINNER_BOARDID_PATH)) {
    return std::make_pair(PlatformType::Allwinner,BoardType::NanoPiDuo2);
  }
  return {};
}
static std::pair<PlatformType,BoardType> detect_pc(){
  const auto arch_opt=OHDUtil::run_command_out("arch");
  if(arch_opt==std::nullopt){
    openhd::log::get_default()->warn("Arch not found");
    return {PlatformType::Unknown,BoardType::Unknown};
  }
  const auto arch=arch_opt.value();
  std::smatch result;
  std::regex r1{"x86_64"};
  auto res1 = std::regex_search(arch, result, r1);

  std::regex r2{"i386"};
  auto res2 = std::regex_search(arch, result, r2);

  if (!res1 && !res2) {
    return {PlatformType::Unknown,BoardType::Unknown};
  }
  return std::make_pair(PlatformType::PC, BoardType::GenericPC);
}

static std::shared_ptr<OHDPlatform> internal_discover(){
  const auto res=detect_raspberrypi();
  if(res.has_value()){
    return std::make_shared<OHDPlatform>(res.value().first,res.value().second);
  }
  const auto res2=detect_jetson();
  if(res2.has_value()){
    return std::make_shared<OHDPlatform>(res2.value().first,res2.value().second);
  }
  const auto res3 = detect_rockchip();
  if(res3.has_value()){
    return std::make_shared<OHDPlatform>(res3.value().first,res3.value().second);
  }
  const auto res4=detect_allwinner();
  if(res4.has_value()){
    return std::make_shared<OHDPlatform>(res4.value().first,res4.value().second);
  }
  const auto res5=detect_pc();
  return std::make_shared<OHDPlatform>(res5.first,res5.second);
}


std::shared_ptr<OHDPlatform> DPlatform::discover() {
  openhd::log::get_default()->debug("Platform::discover()");
  auto platform=internal_discover();
  write_platform_manifest(*platform);
  return platform;
}

// Thw write out here is only for debugging
static void to_json(nlohmann::json& j,const OHDPlatform &ohdPlatform) {
  j = nlohmann::json{{"platform_type", platform_type_to_string(ohdPlatform.platform_type)},
                     {"board_type", board_type_to_string(ohdPlatform.board_type)}};
}

static constexpr auto PLATFORM_MANIFEST_FILENAME = "/tmp/platform_manifest";

void write_platform_manifest(const OHDPlatform& ohdPlatform) {
  nlohmann::json manifest = ohdPlatform;
  std::ofstream _t(PLATFORM_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}