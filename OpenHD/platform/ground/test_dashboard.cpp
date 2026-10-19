#include "ground_config.h"
#include "openhd_ncurses_ui.h"
#include <curses.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
std::string screen() {
  std::string text;
  for (int y = 0; y < LINES; ++y) {
    for (int x = 0; x < COLS; ++x) text += char(mvinch(y, x) & A_CHARTEXT);
    text += '\n';
  }
  return text;
}
void keys(std::initializer_list<int> values) {
  for (auto it = std::rbegin(values); it != std::rend(values); ++it) ungetch(*it);
}
void key(int value) { ungetch(value); openhd::ui::update_ncurses(); }
}
int main(int argc, char** argv) {
  const auto output = std::filesystem::absolute(argc > 1 ? argv[1] : "dashboard-test.txt");
  const auto directory = output.parent_path() /
      ("dashboard-test-profile-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  int result = 1;
  std::string failure;
#ifdef _WIN32
  // Give this test its own hidden console. It exercises the real Win32 curses
  // backend without depending on a user's terminal or changing their console.
  FreeConsole(); AllocConsole(); ShowWindow(GetConsoleWindow(), SW_HIDE);
  freopen("CONIN$", "r", stdin); freopen("CONOUT$", "w", stdout); freopen("CONOUT$", "w", stderr);
#endif
  try {
    using namespace openhd::ui;
    using namespace openhd::ground;
    configure_log_directory(directory / "logs");
    init_ncurses(); require(ncurses_active(), "dashboard did not initialize");
    resize_term(30, 116);
    DashboardStatus status;
    status.version = "OpenHD - Ground"; status.platform = "Windows / Devourer USB";
    status.portable_controls = true; status.camera = "Ground receiver"; status.links[0] = "Stopped";
    status.radio_summary = "5180 MHz / 20 MHz";
    status.video_summary = "Video: 0 packets"; status.telemetry_summary = "Telemetry: 0 packets";
    set_dashboard_status(status);
    ncurses_log("Devourer", 2, "No radio connected. Press C for settings or R to retry.");
    update_ncurses();
    require(screen().find("Link Status") != std::string::npos, "missing Linux link panel");
    require(screen().find("[C] Settings") != std::string::npos, "missing settings control");
    { std::ofstream dump(output); dump << screen(); }
    key('c'); require(take_dashboard_action() == DashboardAction::Configure, "settings shortcut failed");
    Config config;
    keys({27}); require(!edit_settings(config, directory / "ground.json"), "settings cancellation failed");
    require(!std::filesystem::exists(directory / "ground.json"), "cancel wrote settings");
    keys({KEY_DOWN, '\n', '5', '7', '4', '5', '\n', 's'});
    require(edit_settings(config, directory / "ground.json"), "settings save failed");
    require(config.frequency == 5745 && load_config(directory / "ground.json").frequency == 5745,
            "saved frequency was not restored");
    { std::ofstream dump(output.string() + ".settings.txt"); dump << screen(); }
    keys({KEY_DOWN, '\n', '5', '1', '8', '1', '\n', 's', 27});
    require(!edit_settings(config, directory / "ground.json"), "invalid settings accepted");
    require(config.frequency == 5745 && screen().find("Unsupported frequency") != std::string::npos,
            "invalid settings were not rejected transactionally");
    key('d'); require(screen().find("Debug menu") != std::string::npos, "debug menu failed");
    key('s'); require(screen().find("Saved all logs") != std::string::npos, "log export failed");
    key(27); key('p');
    require(screen().find("Windows will keep running") != std::string::npos, "stop confirmation is misleading");
    require(take_dashboard_action() == DashboardAction::None, "stop lacked confirmation");
    key('n'); key('r'); key('y');
    require(take_dashboard_action() == DashboardAction::Restart, "restart confirmation failed");
    key('q'); require(take_dashboard_action() == DashboardAction::Exit, "exit shortcut failed");
    shutdown_ncurses();
    result = 0;
  } catch (const std::exception& error) { failure = error.what(); openhd::ui::shutdown_ncurses(); }
  std::ofstream report(output.string() + ".result");
  report << (result ? "FAIL: " + failure : "PASS: shared dashboard, settings, persistence, validation, export and controls") << '\n';
  return result;
}
