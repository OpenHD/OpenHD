//
// Created by consti10 on 22.08.22.
//
#ifdef OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_

#include <array>
#include <functional>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>

#include "openhd_spdlog.hpp"
#include "openhd_util.hpp"

/**
 * The Paradigm of this class is similar to how for example external devices
 * are handled in general in OpenHD: If the user says he wants RC joystick
 * control, try to open the joystick and read data, re-connect if anything goes
 * wrong during run time. This class does all the connecting and handles disconnecting
 * and reading values in its own thread - you can query a "state" from any thread at any
 * time though.
 * Theoretically, we could just use this thread also for sending the RC data via mavlink - but this
 * is a bit dangerous, since I don't completely trust SDL yet (in regards to disconnecting joysticks).
 */
class JoystickReader {
 public:
  // See mavlink RC override https://mavlink.io/en/messages/common.html#RC_CHANNELS_OVERRIDE
  static constexpr uint16_t DEFAULT_RC_CHANNELS_VALUE=UINT16_MAX;
  // the rc channel override message(s) support 18 values, so we do so, too
  static constexpr auto N_CHANNELS=18;
  static constexpr auto N_CHANNELS_RESERVED_FOR_AXES=8;
  static constexpr uint16_t VALUE_BUTTON_UP=2000;
  static constexpr uint16_t VALUE_BUTTON_DOWN=1000;
  struct CurrChannelValues{
    std::array<uint16_t,N_CHANNELS> values{DEFAULT_RC_CHANNELS_VALUE};
    // Time point when we received the last update to at least one of the channel(s)
    std::chrono::steady_clock::time_point last_update;
    // Weather we think the RC (joystick) is currently connected or not.
    bool considered_connected=false;
    // the name of the joystick
    std::string joystick_name="unknown";
  };
  // Channel mapping: just look at the default to understand ;)
  using CHAN_MAP=std::array<int,N_CHANNELS_RESERVED_FOR_AXES>;
  explicit JoystickReader(CHAN_MAP chan_map);
  ~JoystickReader();
  // Get the current "state", thread-safe
  CurrChannelValues get_current_state();
  // update the channel mapping, thread-safe
  void update_channel_maping(const CHAN_MAP& new_chan_map);
  // For debugging
  static std::string curr_state_to_string(const CurrChannelValues& curr_channel_values);
  // (custom) channel mapping - rudimentary, since one can do that just as well on the FC
  static std::optional<CHAN_MAP> convert_string_to_channel_mapping(const std::string& input);
  static bool validate_channel_mapping(const CHAN_MAP& chan_map);
  static std::array<int,N_CHANNELS_RESERVED_FOR_AXES> get_default_channel_mapping();
 private:
  void loop();
  void connect_once_and_read_until_error();
  // Wait up to timeout_ms for an event, and then read as many events as there are available
  // We are only interested in the Joystick events
  void wait_for_events(int timeout_ms);
  int process_event(void* event,std::array<uint16_t,N_CHANNELS>& values);
  void reset_curr_values();
  std::unique_ptr<std::thread> m_read_joystick_thread;
  bool terminate=false;
  std::mutex m_curr_values_mutex;
  CurrChannelValues m_curr_values;
  std::shared_ptr<spdlog::logger> m_console;
  std::mutex m_chan_map_mutex;
  CHAN_MAP m_chan_map{};
 private:
  // just taken from previous openhd
  static uint16_t parsetoMultiWii(int16_t value);
  void write_matching_axis(std::array<uint16_t,JoystickReader::N_CHANNELS>& rc_data,uint8_t axis_index,int16_t value);
  static void write_matching_button(std::array<uint16_t,18>&rc_data,uint8_t button,bool up);
  std::optional<int> get_mapped_axis(int axis_index);
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_
#endif //OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND
