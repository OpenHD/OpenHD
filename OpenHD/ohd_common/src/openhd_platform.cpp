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

#include "openhd_platform.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <set>

#include "openhd_spdlog.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

// Constants
static constexpr auto NVIDIA_XAVIER_BOARDID_PATH =
    "/proc/device-tree/nvidia,dtsfilename";
static constexpr auto DEVICE_TREE_COMPATIBLE_PATH =
    "/proc/device-tree/compatible";
static constexpr auto ALLWINNER_BOARDID_PATH = "/dev/cedar_dev";
static constexpr auto SIGMASTAR_BOARDID_PATH = "/dev/mstar_ive0";
static constexpr auto QUALCOMM_BOARDID_PATH = "/proc/device-tree/model";

static int internal_discover_platform() {
  openhd::log::get_default()->warn("OpenHD Platform Discovery started!");

  if (OHDFilesystemUtil::exists("/proc/device-tree/model")) {
    const std::string model_content =
        OHDFilesystemUtil::read_file("/proc/device-tree/model");
    if (OHDUtil::contains_after_uppercase(model_content, "ORQA")) {
      openhd::log::get_default()->warn("Detected Willy platform.");
      return X_PLATFORM_TYPE_WILLY;
    }
  }
  if (OHDFilesystemUtil::exists(ALLWINNER_BOARDID_PATH)) {
    openhd::log::get_default()->warn("Detected Allwinner platform (X20).");
    return X_PLATFORM_TYPE_ALWINNER_X20;
  }

  if (OHDFilesystemUtil::exists("/boot/config.txt")) {
    openhd::log::get_default()->warn(
        "Detected potential Raspberry Pi platform.");
    const auto filename_proc_cpuinfo = "/proc/cpuinfo";
    const auto proc_cpuinfo_opt =
        OHDFilesystemUtil::opt_read_file("/proc/cpuinfo");

    if (!proc_cpuinfo_opt.has_value()) {
      openhd::log::get_default()->warn(
          "File {} does not exist. Unable to complete Raspberry Pi detection.",
          filename_proc_cpuinfo);
      return X_PLATFORM_TYPE_RPI_OLD;
    }

    openhd::log::get_default()->warn("Checking Raspberry Pi hardware...");
    if (OHDUtil::contains(proc_cpuinfo_opt.value(), "BCM2711")) {
      openhd::log::get_default()->warn("Raspberry Pi 4 detected.");
      return X_PLATFORM_TYPE_RPI_4;
    }

    openhd::log::get_default()->warn("Detected an older Raspberry Pi (<=3).");
    return X_PLATFORM_TYPE_RPI_OLD;
  }

  if (OHDFilesystemUtil::exists(SIGMASTAR_BOARDID_PATH)) {
    openhd::log::get_default()->warn("Detected SigmaStar platform.");
    return X_PLATFORM_TYPE_OPENIPC_SIGMASTAR_UNDEFINED;
  }

  if (OHDFilesystemUtil::exists(DEVICE_TREE_COMPATIBLE_PATH)) {
    openhd::log::get_default()->warn("Checking for Rockchip platforms...");

    const std::string compatible_content =
        OHDFilesystemUtil::read_file(DEVICE_TREE_COMPATIBLE_PATH);
    const std::string device_tree_model =
        OHDFilesystemUtil::read_file("/proc/device-tree/model");
    std::regex r("rockchip,(r[kv][0-9]+)");
    std::smatch sm;

    if (regex_search(compatible_content, sm, r)) {
      const std::string chip = sm[1];
      openhd::log::get_default()->warn("Rockchip chip identified: {}", chip);

      if (chip == "rk3588") {
        if (OHDUtil::contains_after_uppercase(device_tree_model,
                                              "Radxa ROCK 5A")) {
          openhd::log::get_default()->warn(
              "Detected Rockchip RK3588 (Radxa ROCK 5A).");
          return X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A;
        } else {
          openhd::log::get_default()->warn(
              "Detected Rockchip RK3588 (Radxa ROCK 5B).");
          return X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B;
        }
      } else if (chip == "rk3566") {
        if (OHDUtil::contains_after_uppercase(device_tree_model,
                                              "Radxa CM3 RPI CM4 IO")) {
          openhd::log::get_default()->warn(
              "Detected Rockchip RK3566 (Radxa CM3).");
          return X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3;
        } else if (OHDUtil::contains_after_uppercase(device_tree_model,
                                                     "Radxa ROCK3 Model A")) {
          openhd::log::get_default()->warn(
              "Detected Rockchip RK3566 (Radxa ROCK3 Model A).");
          return X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W;
        } else {
          openhd::log::get_default()->warn(
              "Detected Rockchip RK3566 (default Radxa ZERO3W).");
          return X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W;
        }
      } else if (chip == "rv1126") {
        openhd::log::get_default()->warn("Detected Rockchip RV1126.");
        return X_PLATFORM_TYPE_ROCKCHIP_RV1126;
      } else if (chip == "rv1103") {
        openhd::log::get_default()->warn("Detected Rockchip RV1103.");
        return X_PLATFORM_TYPE_ROCKCHIP_RV1103;
      } else if (chip == "rv1106") {
        openhd::log::get_default()->warn("Detected Rockchip RV1106");
        return X_PLATFORM_TYPE_ROCKCHIP_RV1106;
      } else if (chip == "rk3506") {
        openhd::log::get_default()->warn("Detected Luckfox Lyra");
        return X_PLATFORM_TYPE_LUCKFOX_LYRA;
      }
    }

    openhd::log::get_default()->warn("No specific Rockchip match found.");
    return X_PLATFORM_TYPE_UNKNOWN;
  }

  if (OHDFilesystemUtil::exists(NVIDIA_XAVIER_BOARDID_PATH)) {
    openhd::log::get_default()->warn("Detected NVIDIA Xavier platform.");
    return X_PLATFORM_TYPE_NVIDIA_XAVIER;
  }

  if (OHDFilesystemUtil::exists(QUALCOMM_BOARDID_PATH)) {
    openhd::log::get_default()->warn("Checking for Qualcomm platforms...");
    const std::string qualcomm_board_id_content =
        OHDFilesystemUtil::read_file(QUALCOMM_BOARDID_PATH);

    std::regex qualcomm_regex("(qcs405|qrb5165)");
    std::smatch match;

    if (std::regex_search(qualcomm_board_id_content, match, qualcomm_regex)) {
      if (match[1] == "qcs405") {
        openhd::log::get_default()->warn("Detected Qualcomm QCS405.");
        return X_PLATFORM_TYPE_QUALCOMM_QCS405;
      } else if (match[1] == "qrb5165") {
        openhd::log::get_default()->warn("Detected Qualcomm QRB5165.");
        return X_PLATFORM_TYPE_QUALCOMM_QRB5165;
      }
    }

    openhd::log::get_default()->warn("No specific Qualcomm match found.");
    return X_PLATFORM_TYPE_UNKNOWN;
  }

  const auto arch_opt = OHDUtil::run_command_out("arch");

  if (!arch_opt.has_value()) {
    openhd::log::get_default()->warn("Could not determine architecture.");
    return X_PLATFORM_TYPE_UNKNOWN;
  }

  const auto arch = arch_opt.value();
  openhd::log::get_default()->warn("Architecture detected: {}", arch);

  if (std::regex_search(arch, std::regex{"x86_64"})) {
    openhd::log::get_default()->warn("Detected x86 platform.");
    return X_PLATFORM_TYPE_X86;
  }

  openhd::log::get_default()->warn("Unknown platform.");
  return X_PLATFORM_TYPE_UNKNOWN;
}

