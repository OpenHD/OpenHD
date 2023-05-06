//
// Created by consti10 on 07.11.22.
//
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_

#include "JoystickReader.h"
#include "../mav_helper.h"
#include <memory>
#include "ChannelMappingUtil.hpp"

// Really simple, have a thread that send out telemetry RC data at a fixed rate
// (we cannot just use the thread that fetches data from the joystick, at least not for now)
class RcJoystickSender {
 public:
  // This callback is called in regular intervalls with valid rc channel data as long as there is a joystick connected & well.
  // If there is something wrong with the joystick / no joystick connected this cb is not called (such that FC can do failsafe)
  typedef std::function<void(std::array<uint16_t,18> channels)> SEND_MESSAGE_CB;
  RcJoystickSender(SEND_MESSAGE_CB cb,int update_rate_hz,openhd::CHAN_MAP chan_map);
  void change_update_rate(int update_rate_hz);
  // update the channel mapping, thread-safe
  void update_channel_maping(const openhd::CHAN_MAP& new_chan_map);
  ~RcJoystickSender();
 private:
  void send_data_until_terminate();
  std::unique_ptr<JoystickReader> m_joystick_reader;
  std::unique_ptr<std::thread> m_send_data_thread;
  const SEND_MESSAGE_CB m_cb;
  int m_delay_in_milliseconds;
  bool terminate=false;
 private:

};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_
#endif //OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND