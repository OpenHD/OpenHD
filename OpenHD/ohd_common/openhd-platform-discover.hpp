//
// Created by consti10 on 23.06.22.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_PLATFORM_DISCOVER_HPP_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_PLATFORM_DISCOVER_HPP_

#include "openhd-platform.hpp"
#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"

#include <iostream>
#include <set>
#include <regex>

/**
 * Discover the platform we are running on.
 * Note: One should not use a instance of this class for anything else than discovery, to pass around the discovered
 * data use the struct from ohd_platform.
 */
class DPlatform {
 public:
  DPlatform() = default;
  virtual ~DPlatform() = default;
  static std::shared_ptr<OHDPlatform> discover(){
    openhd::loggers::get_default()->debug("Platform::discover()");
    auto platform=internal_discover();
    write_platform_manifest(*platform);
    return platform;
  }
 private:
  static constexpr auto JETSON_BOARDID_PATH = "/proc/device-tree/nvidia,boardids";
  static constexpr auto ROCKCHIP_PATH = "/proc/device-tree/rockchip-suspend/compatible";
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
    const auto res4=detect_pc();
    return std::make_shared<OHDPlatform>(res4.first,res4.second);
  }
  static std::optional<std::pair<PlatformType,BoardType>> detect_raspberrypi(){
    std::ifstream t("/proc/cpuinfo");
    std::string raw_value((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

    std::smatch result;
    // example "Revision	: 2a020d3"
    std::regex r{"Revision\\t*:\\s*([\\w]+)"};

    if (!std::regex_search(raw_value, result, r)) {
      openhd::loggers::get_default()->debug("Detect rpi no result");
      return {};
    }

    if (result.size() != 2) {
      openhd::loggers::get_default()->debug("Detect ri result doesnt match");
      return {};
    }

    const PlatformType platform_type = PlatformType::RaspberryPi;
    BoardType board_type=BoardType::Unknown;

    const std::string raspberry_identifier = result[1];
    openhd::loggers::get_default()->debug("Pi identifier:{"+raspberry_identifier+"}");

    //const std::set<std::string> pi4b_identifiers = {"a03111", "b03111", "b03112", "c03111", "c03112", "d03114"};
	// Not sure
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
    if (OHDFilesystemUtil::exists(ROCKCHIP_PATH)) {
      std::string rockchip_compatible = OHDFilesystemUtil::read_file(ROCKCHIP_PATH);

      if(rockchip_compatible.find("rk3588") != std::string::npos){
        return std::make_pair(PlatformType::Rockchip,BoardType::RK3588);
      }else{
        return std::make_pair(PlatformType::Rockchip,BoardType::Unknown);
      }      
    }
    return {};
  }
  static std::pair<PlatformType,BoardType> detect_pc(){
    const auto arch_opt=OHDUtil::run_command_out("arch");
    if(arch_opt==std::nullopt){
      openhd::loggers::get_default()->warn("Arch not found");
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
};

#endif //OPENHD_OPENHD_OHD_COMMON_OPENHD_PLATFORM_DISCOVER_HPP_
