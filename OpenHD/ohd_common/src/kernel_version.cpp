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

#include "kernel_version.h"

#include <regex>
#include <sys/utsname.h>

namespace openhd {

static std::regex version_regex{"([0-9]+)\\.([0-9]+)\\.([0-9]+).*"};

std::optional<KernelVersion> get_kernel_version() {
  struct utsname uts {};
  if (uname(&uts) != 0) return std::nullopt;

  std::cmatch matches;
  if (!std::regex_match(uts.release, matches, version_regex)) {
    return std::nullopt;
  }

  KernelVersion version;
  version.major = std::stoi(matches[1]);
  version.minor = std::stoi(matches[2]);
  version.patch = std::stoi(matches[3]);
  return version;
}

bool is_kernel_major(int expected_major) {
  const auto version = get_kernel_version();
  return version.has_value() && version->major == expected_major;
}

std::string kernel_version_to_string(const KernelVersion& version) {
  return std::to_string(version.major) + "." + std::to_string(version.minor) +
         "." + std::to_string(version.patch);
}

}  // namespace openhd

