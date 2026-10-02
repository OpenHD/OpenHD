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

#include "openhd_spdlog.h"
#include "openhd_spdlog_include.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

#include "config_paths.h"

namespace {
bool contains(const std::filesystem::path& path, const char* message) {
  std::ifstream file(path);
  const std::string contents{std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>()};
  return contents.find(message) != std::string::npos;
}
}  // namespace

int main(int argc, char *argv[]) {
  // Optional expectation allows testing build/image defaults in fresh processes
  // without forcing logging on through OPENHD_PERSISTENT_LOG_DIR.
  if (const char* recordings = std::getenv("OPENHD_TEST_RECORDINGS_DIR")) {
    setVideoPath(recordings);
  }
  openhd::log::initialize_persistent_logging();
  if (argc > 1 && openhd::log::persistent_logging_enabled() !=
                      (std::string(argv[1]) == "enabled")) {
    std::cerr << "Unexpected persistent logging default\n";
    return 1;
  }
  const bool enabled = openhd::log::persistent_logging_enabled();
  const auto directory = std::filesystem::path(openhd::log::persistent_log_directory());
  if (const char* recordings = std::getenv("OPENHD_TEST_RECORDINGS_DIR")) {
    if (directory != std::filesystem::path(recordings) / "logs" / "openhd") {
      std::cerr << "Logs do not use the recordings directory\n";
      return 1;
    }
  }
  openhd::log::get_default()->debug("OpenHD category debug");
  openhd::log::get_default()->warn("OpenHD category warning");
  openhd::log::create_or_get("camera_test")->info("Camera category message");
  openhd::log::create_or_get("telemetry_test")->info("Other category message");
  if (openhd::log::persistent_logging_enabled()) {
    bool listener_saw_stop = false;
    bool listener_saw_start = false;
    const auto listener = openhd::log::add_persistent_logging_listener(
        [&](bool enabled) {
          listener_saw_start = listener_saw_start || enabled;
          listener_saw_stop = listener_saw_stop || !enabled;
        });
    if (!openhd::log::set_persistent_logging_enabled(false, false)) return 1;
    openhd::log::create_or_get("camera_test")->warn("MUST_NOT_BE_SAVED");
    if (!openhd::log::set_persistent_logging_enabled(true, false)) return 1;
    openhd::log::create_or_get("camera_test")->info("Runtime restart message");
    openhd::log::remove_persistent_logging_listener(listener);
    if (!listener_saw_stop || !listener_saw_start) return 1;
  }
  spdlog::shutdown();
  if (enabled &&
      (!contains(directory / "openhd.log", "OpenHD category debug") ||
       !contains(directory / "camera.log", "Camera category message") ||
       !contains(directory / "other.log", "Other category message") ||
       !contains(directory / "camera.log", "Runtime restart message") ||
       contains(directory / "camera.log", "MUST_NOT_BE_SAVED"))) {
    std::cerr << "Persistent log contents do not match runtime controls\n";
    return 1;
  }
  return 0;
}
