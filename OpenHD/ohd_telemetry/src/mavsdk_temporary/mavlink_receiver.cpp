#include "mavlink_receiver.h"

#include <iomanip>

#include "log.h"

namespace mavsdk {

MAVLinkReceiver::MAVLinkReceiver(uint8_t channel) : _channel(channel) {
  if (const char* env_p = std::getenv("MAVSDK_DROP_DEBUGGING")) {
    if (std::string(env_p) == "1") {
      LogDebug() << "Drop debugging is on.";
      _drop_debugging_on = true;
    }
  }
}

void MAVLinkReceiver::set_new_datagram(char* datagram, unsigned datagram_len) {
  _datagram = datagram;
  _datagram_len = datagram_len;

  if (_drop_debugging_on) {
    _drop_stats.bytes_received += _datagram_len;
  }
}

bool MAVLinkReceiver::parse_message() {
  // Note that one datagram can contain multiple mavlink messages.
  for (unsigned i = 0; i < _datagram_len; ++i) {
    if (mavlink_parse_char(_channel, _datagram[i], &_last_message, &_status) ==
        1) {
      // Move the pointer to the datagram forward by the amount parsed.
      _datagram += (i + 1);
      // And decrease the length, so we don't overshoot in the next round.
      _datagram_len -= (i + 1);

      if (_drop_debugging_on) {
        debug_drop_rate();
      }

      // We have parsed one message, let's return, so it can be handled.
      return true;
    }
  }

  // No (more) messages, let's give up.
  _datagram = nullptr;
  _datagram_len = 0;
  return false;
}

void MAVLinkReceiver::debug_drop_rate() {
  if (_last_message.msgid == MAVLINK_MSG_ID_SYS_STATUS) {
    const unsigned msg_len =
        (_last_message.len + MAVLINK_NUM_NON_PAYLOAD_BYTES);

    _drop_stats.bytes_received -= msg_len;

    mavlink_sys_status_t sys_status;
    mavlink_msg_sys_status_decode(&_last_message, &sys_status);

    if (!_drop_stats.first) {
      LogDebug() << "----------------------------------------------------------"
                    "---------"
                 << "-----------";

      if (_drop_stats.bytes_received <= sys_status.errors_comm &&
          sys_status.errors_count2 <= sys_status.errors_comm) {
        _drop_stats.bytes_sent_overall += sys_status.errors_comm;
        //_bytes_at_gimbal_overall += sys_status.errors_count1;
        _drop_stats.bytes_at_camera_overall += sys_status.errors_count2;
        _drop_stats.bytes_at_sdk_overall += _drop_stats.bytes_received;

        _drop_stats.time_elapsed +=
            _time.elapsed_since_s(_drop_stats.last_time);

        print_line("FMU   ", sys_status.errors_comm, sys_status.errors_comm,
                   _drop_stats.bytes_sent_overall,
                   _drop_stats.bytes_sent_overall);

        print_line("Camera", sys_status.errors_count2, sys_status.errors_comm,
                   _drop_stats.bytes_at_camera_overall,
                   _drop_stats.bytes_sent_overall);

        print_line("SDK   ", _drop_stats.bytes_received, sys_status.errors_comm,
                   _drop_stats.bytes_at_sdk_overall,
                   _drop_stats.bytes_sent_overall);

      } else {
        LogDebug() << "Missed SYS_STATUS";
      }
    }
    _drop_stats.first = false;

    _drop_stats.last_time = _time.steady_time();

    // Reset afterwards:
    _drop_stats.bytes_received = msg_len;
  }
}

void MAVLinkReceiver::print_line(const char* index, uint64_t count,
                                 uint64_t count_total, uint64_t overall_bytes,
                                 uint64_t overall_bytes_total) {
  LogDebug() << "count " << index << ": " << std::setw(6) << count
             << ", loss: " << std::setw(6) << count_total - count << ",  "
             << std::setw(6) << std::setprecision(2) << std::fixed
             << 100.0f * float(count) / float(count_total)
             << " %, overall: " << std::setw(6) << std::setprecision(2)
             << std::fixed
             << (100.0f * float(overall_bytes) / float(overall_bytes_total))
             << " %, " << std::setw(6) << std::setprecision(2) << std::fixed
             << (float(overall_bytes) / float(_drop_stats.time_elapsed) /
                 1024.0f)
             << " KiB/s";
}

}  // namespace mavsdk
