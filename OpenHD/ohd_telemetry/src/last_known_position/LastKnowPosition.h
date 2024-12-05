/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

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
