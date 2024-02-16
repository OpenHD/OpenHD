//
// Created by consti10 on 23.01.23.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_LAST_KNOWN_POSITION_LASTKNOWPOSITION_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_LAST_KNOWN_POSITION_LASTKNOWPOSITION_H_

#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

/**
 * This class exposes the following simple functionality:
 * Have a file on the disc that contains the X last known positions of the UAV
 * Needs to be updated by listening for MAVLINK_MSG_ID_GLOBAL_POSITION_INT
 * messages Writing to the disk happens max. 1 time per second and is decoupled
 * in an extra thread - this way, we reduce the file writes AND are guaranteed
 * data is written after a specific amount of time even if the "on_new_position"
 * is not called anymore by the telemetry parsing thread.
 */
class LastKnowPosition {
 public:
  LastKnowPosition();
  ~LastKnowPosition();
  void on_new_position(double latitude, double longitude, double altitude);

 private:
  const std::string m_this_flight_filename;
  struct Position {
    // When the position was received
    std::chrono::steady_clock::time_point recv;
    double latitude;
    double longitude;
    double altitude_m;
  };
  std::unique_ptr<std::thread> m_write_thread;
  bool m_write_run = true;
  void write_position_loop();
  std::mutex m_position_mutex;
  std::list<Position> m_positions;
  bool m_positions_updated = false;
  static constexpr auto MAX_N_BUFFERED_POSITIONS = 20;
  // Gets the last X position values or std::nullopt if there was no update
  std::optional<std::list<Position>> threadsafe_get_positions_if_updated();
  void threadsafe_update_position(Position position);
  static std::string positions_to_file(const std::list<Position>& positions);
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_LAST_KNOWN_POSITION_LASTKNOWPOSITION_H_
