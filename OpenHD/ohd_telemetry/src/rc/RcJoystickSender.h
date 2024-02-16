//
// Created by consti10 on 07.11.22.
//
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_

#include <atomic>
#include <memory>

#include "../mav_helper.h"
#include "ChannelMappingUtil.hpp"
#include "JoystickReader.h"

// Really simple, have a thread that send out telemetry RC data at a fixed rate
// (we cannot just use the thread that fetches data from the joystick, at least
// not for now)
class RcJoystickSender {
 public:
  // This callback is called in regular intervalls with valid rc channel data as
  // long as there is a joystick connected & well. If there is something wrong
  // with the joystick / no joystick connected this cb is not called (such that
  // FC can do failsafe)
  typedef std::function<void(std::array<uint16_t, 18> channels)>
      SEND_MESSAGE_CB;
  RcJoystickSender(SEND_MESSAGE_CB cb, int update_rate_hz,
                   openhd::CHAN_MAP chan_map);
  ~RcJoystickSender();
  // atomic, can be called from any thread.
  void change_update_rate(int update_rate_hz);
  // update the channel mapping, thread-safe
  void update_channel_mapping(const openhd::CHAN_MAP& new_chan_map);

 private:
  // get the current channel mapping, thread-safe
  openhd::CHAN_MAP get_current_channel_mapping();
  void send_data_until_terminate();
  std::unique_ptr<JoystickReader> m_joystick_reader;
  std::unique_ptr<std::thread> m_send_data_thread;
  const SEND_MESSAGE_CB m_cb;
  // Controls the update rate how often we send the rc packets to the air unit.
  // We can just use std::atomic for thread safety here
  std::atomic<int> m_delay_in_milliseconds;
  bool terminate = false;

 private:
  std::mutex m_chan_map_mutex;
  openhd::CHAN_MAP m_chan_map;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_
#endif  // OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND