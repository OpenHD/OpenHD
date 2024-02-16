//
// Created by consti10 on 07.11.22.
//
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#include "RcJoystickSender.h"

#include <utility>

RcJoystickSender::RcJoystickSender(SEND_MESSAGE_CB cb, int update_rate_hz,
                                   openhd::CHAN_MAP chan_map)
    : m_cb(std::move(cb)),
      m_delay_in_milliseconds(1000 / update_rate_hz),
      m_chan_map(chan_map) {
  if (!openhd::validate_channel_mapping(chan_map)) {
    openhd::log::get_default()->warn("Invalid channel mapping");
    m_chan_map = openhd::get_default_channel_mapping();
  }
  m_joystick_reader = std::make_unique<JoystickReader>();
  m_send_data_thread =
      std::make_unique<std::thread>([this] { send_data_until_terminate(); });
}

void RcJoystickSender::send_data_until_terminate() {
  while (!terminate) {
    const auto curr = m_joystick_reader->get_current_state();
    // We only send data if the joystick is in the connected state
    // Otherwise, we just stop sending data, which should result in a failsafe
    // at the FC.
    if (curr.considered_connected) {
      // map all the channels before we send them out
      // mapping might change at any time, and the compute overhead - well, we
      // are not on a microcontroller ;)
      auto curr_mapping = get_current_channel_mapping();
      auto mapped_channels = openhd::remap_channels(curr.values, curr_mapping);
      m_cb(mapped_channels);
    }
    std::this_thread::sleep_for(
        std::chrono::milliseconds(m_delay_in_milliseconds));
  }
}

RcJoystickSender::~RcJoystickSender() {
  terminate = true;
  m_send_data_thread->join();
  m_send_data_thread.reset();
  m_joystick_reader.reset();
}

void RcJoystickSender::change_update_rate(int update_rate_hz) {
  const int val = (1000 / update_rate_hz);
  if (val >= 0) {
    m_delay_in_milliseconds = val;
  } else {
    openhd::log::get_default()->warn("Invalid update rate hz {}",
                                     update_rate_hz);
  }
}

void RcJoystickSender::update_channel_mapping(
    const openhd::CHAN_MAP& new_chan_map) {
  std::lock_guard<std::mutex> guard(m_chan_map_mutex);
  if (!openhd::validate_channel_mapping(new_chan_map)) {
    openhd::log::get_default()->warn("Invalid channel mapping");
    return;
  }
  m_chan_map = new_chan_map;
}

openhd::CHAN_MAP RcJoystickSender::get_current_channel_mapping() {
  std::lock_guard<std::mutex> guard(m_chan_map_mutex);
  return m_chan_map;
}

#endif  // OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
