#include "openhd_ncurses_ui.h"
#include <curses.h>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {
std::string screen_text() {
  std::string result;
  for (int y = 0; y < LINES; ++y) {
    for (int x = 0; x < COLS; ++x)
      result += static_cast<char>(mvinch(y, x) & A_CHARTEXT);
    result += '\n';
  }
  return result;
}
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
void key(int value) { ungetch(value); openhd::ui::update_ncurses(); }
}

int main(int argc, char** argv) {
  using namespace openhd::ui;
  // This must be harmless before the UI's dynamic state is in use.
  ncurses_log("pre-main", 3, "ignored before initialization");
  init_ncurses();
  if (!ncurses_active()) return 77;  // Run in a pseudo-terminal.
  try {
    DashboardStatus status;
    status.version = "OpenHD UI test";
    status.platform = "Test platform";
    set_dashboard_status(status);
    resizeterm(28, 110);
    for (int i = 0; i < 1100; ++i)
      ncurses_log("test", 3, "message " + std::to_string(i));
    update_ncurses();
    require(screen_text().find("Link Status") != std::string::npos, "missing link panel");
    require(screen_text().find("message 1099") != std::string::npos, "not following newest log");
    if (argc > 1) { std::ofstream snapshot(argv[1]); snapshot << screen_text(); }
    key(KEY_PPAGE);
    require(screen_text().find("paused") != std::string::npos, "log scroll did not pause");
    key(KEY_END);
    require(screen_text().find("message 1099") != std::string::npos, "End did not resume logs");
    ncurses_log("cam0", 1, "camera-debug-marker");
    std::cerr << "devourer [I] devourer-debug-marker" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    update_ncurses();
    key('d');
    require(screen_text().find("Debug menu") != std::string::npos, "debug menu failed");
    require(screen_text().find("[x] [1] Devourer") != std::string::npos,
            "debug menu does not show source checkboxes");
    key('x');
    key('2');
    key('d');
    require(screen_text().find("camera-debug-marker") != std::string::npos,
            "camera filter omitted camera log");
    require(screen_text().find("devourer-debug-marker") == std::string::npos,
            "camera filter included Devourer log");
    key('d'); key('1'); key('d');
    require(screen_text().find("camera-debug-marker") != std::string::npos &&
            screen_text().find("devourer-debug-marker") != std::string::npos,
            "multi-select debug sources failed");
    char export_template[] = "/tmp/openhd-ui-export-XXXXXX";
    const char* export_dir = mkdtemp(export_template);
    require(export_dir != nullptr, "cannot create export test directory");
    setenv("OPENHD_LOG_EXPORT_DIR", export_dir, 1);
    key('d'); key('s');
    require(screen_text().find("Saved all logs") != std::string::npos, "log export failed");
    bool found_export = false;
    for (const auto& entry : std::filesystem::directory_iterator(export_dir)) {
      std::ifstream exported(entry.path());
      const std::string contents((std::istreambuf_iterator<char>(exported)), {});
      found_export = contents.find("camera-debug-marker") != std::string::npos &&
                     contents.find("devourer-debug-marker") != std::string::npos;
    }
    require(found_export, "export did not contain all log categories");
    std::filesystem::remove_all(export_dir);
    unsetenv("OPENHD_LOG_EXPORT_DIR");
    key('a'); key('d');
    key('\t');
    require(screen_text().find("Link Status") == std::string::npos, "log-only view failed");
    key('\t');
    key('h');
    require(screen_text().find("follow live") != std::string::npos, "help failed");
    key(27);
    key('r');
    require(take_dashboard_action() == DashboardAction::None, "restart without confirmation");
    key('n');
    require(take_dashboard_action() == DashboardAction::None, "restart cancellation failed");
    key('r'); key('y');
    require(take_dashboard_action() == DashboardAction::Restart, "restart confirmation failed");
    key('p'); key('y');
    require(take_dashboard_action() == DashboardAction::Shutdown, "shutdown confirmation failed");
    require(take_dashboard_action() == DashboardAction::None, "action not consumed");
    resizeterm(8, 25); update_ncurses();
    require(screen_text().find("Resize terminal") != std::string::npos, "small terminal fallback failed");
    resizeterm(18, 60); update_ncurses();
    require(screen_text().find("Link Status") != std::string::npos, "resize recovery failed");
    shutdown_ncurses(); shutdown_ncurses();
    require(!ncurses_active(), "shutdown failed");
    std::cout << "Dashboard checks passed\n";
  } catch (const std::exception& error) {
    shutdown_ncurses(); std::cerr << error.what() << '\n'; return 1;
  }
}