static void write_platform_manifest(const OHDPlatform& ohdPlatform) {
  static constexpr auto PLATFORM_MANIFEST_FILENAME =
      "/tmp/platform_manifest.txt";
  OHDFilesystemUtil::write_file(PLATFORM_MANIFEST_FILENAME,
                                ohdPlatform.to_string());
}

static OHDPlatform discover_and_write_manifest() {
  auto platform_int = internal_discover_platform();
  auto platform = OHDPlatform(platform_int);
  write_platform_manifest(platform);
  return platform;
}

std::string x_platform_type_to_string(int platform_type) {
  switch (platform_type) {
    case X_PLATFORM_TYPE_UNKNOWN:
      return "UNKNOWN";
    case X_PLATFORM_TYPE_X86:
      return "X86";
    case X_PLATFORM_TYPE_RPI_OLD:
      return "RPI<=3";
    case X_PLATFORM_TYPE_RPI_4:
      return "RPI 4";
    case X_PLATFORM_TYPE_RPI_5:
      return "RPI 5";
    case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W:
      return "RADXA ZERO3W";
    case X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3:
      return "RADXA CM3";
    case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A:
      return "RADXA RK3588S";
    case X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B:
      return "RADXA RK3588";
    case X_PLATFORM_TYPE_ROCKCHIP_RV1126:
      return "RV1126";
    case X_PLATFORM_TYPE_ROCKCHIP_RV1103:
      return "RV1103";
    case X_PLATFORM_TYPE_ROCKCHIP_RV1106:
      return "RV1106";
    case X_PLATFORM_TYPE_WILLY:
      return "Willy";
    case X_PLATFORM_TYPE_ALWINNER_X20:
      return "X20";
    case X_PLATFORM_TYPE_OPENIPC_SIGMASTAR_UNDEFINED:
      return "OPENIPC SIGMASTAR";
    case X_PLATFORM_TYPE_NVIDIA_XAVIER:
      return "NVIDIA_XAVIER";
    case X_PLATFORM_TYPE_QUALCOMM_QCS405:
      return "QUALCOMM_QCS405";
    case X_PLATFORM_TYPE_QUALCOMM_QRB5165:
      return "QUALCOMM_QRB5165";
    default:
      std::stringstream ss;
      ss << "ERR-UNDEFINED{" << platform_type << "}";
      return ss.str();
  }
}

