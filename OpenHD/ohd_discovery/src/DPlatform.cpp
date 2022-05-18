#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <set>

#include <regex>

#include "json.hpp"

#include "openhd-platform.hpp"
#include "openhd-log.hpp"
#include "openhd-util.hpp"
#include "openhd-util-filesystem.hpp"

#include "DPlatform.h"

static constexpr auto JETSON_BOARDID_PATH = "/proc/device-tree/nvidia,boardids";

void DPlatform::discover() {
  std::cout << "Platform::discover()" << std::endl;

  detect_raspberrypi();
  detect_jetson();
  detect_pc();
}

void DPlatform::detect_raspberrypi() {
  std::ifstream t("/proc/cpuinfo");
  std::string raw_value((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

  std::smatch result;
  // example "Revision	: 2a020d3"
  std::regex r{"Revision\\t*:\\s*([\\w]+)"};

  if (!std::regex_search(raw_value, result, r)) {
	std::cout << "Detect rpi no result" << std::endl;
	return;
  }

  if (result.size() != 2) {
	std::cout << "Detect ri result doesnt match" << std::endl;
	return;
  }

  m_platform_type = PlatformTypeRaspberryPi;

  std::string raspberry_identifier = result[1];

  std::set<std::string> pi4b_identifiers = {"a03111", "b03111", "b03112", "c03111", "c03112", "d03114"};
  if (pi4b_identifiers.find(raspberry_identifier) != pi4b_identifiers.end()) {
	m_board_type = BoardTypeRaspberryPi4B;
  }

  std::set<std::string> pi3b_identifiers = {"2a02082", "2a22082", "2a32082", "2a52082"};
  if (pi3b_identifiers.find(raspberry_identifier) != pi3b_identifiers.end()) {
	m_board_type = BoardTypeRaspberryPi3B;
  }

  std::set<std::string> pizero_identifiers = {"2900092", "2900093", "2920092", "2920093"};
  if (pizero_identifiers.find(raspberry_identifier) != pizero_identifiers.end()) {
	m_board_type = BoardTypeRaspberryPiZero;
  }

  std::set<std::string> pi2b_identifiers = {"2a22042", "2a21041", "2a01041", "2a01040"};
  if (pi2b_identifiers.find(raspberry_identifier) != pi2b_identifiers.end()) {
	m_board_type = BoardTypeRaspberryPi2B;
  }

  if (raspberry_identifier == "29020e0") {
	m_board_type = BoardTypeRaspberryPi3APlus;
  }

  if (raspberry_identifier == "2a020d3") {
	m_board_type = BoardTypeRaspberryPi3BPlus;
  }

  if (raspberry_identifier == "29000c1") {
	m_board_type = BoardTypeRaspberryPiZeroW;
  }
}

void DPlatform::detect_jetson() {
  if (OHDFilesystemUtil::exists(JETSON_BOARDID_PATH)) {
	m_platform_type = PlatformTypeJetson;
	m_board_type = BoardTypeJetsonNano;
  }
}

void DPlatform::detect_pc() {
  std::array<char, 512> buffer{};
  std::string raw_value;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("arch", "r"), pclose);
  if (!pipe) {
	return;
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
	raw_value += buffer.data();
  }

  std::smatch result;
  std::regex r1{"x86_64"};
  auto res1 = std::regex_search(raw_value, result, r1);

  std::regex r2{"i386"};
  auto res2 = std::regex_search(raw_value, result, r2);

  if (!res1 && !res2) {
	return;
  }

  m_platform_type = PlatformTypePC;
  m_board_type = BoardTypeGenericPC;
}

void DPlatform::write_manifest() {
  const OHDPlatform ohdPlatform{m_platform_type, m_board_type, m_carrier_type};
  write_platform_manifest(ohdPlatform);
}

