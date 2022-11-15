//
// Created by consti10 on 07.11.22.
//
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#include "RcJoystickSender.h"

#include <utility>

RcJoystickSender::RcJoystickSender(SEND_MESSAGE_CB cb,int update_rate_hz,JoystickReader::CHAN_MAP chan_map):
m_cb(std::move(cb)),m_delay_in_milliseconds(1000/update_rate_hz) {
  m_joystick_reader=std::make_unique<JoystickReader>(chan_map);
  m_send_data_thread=std::make_unique<std::thread>([this] {
    send_data_until_terminate();
  });
}

void RcJoystickSender::send_data_until_terminate() {
  while (!terminate){
    const auto curr=m_joystick_reader->get_current_state();
    // We only send data if the joystick is in the connected state
    // Otherwise, we just stop sending data, which should result in a failsafe at the FC.
    if(curr.considered_connected){
      auto msg= pack_rc_message(OHD_SYS_ID_GROUND,0,curr.values,0,0);
      m_cb(msg);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(m_delay_in_milliseconds));
  }
}

RcJoystickSender::~RcJoystickSender() {
  terminate= true;
  m_send_data_thread->join();
  m_send_data_thread.reset();
  m_joystick_reader.reset();
}

void RcJoystickSender::change_update_rate(int update_rate_hz) {
  const int val=(1000/update_rate_hz);
  if(val>=0){
    m_delay_in_milliseconds=val;
  }else{
    openhd::log::get_default()->warn("Invalid update rate hz {]",update_rate_hz);
  }
}

void RcJoystickSender::update_channel_maping(const JoystickReader::CHAN_MAP& new_chan_map) {
  m_joystick_reader->update_channel_maping(new_chan_map);
}

#endif //OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
