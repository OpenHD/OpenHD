#include "ground_config.h"
#include "openhd_ncurses_ui.h"
#include <curses.h>
#include <algorithm>
#include <array>
#include <thread>

namespace openhd::ground {
namespace {
void text(int y, int x, const std::string& value, int color = 0) {
  if (y >= LINES || x >= COLS - 1) return;
  attron(COLOR_PAIR(color)); mvaddnstr(y, x, value.c_str(), COLS - x - 1); attroff(COLOR_PAIR(color));
}
std::string input(const std::string& title, const std::string& initial) {
  std::string value = initial;
  bool replace = true;
  while (true) {
    erase(); text(2, 3, title, 4);
    text(4, 3, value.empty() ? "(empty)" : value);
    text(6, 3, "Type to replace. Backspace edits. Ctrl+U clears. Enter accepts. Esc cancels.", 4);
    refresh(); const int key = getch();
#ifdef PDCURSES
    if (key == KEY_RESIZE) resize_term(0, 0);
#endif
    if (key == 27) return initial;
    if (key == '\n' || key == '\r' || key == KEY_ENTER) return value;
    if (key == 21) { value.clear(); replace = false; }
    if (key == KEY_BACKSPACE || key == 8 || key == 127) { if (!value.empty()) value.pop_back(); replace = false; }
    else if (key >= 32 && key <= 126 && value.size() < 1024) {
      if (replace) { value.clear(); replace = false; }
      value += char(key);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}
}
bool edit_settings(Config& config, const std::filesystem::path& path) {
  Config draft = config;
  int cursor = 0;
  std::string message;
  std::vector<UsbAdapter> adapters;
  auto scan = [&] {
    try { adapters = enumerate_usb(); message = "USB list refreshed. Candidate IDs still require a compatible driver."; }
    catch (const std::exception& error) { message = error.what(); }
  };
  scan();
  while (true) {
    erase();
    text(1, 3, "OpenHD - Ground settings", 4);
    text(2, 3, "Same radio link as Linux. Settings are saved for the next launch.");
    const std::array<std::string, 11> labels{"USB radio", "Frequency (MHz)", "Channel width (MHz)",
      "Keypair file", "UDP destination", "Primary video port", "Secondary video port", "Telemetry output port",
      "Telemetry input port", "Start automatically", "Save and start"};
    const std::array<std::string, 11> values{draft.devices.empty() ? "Automatic discovery" : draft.devices.front(),
      std::to_string(draft.frequency), std::to_string(draft.width), draft.keyfile.empty() ? "Built-in OpenHD default" : draft.keyfile,
      draft.destination, std::to_string(draft.video_port), std::to_string(draft.video2_port), std::to_string(draft.telemetry_port),
      std::to_string(draft.uplink_port), draft.autostart ? "Yes" : "No", "[ Enter ]"};
    if (COLS < 70 || LINES < 23) {
      text(4, 3, "Resize this window to at least 70 columns x 23 rows.", 3);
    } else {
      for (int i = 0; i < int(labels.size()); ++i) {
        text(4 + i, 3, (cursor == i ? "> " : "  ") + labels[i], cursor == i ? 4 : 0);
        text(4 + i, 32, values[i], cursor == i ? 4 : 0);
      }
      text(17, 3, "Up/Down: choose  Enter: change  S: save/start  R: rescan USB  Esc: cancel", 4);
      text(19, 3, message, 3);
      text(21, 3, "Profile: " + path.string());
    }
    refresh(); const int key = getch();
    if (key == 27) return false;
    if (key == KEY_UP) cursor = std::max(0, cursor - 1);
    if (key == KEY_DOWN) cursor = std::min(10, cursor + 1);
    if (key == 'r' || key == 'R') scan();
    if (key == 's' || key == 'S' || ((key == '\n' || key == '\r' || key == KEY_ENTER) && cursor == 10)) {
      try { save_config(draft, path); config = draft; return true; }
      catch (const std::exception& error) { message = error.what(); }
    } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
      try {
        if (cursor == 0) {
          std::vector<std::string> choices{""};
          for (const auto& adapter : adapters) if (adapter.candidate) choices.push_back(adapter.selector);
          const auto current = draft.devices.empty() ? "" : draft.devices.front();
          auto found = std::find(choices.begin(), choices.end(), current);
          const auto next = found == choices.end() || ++found == choices.end() ? choices.front() : *found;
          draft.devices = next.empty() ? std::vector<std::string>{} : std::vector<std::string>{next};
          if (choices.size() == 1) message = "No Devourer candidate found. Connect the adapter, install WinUSB if needed, then R.";
        } else if (cursor == 2) {
          const std::array<int, 4> widths{20, 40, 80, 10};
          auto found = std::find(widths.begin(), widths.end(), draft.width);
          draft.width = found == widths.end() || ++found == widths.end() ? widths.front() : *found;
        } else if (cursor == 9) draft.autostart = !draft.autostart;
        else if (cursor == 3) draft.keyfile = input("Custom 128-byte keypair path (leave empty for Linux default)", draft.keyfile);
        else if (cursor == 4) draft.destination = input("UDP forwarding IPv4 address", draft.destination);
        else {
          int* field = cursor == 1 ? &draft.frequency : cursor == 5 ? &draft.video_port : cursor == 6 ? &draft.video2_port :
                       cursor == 7 ? &draft.telemetry_port : &draft.uplink_port;
          const auto value = input(labels[cursor], std::to_string(*field));
          std::size_t used = 0; const int number = std::stoi(value, &used);
          if (used != value.size()) throw std::invalid_argument("Enter a whole number");
          *field = number;
        }
      } catch (const std::exception& error) { message = error.what(); }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}
}
