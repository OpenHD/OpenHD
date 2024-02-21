//
// Created by consti10 on 23.01.23.
//

#include "LastKnowPosition.h"

#include <iomanip>

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"
#include "openhd_util_filesystem.h"

static constexpr auto LAST_KNOWN_POSITION_DIRECTORY =
    "/home/openhd/LastKnownPosition/";
static constexpr auto FILENAME = "flight.txt";

static std::string get_this_flight_filename() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::stringstream ss;
  ss << LAST_KNOWN_POSITION_DIRECTORY << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S")
     << ".txt";
  return ss.str();
}

LastKnowPosition::LastKnowPosition()
    //: m_directory(get_this_flight_directory())
    : m_this_flight_filename(get_this_flight_filename()) {
  openhd::log::get_default()->debug("Writing position to [{}]",
                                    m_this_flight_filename);
  OHDFilesystemUtil::create_directories(LAST_KNOWN_POSITION_DIRECTORY);
  m_write_thread =
      std::make_unique<std::thread>([this]() { this->write_position_loop(); });
}

LastKnowPosition::~LastKnowPosition() {
  m_write_run = false;
  m_write_thread->join();
  m_write_thread = nullptr;
}

void LastKnowPosition::on_new_position(double latitude, double longitude,
                                       double altitude) {
  // bad data - do not pollute
  if (latitude == 0.0 || longitude == 0.0) {
    return;
  }
  threadsafe_update_position(LastKnowPosition::Position{
      std::chrono::steady_clock::now(), latitude, longitude, altitude});
}

void LastKnowPosition::write_position_loop() {
  while (m_write_run) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Get the last X positions (if there is no update,aka no new data or crash,
    // we get nullopt)
    auto positions_opt = threadsafe_get_positions_if_updated();
    if (positions_opt == std::nullopt) {
      // No update
      continue;
    }
    const auto positions = positions_opt.value();
    assert(!positions.empty());
    const auto content = positions_to_file(positions);
    // and update the file
    OHDFilesystemUtil::write_file(m_this_flight_filename, content);
  }
}

std::optional<std::list<LastKnowPosition::Position>>
LastKnowPosition::threadsafe_get_positions_if_updated() {
  std::lock_guard<std::mutex> guard(m_position_mutex);
  if (m_positions_updated) {
    m_positions_updated = false;
    return m_positions;
  }
  return std::nullopt;
}

void LastKnowPosition::threadsafe_update_position(
    LastKnowPosition::Position position) {
  std::lock_guard<std::mutex> guard(m_position_mutex);
  if (m_positions.size() >= MAX_N_BUFFERED_POSITIONS) {
    // remove the oldest position
    m_positions.pop_front();
  }
  // Add the new position
  m_positions.push_back(position);
  // mark new data available
  m_positions_updated = true;
}

std::string LastKnowPosition::positions_to_file(
    const std::list<Position>& positions) {
  std::stringstream ss;
  for (const auto& position : positions) {
    ss << fmt::format("Lat:{},Lon:{},Alt:{}\n", position.latitude,
                      position.longitude, position.altitude_m);
  }
  return ss.str();
}
