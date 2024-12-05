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

#include "openhd_profile.h"

#include <sstream>

#include "openhd_settings_directories.h"
#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util_filesystem.h"

static constexpr auto PROFILE_MANIFEST_FILENAME = "/tmp/profile_manifest.txt";

void write_profile_manifest(const OHDProfile& ohdProfile) {
  std::stringstream ss;
  ss << "AIR:" << (ohdProfile.is_air ? "YES" : "NO") << "\n";
  ss << "UNIT_ID:" << ohdProfile.unit_id << "\n";
  OHDFilesystemUtil::write_file(PROFILE_MANIFEST_FILENAME, ss.str());
}

OHDProfile DProfile::discover(bool is_air) {
  openhd::log::get_default()->debug("Profile:[{}]", is_air ? "AIR" : "GND");
  // We read the unit id from the persistent storage, later write it to the tmp
  // storage json
  const auto unit_id = openhd::getOrCreateUnitId();
  return OHDProfile(is_air, unit_id);
}
