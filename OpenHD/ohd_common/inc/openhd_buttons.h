//
// Created by consti10 on 02.02.24.
//

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
