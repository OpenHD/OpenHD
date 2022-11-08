//
// Created by consti10 on 07.11.22.
//
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_

#include "JoystickReader.h"
#include "../mav_helper.h"
#include <memory>

// Really simple, have a thread that send out telemetry RC data at a fixed rate
// (we cannot just use the thread that fetches data from the joystick, at least not for now)
class RcJoystickSender {
 public:
  typedef std::function<void(const MavlinkMessage& msg)> SEND_MESSAGE_CB;
  RcJoystickSender(SEND_MESSAGE_CB cb,int update_rate_hz);
  void change_update_rate(int update_rate_hz);
  ~RcJoystickSender();
 private:
  void send_data_until_terminate();
  std::unique_ptr<JoystickReader> m_joystick_reader;
  std::unique_ptr<std::thread> m_send_data_thread;
  const SEND_MESSAGE_CB m_cb;
  int m_delay_in_milliseconds;
  bool terminate=false;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_
#endif //OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND