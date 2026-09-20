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

#include "AdsbComponent.h"

#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <signal.h>
#include <spawn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "openhd_util.h"
#include <spdlog/spdlog.h>

extern char** environ;

AdsbComponent::AdsbComponent(uint8_t parent_sys_id)
    : MavlinkComponent(parent_sys_id, MAV_COMP_ID_ADSB),
      m_console(openhd::log::create_or_get("ADSB")) {
  m_process_thread = std::thread(&AdsbComponent::process_runner, this);
  m_tcp_thread = std::thread(&AdsbComponent::tcp_client_runner, this);
}

AdsbComponent::~AdsbComponent() {
  m_terminate = true;
  const auto pid = m_dump1090_pid.load();
  if (pid > 0) {
    // Only stop the helper owned by this component. Never kill unrelated
    // dump1090 processes that the user may be running.
    kill(pid, SIGTERM);
  }
  if (m_process_thread.joinable()) {
    m_process_thread.join();
  }
  if (m_tcp_thread.joinable()) {
    m_tcp_thread.join();
  }
}

void AdsbComponent::process_runner() {
  while (!m_terminate) {
    m_console->info("Starting dump1090...");
    char program[] = "openhd_dump1090";
    char net[] = "--net";
    char quiet[] = "--quiet";
    char port_option[] = "--net-sbs-port";
    char port[] = "30003";
    char* argv[] = {program, net, quiet, port_option, port, nullptr};
    pid_t child_pid = -1;
    const int spawn_result =
        posix_spawnp(&child_pid, program, nullptr, nullptr, argv, environ);
    if (spawn_result != 0) {
      m_console->warn("Cannot start bundled dump1090: {}", strerror(spawn_result));
      if (!wait_for_retry()) break;
      continue;
    }
    m_dump1090_pid = child_pid;
    if (m_terminate) {
      kill(child_pid, SIGTERM);
    }

    int status = 0;
    while (waitpid(child_pid, &status, 0) < 0 && errno == EINTR) {
    }
    m_dump1090_pid = -1;
    if (WIFEXITED(status)) {
      m_console->info("dump1090 exited with code {}", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      m_console->info("dump1090 stopped by signal {}", WTERMSIG(status));
    }

    if (m_terminate) break;
    if (!wait_for_retry()) break;
  }
}

bool AdsbComponent::wait_for_retry() const {
  for (int i = 0; i < 50 && !m_terminate; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return !m_terminate;
}

void AdsbComponent::tcp_client_runner() {
  while (!m_terminate) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
      if (!wait_for_retry()) break;
      continue;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(30003);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
      close(sock);
      if (!wait_for_retry()) break;
      continue;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      close(sock);
      if (!wait_for_retry()) break;
      continue;
    }

    m_console->info("Connected to dump1090 on 127.0.0.1:30003");

    // Timeout so we can check m_terminate
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    std::string buffer = "";
    char recv_buf[1024];

    while (!m_terminate) {
      int bytes_read = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
      if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          continue;
        }
        break; // Error or disconnect
      } else if (bytes_read == 0) {
        break; // Disconnect
      }

      recv_buf[bytes_read] = '\0';
      buffer += recv_buf;

      size_t pos;
      while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string line = buffer.substr(0, pos);
        OHDUtil::trim(line);
        buffer.erase(0, pos + 1);

        if (line.empty()) continue;

        // Parse SBS format
        // Example: MSG,3,111,11111,392B74,111111,2023/10/11,12:00:00.000,2023/10/11,12:00:00.000,,40000,,,48.0,-122.0,,,,,,0
        auto parts = OHDUtil::split_into_substrings(line, ',');
        if (parts.size() >= 22 && parts[0] == "MSG") {
          std::string msg_type = parts[1];
          std::string hex_id = parts[4];

          auto icao_opt = OHDUtil::string_to_long_hex(hex_id);
          if (!icao_opt) continue;
          uint32_t icao = icao_opt.value();

          std::lock_guard<std::mutex> lock(m_adsb_mutex);
          if (m_adsb_vehicles.find(icao) == m_adsb_vehicles.end()) {
            mavlink_adsb_vehicle_t v{};
            v.ICAO_address = icao;
            // set empty callsign
            memset(v.callsign, 0, sizeof(v.callsign));
            v.altitude_type = ADSB_ALTITUDE_TYPE_PRESSURE_QNH;
            v.emitter_type = ADSB_EMITTER_TYPE_NO_INFO;
            v.flags = 0;
            m_adsb_vehicles[icao] = v;
          }

          mavlink_adsb_vehicle_t& v = m_adsb_vehicles[icao];
          m_last_seen[icao] = std::chrono::steady_clock::now();

          if (msg_type == "1") {
            // MSG,1 - Callsign
            std::string callsign = parts[10];
            OHDUtil::trim(callsign);
            if (!callsign.empty()) {
              memset(v.callsign, 0, sizeof(v.callsign));
              strncpy(v.callsign, callsign.c_str(), sizeof(v.callsign) - 1);
              v.flags |= ADSB_FLAGS_VALID_CALLSIGN;
            }
          } else if (msg_type == "3") {
            // MSG,3 - Airborne Position
            auto alt_opt = OHDUtil::string_to_int(parts[11]);
            auto lat_opt = OHDUtil::string_to_float(parts[14]);
            auto lon_opt = OHDUtil::string_to_float(parts[15]);

            if (alt_opt) {
              const auto altitude_mm = static_cast<int64_t>(alt_opt.value()) * 3048 / 10;
              v.altitude = static_cast<int32_t>(std::clamp<int64_t>(
                  altitude_mm, std::numeric_limits<int32_t>::min(),
                  std::numeric_limits<int32_t>::max()));
              v.flags |= ADSB_FLAGS_VALID_ALTITUDE;
            }
            if (lat_opt && lon_opt && lat_opt.value() >= -90.0f &&
                lat_opt.value() <= 90.0f && lon_opt.value() >= -180.0f &&
                lon_opt.value() <= 180.0f) {
              v.lat = static_cast<int32_t>(lat_opt.value() * 1e7);
              v.lon = static_cast<int32_t>(lon_opt.value() * 1e7);
              v.flags |= ADSB_FLAGS_VALID_COORDS;
            }
          } else if (msg_type == "4") {
            // MSG,4 - Airborne Velocity
            auto speed_opt = OHDUtil::string_to_float(parts[12]); // knots
            auto track_opt = OHDUtil::string_to_float(parts[13]); // heading
            auto vert_rate_opt = OHDUtil::string_to_float(parts[16]); // ft/min

            if (speed_opt && speed_opt.value() >= 0.0f) {
              v.hor_velocity = static_cast<uint16_t>(std::clamp(
                  speed_opt.value() * 51.4444f, 0.0f,
                  static_cast<float>(std::numeric_limits<uint16_t>::max())));
              v.flags |= ADSB_FLAGS_VALID_VELOCITY;
            }
            if (track_opt && track_opt.value() >= 0.0f &&
                track_opt.value() < 360.0f) {
              v.heading = static_cast<uint16_t>(track_opt.value() * 100.0f);
              v.flags |= ADSB_FLAGS_VALID_HEADING;
            }
            if (vert_rate_opt) {
              v.ver_velocity = static_cast<int16_t>(std::clamp(
                  vert_rate_opt.value() * 0.508f,
                  static_cast<float>(std::numeric_limits<int16_t>::min()),
                  static_cast<float>(std::numeric_limits<int16_t>::max())));
              // Note: openhd might use negative for up/down depending on conventions but we leave it as parsed,
              // dump1090 gives + for up, mavlink ADSB_VEHICLE ver_velocity positive is UP.
            }
          }
        }
      }
    }

    m_console->info("Disconnected from dump1090, retrying...");
    close(sock);
  }
}

std::vector<MavlinkMessage> AdsbComponent::generate_mavlink_messages() {
  std::vector<MavlinkMessage> res;

  std::lock_guard<std::mutex> lock(m_adsb_mutex);
  const auto stale_before =
      std::chrono::steady_clock::now() - std::chrono::seconds(15);
  for (auto it = m_last_seen.begin(); it != m_last_seen.end();) {
    if (it->second < stale_before) {
      m_adsb_vehicles.erase(it->first);
      it = m_last_seen.erase(it);
    } else {
      ++it;
    }
  }
  for (const auto& pair : m_adsb_vehicles) {
    MavlinkMessage msg;
    mavlink_msg_adsb_vehicle_encode(m_sys_id, m_comp_id, &msg.m, &pair.second);
    res.push_back(msg);
  }

  return res;
}

std::vector<MavlinkMessage> AdsbComponent::process_mavlink_messages(
    std::vector<MavlinkMessage> messages) {
  return {};
}
