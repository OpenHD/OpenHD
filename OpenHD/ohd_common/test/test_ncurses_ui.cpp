#include "openhd_ncurses_ui.h"
#include <curses.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

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
    key('\t');
    require(screen_text().find("Link Status") == std::string::npos, "log-only view failed");
    key('\t');
    key(KEY_F(1));
    require(screen_text().find("follow live") != std::string::npos, "help failed");
    key(27);
    key(KEY_F(3));
    require(take_dashboard_action() == DashboardAction::None, "restart without confirmation");
    key('n');
    require(take_dashboard_action() == DashboardAction::None, "restart cancellation failed");
    key(KEY_F(3)); key('y');
    require(take_dashboard_action() == DashboardAction::Restart, "restart confirmation failed");
    key(KEY_F(5)); key('y');
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
