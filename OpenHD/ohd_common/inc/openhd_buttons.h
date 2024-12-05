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

#ifndef OPENHD_OPENHD_BUTTONS_H
#define OPENHD_OPENHD_BUTTONS_H

namespace openhd {

/**
 * Similar to openhd_led, openhd_buttons abstracts away hw-differences regarding
 * buttons. (Or stuff similar to buttons) Some functionalities might not be
 * supported on some hardware types, or a button might actually be a gpio jumper
 * for now.
 */
class ButtonManager {
 public:
  static ButtonManager& instance();
  /**
   * Called once at boot. Returns true if the button refering to the
   * 'Clean all settings / reset openhd core' functionality is pressed
   */
  bool user_wants_reset_openhd_core();

 private:
  explicit ButtonManager() = default;
};

}  // namespace openhd
class openhd_buttons {};

#endif  // OPENHD_OPENHD_BUTTONS_H
