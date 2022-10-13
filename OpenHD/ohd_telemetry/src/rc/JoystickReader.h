//
// Created by consti10 on 22.08.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_

#include <SDL/SDL.h>

#include <thread>
#include <array>
#include <functional>
#include <optional>

/**
 * The Paradigma of this class is similar to how for example external devices are handled in general in OpenHD:
 * If the user says he wants RC joystick control, try to open the joystick and read data,
 * re-connect if anything goes wrong during run time.
 */
class JoystickReader {
 public:
  // Called every time there is new joystick data
  typedef std::function<void(std::array<uint16_t,16> data)> NEW_JOYSTICK_DATA_CB;
  // thread-safe. Fetch new updated joystick values if there is any.
  std::optional<std::array<uint16_t,16>> get_new_data_if_available();
 public:
  explicit JoystickReader(NEW_JOYSTICK_DATA_CB cb= nullptr);
  ~JoystickReader();
 private:
  void loop();
  void connect_once_and_read_until_error();
  std::unique_ptr<std::thread> m_read_joystick_thread;
  // stores actual joystick data
  std::array<uint16_t,16> m_curr_joystick_data;
  const NEW_JOYSTICK_DATA_CB m_cb;
 private:
  bool terminate=false;
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_
