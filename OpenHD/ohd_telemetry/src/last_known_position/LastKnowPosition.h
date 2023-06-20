//
// Created by consti10 on 23.01.23.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_LAST_KNOWN_POSITION_LASTKNOWPOSITION_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_LAST_KNOWN_POSITION_LASTKNOWPOSITION_H_

#include <string>
#include <chrono>

/**
 * This class exposes the following simple functionality:
 * Constantly listen for updates from the FC in regards to its position
 * (aka MAVLINK_MSG_ID_GLOBAL_POSITION_INT messages) and write the coordinates
 * to a file overwriting the previous position. This way, if the drone crashes,
 * the user can always get the last known position from a file in openhd.
 * TODO
 */
class LastKnowPosition {
 public:
  LastKnowPosition();
  void on_new_position(double latitude,double longitude,double altitude);
 private:
  const std::string m_directory;
  std::chrono::steady_clock::time_point m_last_position_update=std::chrono::steady_clock::now();
  int m_update_index=0;
};

#endif  // OPENHD_OPENHD_OHD_TELEMETRY_SRC_LAST_KNOWN_POSITION_LASTKNOWPOSITION_H_
