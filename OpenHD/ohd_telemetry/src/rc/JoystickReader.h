//
// Created by consti10 on 22.08.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_

#include <array>
#include <functional>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>
#include "openhd-spdlog.hpp"

/**
 * The Paradigma of this class is similar to how for example external devices
 * are handled in general in OpenHD: If the user says he wants RC joystick
 * control, try to open the joystick and read data, re-connect if anything goes
 * wrong during run time.
 */
class JoystickReader {
 public:
  // Called every time there is new joystick data
  typedef std::function<void(std::array<uint16_t,16> data)> NEW_JOYSTICK_DATA_CB;
  // thread-safe. Fetch new updated joystick values if there is any.
  //std::optional<std::array<uint16_t,16>> get_new_data_if_available();
 public:
  explicit JoystickReader(NEW_JOYSTICK_DATA_CB cb= nullptr);
  ~JoystickReader();
  struct CurrChannelValues{
    std::array<uint16_t,16> values;
    // Time point when we received the last update to at least one of the channel(s)
    std::chrono::steady_clock::time_point last_update;
    bool considered_connected=false;
  };
  CurrChannelValues get_current_state();
  static std::string curr_state_to_string(const CurrChannelValues& curr_channel_values){
    std::stringstream ss;
    ss<<"Connected:"<<(curr_channel_values.considered_connected ? "Y":"N")<<"\n";
    ss<<"Values:[";
    for(int i=0;i<curr_channel_values.values.size();i++){
      ss<<(int)curr_channel_values.values[i];
      if(i==curr_channel_values.values.size()-1){
        ss<<"]\n";
      }else{
        ss<<",";
      }
    }
    const auto delay_since_last_update=std::chrono::steady_clock::now()-curr_channel_values.last_update;
    ss<<"Delay since last update:"<<std::chrono::duration_cast<std::chrono::milliseconds>(delay_since_last_update).count()<<"ms";
    return ss.str();
  }
 private:
  void loop();
  void connect_once_and_read_until_error();
  int read_events_until_empty();
  std::unique_ptr<std::thread> m_read_joystick_thread;
  // stores actual joystick data
  std::array<uint16_t,16> m_curr_joystick_data;
  const NEW_JOYSTICK_DATA_CB m_cb;
  bool terminate=false;
  std::mutex m_curr_values_mutex;
  CurrChannelValues m_curr_values;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_