int get_fec_max_block_size_for_platform() {
  auto platform_type = OHDPlatform::instance().platform_type;

  if (platform_type == X_PLATFORM_TYPE_RPI_4 ||
      platform_type == X_PLATFORM_TYPE_RPI_CM4) {
    return 50;
  }
  if (platform_type == X_PLATFORM_TYPE_RPI_OLD) {
    return 30;
  }
  if (platform_type == X_PLATFORM_TYPE_X86) {
    return 80;
  }
  if (platform_type == X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W ||
      platform_type == X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3 ||
      platform_type == X_PLATFORM_TYPE_ROCKCHIP_RV1103 ||
      platform_type == X_PLATFORM_TYPE_ROCKCHIP_RV1106 ||
      platform_type == X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A ||
      platform_type == X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B) {
    return 20;
  }
  if (platform_type == X_PLATFORM_TYPE_ALWINNER_X20) {
    return 20;
  }
  if (platform_type == X_PLATFORM_TYPE_NVIDIA_XAVIER) {
    return 50;
  }
  if (platform_type == X_PLATFORM_TYPE_WILLY) {
    return 50;
  }
  if (platform_type == X_PLATFORM_TYPE_QUALCOMM_QRB5165 ||
      platform_type == X_PLATFORM_TYPE_QUALCOMM_QCS405) {
    return 50;
  }

  return 20;
}

// OHDPlatform methods
const OHDPlatform& OHDPlatform::instance() {
  static OHDPlatform instance = discover_and_write_manifest();
  return instance;
}

std::string OHDPlatform::to_string() const {
  std::stringstream ss;
  ss << "OHDPlatform:[" << x_platform_type_to_string(platform_type) << "]";
  return ss.str();
}

bool OHDPlatform::is_rpi() const {
  return platform_type >= 10 && platform_type < 20;
}

bool OHDPlatform::is_rock() const {
  return platform_type >= 20 && platform_type < 30;
}

bool OHDPlatform::is_rpi_or_x86() const {
  return is_rpi() || platform_type == X_PLATFORM_TYPE_X86;
}

bool OHDPlatform::is_x20() const {
  return platform_type == X_PLATFORM_TYPE_ALWINNER_X20;
}

bool OHDPlatform::is_willy() const {
  return platform_type == X_PLATFORM_TYPE_WILLY;
}

bool OHDPlatform::is_zero3w() const {
  return platform_type == X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_ZERO3W;
}

bool OHDPlatform::is_radxa_cm3() const {
  return platform_type == X_PLATFORM_TYPE_ROCKCHIP_RK3566_RADXA_CM3;
}

bool OHDPlatform::is_rock5_a() const {
  return platform_type == X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_A;
}

bool OHDPlatform::is_rock5_b() const {
  return platform_type == X_PLATFORM_TYPE_ROCKCHIP_RK3588_RADXA_ROCK5_B;
}

bool OHDPlatform::is_rock5_a_b() const { return is_rock5_a() || is_rock5_b(); }

bool OHDPlatform::is_qcs405() const {
  return platform_type == X_PLATFORM_TYPE_QUALCOMM_QCS405;
}

bool OHDPlatform::is_qrb5165() const {
  return platform_type == X_PLATFORM_TYPE_QUALCOMM_QRB5165;
}
