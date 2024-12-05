/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#ifndef OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_
#define OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_

#include <map>
#include <sstream>
#include <string>

#include "config_paths.h"
#include "openhd_util_filesystem.h"

namespace openhd::tmp {

// Note: case sensitive
const auto FILENAME_AIR = std::string(getConfigBasePath()) + "air.txt";
const auto FILENAME_GROUND = std::string(getConfigBasePath()) + "ground.txt";
const auto FILENAME_ETHERNET =
    std::string(getConfigBasePath()) + "ethernet.txt";

static bool file_air_exists() {
  return OHDFilesystemUtil::exists(FILENAME_AIR);
}

static bool file_ground_exists() {
  return OHDFilesystemUtil::exists(FILENAME_GROUND);
}

static bool file_ethernet_exists() {
  return OHDFilesystemUtil::exists(FILENAME_ETHERNET);
}

static bool file_air_or_ground_exists() {
  return file_air_exists() || file_ground_exists();
}

static void delete_any_file_air_or_ground() {
  OHDFilesystemUtil::remove_if_existing(FILENAME_AIR);
  OHDFilesystemUtil::remove_if_existing(FILENAME_GROUND);
}

static void delete_file_ethernet() {
  OHDFilesystemUtil::remove_if_existing(FILENAME_ETHERNET);
}

static void write_file_air() {
  OHDFilesystemUtil::create_directories(std::string(getConfigBasePath()));
  OHDFilesystemUtil::write_file(openhd::tmp::FILENAME_AIR, " ");
}

static void write_file_ground() {
  OHDFilesystemUtil::create_directories(std::string(getConfigBasePath()));
  OHDFilesystemUtil::write_file(openhd::tmp::FILENAME_GROUND, " ");
}

// Structure for Ethernet configuration
struct EthernetConfig {
  std::string ground_unit_ip = "192.168.2.1";
  std::string air_unit_ip = "192.168.2.18";
  int video_port = 5910;
  int telemetry_port = 5920;

  // Parse Ethernet configuration from string
  static EthernetConfig fromString(const std::string& content) {
    EthernetConfig config;
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
      if (line.empty() || line[0] == '#')
        continue;  // Skip comments and empty lines
      auto pos = line.find('=');
      if (pos != std::string::npos) {
        auto key = line.substr(0, pos);
        auto value = line.substr(pos + 1);
        if (key == "GROUND_UNIT_IP")
          config.ground_unit_ip = value;
        else if (key == "AIR_UNIT_IP")
          config.air_unit_ip = value;
        else if (key == "VIDEO_PORT")
          config.video_port = std::stoi(value);
        else if (key == "TELEMETRY_PORT")
          config.telemetry_port = std::stoi(value);
      }
    }
    return config;
  }

  // Serialize Ethernet configuration to string
  std::string toString() const {
    std::ostringstream stream;
    stream << "GROUND_UNIT_IP=" << ground_unit_ip << "\n";
    stream << "AIR_UNIT_IP=" << air_unit_ip << "\n";
    stream << "VIDEO_PORT=" << video_port << "\n";
    stream << "TELEMETRY_PORT=" << telemetry_port << "\n";
    return stream.str();
  }
};

// Write Ethernet configuration to file
static void write_file_ethernet(const EthernetConfig& config) {
  OHDFilesystemUtil::create_directories(getConfigBasePath());
  OHDFilesystemUtil::write_file(openhd::tmp::FILENAME_ETHERNET,
                                config.toString());
}

// Read Ethernet configuration from file
static EthernetConfig read_file_ethernet() {
  if (!file_ethernet_exists()) {
    throw std::runtime_error("Ethernet configuration file not found: " +
                             FILENAME_ETHERNET);
  }
  auto content = OHDFilesystemUtil::read_file(FILENAME_ETHERNET);
  return EthernetConfig::fromString(content);
}

// Delete all configuration files (air, ground, ethernet)
static void delete_all_config_files() {
  delete_any_file_air_or_ground();
  delete_file_ethernet();
}

static bool handle_telemetry_change(int value) {
  // 0==ground, 1==air, other: undefined (rejected)
  if (!(value == 0 || value == 1)) return false;
  if (value == 0) {
    // change to ground mode. Remove any existing file(s) if there are any
    openhd::tmp::delete_any_file_air_or_ground();
    openhd::tmp::write_file_ground();
  } else {
    // change to air mode. Remove any existing file(s) if there are any
    openhd::tmp::delete_any_file_air_or_ground();
    openhd::tmp::write_file_air();
  }
  return true;
}

}  // namespace openhd::tmp

#endif  // OPENHD_OPENHD_OHD_COMMON_OPENHD_TEMPORARY_AIR_OR_GROUND_H_
