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

#include <sstream>

#include "include_json.hpp"
#include "openhd_settings_directories.h"
#include "openhd_sock.h"
#include "openhd_spdlog.h"
#include "openhd_util_filesystem.h"

namespace {

const char* platform_cache_path() {
  static const std::string path =
      std::string(openhd::SETTINGS_BASE_PATH) + "platform.json";
  return path.c_str();
}

std::optional<int> read_cached_platform_type() {
  if (!OHDFilesystemUtil::exists(platform_cache_path())) {
    return std::nullopt;
  }
  const auto content =
      OHDFilesystemUtil::opt_read_file(platform_cache_path(), false);
  if (!content.has_value()) {
    return std::nullopt;
  }
  auto parsed = nlohmann::json::parse(content.value(), nullptr, false);
  if (parsed.is_discarded() || !parsed.contains("platform_type")) {
    return std::nullopt;
  }
  if (!parsed["platform_type"].is_number_integer()) {
    return std::nullopt;
  }
  return parsed["platform_type"].get<int>();
}

void write_cached_platform_type(int platform_type) {
  openhd::generateSettingsDirectoryIfNonExists();
  nlohmann::json payload;
  payload["platform_type"] = platform_type;
  payload["platform_name"] = x_platform_type_to_string(platform_type);
  OHDFilesystemUtil::write_file(platform_cache_path(), payload.dump(2));
}

int request_platform_from_sysutils() {
  auto cached = read_cached_platform_type();
  if (cached.has_value()) {
    return cached.value();
  }
  auto platform_opt = openhd::request_platform_type();
  if (!platform_opt.has_value()) {
    openhd::log::get_default()->warn(
        "Platform request from sysutils failed, defaulting to UNKNOWN.");
    return X_PLATFORM_TYPE_UNKNOWN;
  }
  write_cached_platform_type(platform_opt.value());
  return platform_opt.value();
}

}  // namespace

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
    case X_PLATFORM_TYPE_ORQA:
      return "ORQA";
    case X_PLATFORM_TYPE_UVX_MOD:
      return "UVX_MOD";
    case X_PLATFORM_TYPE_ALWINNER_X20:
      return "X20";
    case X_PLATFORM_TYPE_ALWINNER_CUBIE_A7Z:
      return "A733";
    case X_PLATFORM_TYPE_OPENIPC_SIGMASTAR_UNDEFINED:
      return "OPENIPC SIGMASTAR";
    case X_PLATFORM_TYPE_NVIDIA_XAVIER:
      return "NVIDIA_XAVIER";
    case X_PLATFORM_TYPE_QUALCOMM_QCS405:
      return "QUALCOMM_QCS405";
    case X_PLATFORM_TYPE_QUALCOMM_QRB5165:
      return "QUALCOMM_QRB5165";
    case X_PLATFORM_TYPE_NXP_IMX8:
      return "NXP_IMX8";
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
  if (platform_type == X_PLATFORM_TYPE_ALWINNER_X20 ||
      platform_type == X_PLATFORM_TYPE_ALWINNER_CUBIE_A7Z) {
    return 20;
  }
  if (platform_type == X_PLATFORM_TYPE_NVIDIA_XAVIER) {
    return 50;
  }
  if (platform_type == X_PLATFORM_TYPE_ORQA ||
      platform_type == X_PLATFORM_TYPE_UVX_MOD) {
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
  static OHDPlatform instance = OHDPlatform(request_platform_from_sysutils());
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

bool OHDPlatform::is_a733() const {
  return platform_type == X_PLATFORM_TYPE_ALWINNER_CUBIE_A7Z;
}

bool OHDPlatform::is_orqa() const {
  return platform_type == X_PLATFORM_TYPE_ORQA;
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

bool OHDPlatform::is_uvx_mod() const {
  return platform_type == X_PLATFORM_TYPE_UVX_MOD;
}

bool OHDPlatform::is_qrb5165() const {
  return platform_type == X_PLATFORM_TYPE_QUALCOMM_QRB5165;
}
