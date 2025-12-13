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

#pragma once

#include <optional>
#include <string>

namespace openhd {

struct KernelVersion {
  int major{0};
  int minor{0};
  int patch{0};
};

// Parse uname(2) release into a semantic version (major.minor.patch).
std::optional<KernelVersion> get_kernel_version();

// Convenience helper for simple selection logic.
bool is_kernel_major(int expected_major);

// Convert to human readable string for logs / debugging.
std::string kernel_version_to_string(const KernelVersion& version);

}  // namespace openhd

