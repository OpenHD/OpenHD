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

#include "openhd_naming.h"

#include <fmt/core.h>

#include "config_paths.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

namespace openhd::naming {

static std::string get_name_file_path() {
  return std::string(getConfigBasePath()) + "name.txt";
}

std::string get_unit_name_postfix() {
  const auto name_file = get_name_file_path();
  if (!OHDFilesystemUtil::exists(name_file)) {
    return "";
  }
  auto postfix = OHDFilesystemUtil::read_file(name_file);
  OHDUtil::trim(postfix);
  return postfix;
}

std::string build_unit_name(const bool is_air) {
  const std::string base_name = is_air ? "openhd_air" : "openhd_ground";
  const auto postfix = get_unit_name_postfix();
  if (postfix.empty()) {
    return base_name;
  }
  return fmt::format("{}_{}", base_name, postfix);
}

}  // namespace openhd::naming
