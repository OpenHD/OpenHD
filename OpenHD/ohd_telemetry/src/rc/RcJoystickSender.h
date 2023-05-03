//
// Created by consti10 on 07.11.22.
//
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_

#include "JoystickReader.h"
#include "../mav_helper.h"
#include "openhd_spdlog.h"
#include <memory>
#include <atomic>

// Really simple, have a thread that send out telemetry RC data at a fixed rate
// (we cannot just use the thread that fetches data from the joystick, at least not for now)
class RcJoystickSender {
 public:
  // This callback is called in regular intervalls with valid rc channel data as long as there is a joystick connected & well.
  // If there is something wrong with the joystick / no joystick connected this cb is not called (such that FC can do failsafe)
  typedef std::function<void(std::array<uint16_t,18> channels)> SEND_MESSAGE_CB;
  RcJoystickSender(SEND_MESSAGE_CB cb,int update_rate_hz,JoystickReader::CHAN_MAP chan_map);
  ~RcJoystickSender();
  // Enables sending of rc joystick data IF joystick is connected & alive
  // If no joystick is connected, this does nothing until a joystick is connected
  void start();
  // stops sending joystick data
  void stop();
  void change_update_rate(int update_rate_hz);
  // update the channel mapping, thread-safe
  void update_channel_maping(const JoystickReader::CHAN_MAP& new_chan_map);
 private:
  void send_data_until_terminate();
  std::shared_ptr<spdlog::logger> m_console;
  std::unique_ptr<JoystickReader> m_joystick_reader;
  std::unique_ptr<std::thread> m_send_data_thread= nullptr;
  const SEND_MESSAGE_CB m_cb;
  std::atomic<int> m_delay_in_milliseconds;
  JoystickReader::CHAN_MAP m_chan_map;
  bool terminate=false;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_RCJOYSTICKSENDER_H_
#endif //OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND