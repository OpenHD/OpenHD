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

#ifndef OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_NAMING_H_
#define OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_NAMING_H_

#include <string>

namespace openhd::naming {

/**
 * Returns the postfix configured by the user via name.txt (if available).
 */
std::string get_unit_name_postfix();

/**
 * Returns the default unit name (openhd_air or openhd_ground) with an optional
 * postfix read from name.txt appended with an underscore.
 */
std::string build_unit_name(bool is_air);

}  // namespace openhd::naming

#endif  // OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_NAMING_H_
