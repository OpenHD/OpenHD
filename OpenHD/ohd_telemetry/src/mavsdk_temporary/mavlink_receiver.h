#pragma once

#include <cstdint>

#include "mavlink_include.h"
#include "mavsdk_time.h"

namespace mavsdk {

class MAVLinkReceiver {
 public:
  explicit MAVLinkReceiver(uint8_t channel);

  [[nodiscard]] uint8_t get_channel() const { return _channel; }

  mavlink_message_t& get_last_message() { return _last_message; }

  mavlink_status_t& get_status() { return _status; }

  void set_new_datagram(char* datagram, unsigned datagram_len);

  bool parse_message();

  void debug_drop_rate();
  void print_line(const char* index, uint64_t count, uint64_t count_total,
                  uint64_t overall_bytes, uint64_t overall_bytes_total);

 private:
  uint8_t _channel;
  mavlink_message_t _last_message = {};
  mavlink_status_t _status = {};
  char* _datagram = nullptr;
  unsigned _datagram_len = 0;

  Time _time{};

  bool _drop_debugging_on{false};

  struct {
    uint64_t bytes_received{0};
    uint64_t bytes_sent_overall{0};
    uint64_t bytes_at_camera_overall{0};
    uint64_t bytes_at_sdk_overall{0};
    bool first{true};
    dl_time_t last_time{};
    double time_elapsed{0.0};
  } _drop_stats{};
};

}  // namespace mavsdk
