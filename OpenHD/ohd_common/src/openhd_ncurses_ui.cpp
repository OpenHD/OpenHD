#include "openhd_ncurses_ui.h"
#include "openhd_spdlog.h"

#include <atomic>
#include <mutex>
#ifdef OPENHD_HAVE_CURSES
#include <curses.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/statvfs.h>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>
#endif

namespace openhd::ui {
namespace {
std::atomic<bool> active{false};
#ifdef OPENHD_HAVE_CURSES
enum class LogCategory { Devourer, Camera, Radio, Telemetry, System };
struct LogLine { std::string text; int level; LogCategory category; };
constexpr size_t log_limit = 1000;
constexpr unsigned all_debug_categories = (1u << 5) - 1;
#endif

struct UiState {
  std::mutex mutex;
  DashboardStatus status;
  DashboardAction action = DashboardAction::None;
#ifdef OPENHD_HAVE_CURSES
  std::deque<LogLine> logs;
  int scroll = 0;
  bool logs_only = false;
  int page = 0;
  unsigned debug_mask = all_debug_categories;
  int menu_cursor = 0;
  std::string menu_message;
  std::vector<std::string> network;
  std::string uptime = "--", cpu = "--", ram = "--", disk = "--", temp = "--";
  double cpu_ratio = -1, ram_ratio = -1, disk_ratio = -1, temp_ratio = -1;
  SCREEN* terminal = nullptr;
  FILE* terminal_output = nullptr;
  int saved_stdout = -1;
  int saved_stderr = -1;
  int capture_read = -1;
  std::atomic<bool> capture_running{false};
  std::thread capture_thread;
  FILE* session_log = nullptr;
#endif
};

UiState& ui_state() {
  // OpenHD logging begins in constructors from other translation units. Keep
  // all non-trivial dashboard state behind one first-use initialization point
  // so those constructors cannot observe partially initialized STL objects.
  static auto* value = new UiState();
  return *value;
}

#ifdef OPENHD_HAVE_CURSES

std::string clean(std::string text) {
  for (auto& c : text) if (static_cast<unsigned char>(c) < 32 || c == 127) c = ' ';
  return text;
}
std::string lower(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return text;
}
LogCategory categorize(const std::string& name, const std::string& message) {
  const auto text = lower(name + " " + message);
  if (text.find("devourer") != std::string::npos ||
      text.find("jaguar") != std::string::npos ||
      text.find("kestrel") != std::string::npos)
    return LogCategory::Devourer;
  if (text.find("camera") != std::string::npos ||
      text.find("cam0") != std::string::npos || text.find("cam1") != std::string::npos ||
      text.find("video") != std::string::npos || text.find("gstreamer") != std::string::npos ||
      text.find("gst_") != std::string::npos || text.find("mpp") != std::string::npos ||
      text.find("v_air") != std::string::npos || text.find("v_gnd") != std::string::npos)
    return LogCategory::Camera;
  if (text.find("tele") != std::string::npos || text.find("mavlink") != std::string::npos ||
      text.find("serial") != std::string::npos || text.find("adsb") != std::string::npos ||
      text.find("sbus") != std::string::npos || text.find("joystick") != std::string::npos)
    return LogCategory::Telemetry;
  if (text.find("wifi") != std::string::npos || text.find("wifibroadcast") != std::string::npos ||
      text.find("wb_") != std::string::npos || text.find("wbtx") != std::string::npos ||
      text.find("wbrx") != std::string::npos || text.find("radio") != std::string::npos ||
      text.find("artosyn") != std::string::npos || text.find("microhard") != std::string::npos ||
      text.find("interface") != std::string::npos)
    return LogCategory::Radio;
  return LogCategory::System;
}
std::string selection_name(unsigned mask) {
  if (mask == all_debug_categories) return "All debug sources";
  if (mask == 0) return "No debug sources";
  static constexpr const char* names[] = {
      "Devourer", "Camera/video", "Radio", "Telemetry", "System"};
  std::string result;
  for (int i = 0; i < 5; ++i) {
    if ((mask & (1u << i)) == 0) continue;
    if (!result.empty()) result += " + ";
    result += names[i];
  }
  return result;
}
bool matches_filter(const LogLine& line, unsigned mask) {
  return (mask & (1u << static_cast<unsigned>(line.category))) != 0;
}
bool config_partition_mounted() {
  std::ifstream mounts("/proc/self/mountinfo");
  std::string line;
  while (std::getline(mounts, line)) {
    std::istringstream fields(line);
    std::string id, parent, major_minor, root, mount_point;
    if (fields >> id >> parent >> major_minor >> root >> mount_point) {
      if (mount_point == "/Config") return true;
    }
  }
  return false;
}
std::string toggle_persistent_logs() {
  const bool testing = std::getenv("OPENHD_LOG_CONTROL_DIR") != nullptr;
  if (!testing && !config_partition_mounted())
    return "Change failed: SD card /Config is not mounted";
  const bool enable = !openhd::log::persistent_logging_enabled();
  if (!openhd::log::set_persistent_logging_enabled(enable, true))
    return "Change failed: cannot switch persistent logs";
  return enable ? "Persistent logs started" : "Persistent logs stopped";
}
std::string save_logs_to_sd(UiState& state) {
  const char* override_dir = std::getenv("OPENHD_LOG_EXPORT_DIR");
  if ((!override_dir || !*override_dir) && !config_partition_mounted())
    return "Save failed: SD card /Config is not mounted";
  const std::filesystem::path directory =
      override_dir && *override_dir ? override_dir : "/Config/openhd/logs";
  std::error_code error;
  std::filesystem::create_directories(directory, error);
  if (error) return "Save failed: " + error.message();
  const auto now = std::chrono::system_clock::now();
  const auto time = std::chrono::system_clock::to_time_t(now);
  std::tm local{}; localtime_r(&time, &local);
  std::ostringstream filename;
  filename << "openhd_debug_" << std::put_time(&local, "%Y%m%d_%H%M%S") << '_'
           << std::setfill('0') << std::setw(3)
           << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()%1000
           << ".log";
  const auto path = directory / filename.str();
  std::ofstream output(path, std::ios::trunc);
  if (!output) return "Save failed: cannot write " + path.string();
  output << "OpenHD terminal debug log\n"
         << "Debug sources selected: " << selection_name(state.debug_mask) << "\n\n";
  if (state.session_log) {
    std::fflush(state.session_log);
    const int session_fd = dup(fileno(state.session_log));
    if (session_fd >= 0) {
      lseek(session_fd, 0, SEEK_SET);
      char buffer[4096];
      ssize_t count = 0;
      while ((count = read(session_fd, buffer, sizeof(buffer))) > 0)
        output.write(buffer, count);
      close(session_fd);
    }
  } else {
    for (const auto& line : state.logs) output << line.text << '\n';
  }
  output.flush();
  if (!output) return "Save failed while writing " + path.string();
  output.close();
  const int output_fd = open(path.c_str(), O_RDONLY);
  if (output_fd >= 0) { fsync(output_fd); close(output_fd); }
  const int directory_fd = open(directory.c_str(), O_RDONLY | O_DIRECTORY);
  if (directory_fd >= 0) { fsync(directory_fd); close(directory_fd); }
  return "Saved all logs to " + path.string();
}
void capture_console(int fd) {
  std::string pending;
  char buffer[1024];
  auto& state = ui_state();
  while (state.capture_running.load()) {
    pollfd descriptor{fd, POLLIN, 0};
    const int ready = poll(&descriptor, 1, 100);
    if (ready <= 0) continue;
    const auto count = read(fd, buffer, sizeof(buffer));
    if (count <= 0) break;
    pending.append(buffer, static_cast<size_t>(count));
    for (auto newline = pending.find('\n'); newline != std::string::npos;
         newline = pending.find('\n')) {
      auto line = pending.substr(0, newline);
      pending.erase(0, newline + 1);
      if (!line.empty()) ncurses_log("process", 2, line);
    }
  }
  if (!pending.empty() && active.load()) ncurses_log("process", 2, pending);
}
void put(int y, int x, const std::string& text, int color = 0, int limit = -1) {
  if (y < 0 || y >= LINES || x < 0 || x >= COLS - 1) return;
  const auto value = clean(text);
  const int length = std::min(COLS - x - 1, limit < 0 ? COLS : limit);
  if (length <= 0) return;
  attron(COLOR_PAIR(color));
  mvaddnstr(y, x, value.c_str(), length);
  attroff(COLOR_PAIR(color));
}
void panel(int y, int x, int h, int w, const std::string& title) {
  if (h < 2 || w < 3) return;
  attron(COLOR_PAIR(4));
  mvhline(y, x, ACS_HLINE, w); mvhline(y+h-1, x, ACS_HLINE, w);
  mvvline(y, x, ACS_VLINE, h); mvvline(y, x+w-1, ACS_VLINE, h);
  mvaddch(y,x,ACS_ULCORNER); mvaddch(y,x+w-1,ACS_URCORNER);
  mvaddch(y+h-1,x,ACS_LLCORNER); mvaddch(y+h-1,x+w-1,ACS_LRCORNER);
  attroff(COLOR_PAIR(4));
  put(y,x+2," " + title + " ",4,w-4);
}
std::string amount(double bytes) {
  std::ostringstream out;
  if (bytes >= 1073741824.) out << std::fixed << std::setprecision(1) << bytes/1073741824. << 'G';
  else out << std::fixed << std::setprecision(0) << bytes/1048576. << 'M';
  return out.str();
}
void sample() {
  auto& state = ui_state();
  static auto last = std::chrono::steady_clock::time_point{};
  auto now = std::chrono::steady_clock::now();
  if (now-last < std::chrono::seconds(1)) return;
  last = now;
  double seconds;
  if (std::ifstream("/proc/uptime") >> seconds) {
    const auto s = static_cast<long long>(seconds);
    std::ostringstream out;
    out << s/86400 << "d " << std::setfill('0') << std::setw(2) << s/3600%24
        << ':' << std::setw(2) << s/60%60 << ':' << std::setw(2) << s%60;
    state.uptime = out.str();
  }
  std::ifstream stat("/proc/stat");
  std::string label;
  unsigned long long user, nice, system, idle, wait, irq, soft, steal;
  if (stat >> label >> user >> nice >> system >> idle >> wait >> irq >> soft >> steal) {
    static unsigned long long old_total = 0, old_idle = 0;
    auto total = user+nice+system+idle+wait+irq+soft+steal;
    auto idle_total = idle+wait;
    if (old_total && total > old_total && idle_total >= old_idle) {
      state.cpu_ratio = std::clamp(1.0-double(idle_total-old_idle)/double(total-old_total),0.0,1.0);
      state.cpu = std::to_string(static_cast<int>(state.cpu_ratio*100)) + '%';
    }
    old_total = total; old_idle = idle_total;
  }
  std::ifstream mem("/proc/meminfo");
  std::string line;
  double total_mem = 0, available = -1;
  while (std::getline(mem,line)) {
    std::istringstream fields(line); double value;
    if (!(fields >> label >> value)) continue;
    if (label == "MemTotal:") total_mem = value*1024;
    if (label == "MemAvailable:") available = value*1024;
  }
  if (total_mem > 0 && available >= 0) {
    state.ram_ratio = (total_mem-available)/total_mem;
    state.ram = amount(total_mem-available) + '/' + amount(total_mem);
  }
  struct statvfs fs{};
  if (statvfs("/", &fs) == 0 && fs.f_blocks) {
    const double total = double(fs.f_blocks)*fs.f_frsize;
    const double used = double(fs.f_blocks-fs.f_bfree)*fs.f_frsize;
    state.disk_ratio = used/total; state.disk = amount(used) + '/' + amount(total);
  }
  double temperature;
  if (std::ifstream("/sys/class/thermal/thermal_zone0/temp") >> temperature) {
    state.temp = std::to_string(static_cast<int>(temperature/1000)) + " C";
    state.temp_ratio = temperature/100000.;
  }
  state.network.clear();
  ifaddrs* addresses = nullptr;
  if (getifaddrs(&addresses) == 0) {
    for (auto* p = addresses; p; p = p->ifa_next) {
      if (!p->ifa_addr || p->ifa_addr->sa_family != AF_INET || (p->ifa_flags & IFF_LOOPBACK)) continue;
      char address[INET_ADDRSTRLEN]{};
      auto* in = reinterpret_cast<sockaddr_in*>(p->ifa_addr);
      if (inet_ntop(AF_INET,&in->sin_addr,address,sizeof(address)))
        state.network.push_back(std::string(p->ifa_name) + "  " + address);
    }
    freeifaddrs(addresses);
  }
}
void meter(int y, int x, int w, const std::string& name, const std::string& value, double ratio) {
  put(y,x,name + " " + value,0,w);
  int bar = std::min(12,w-19);
  if (bar < 3 || ratio < 0) return;
  const int start = x+w-bar;
  put(y,start,std::string(bar,'.'),0,bar);
  const int color = ratio > .9 ? 2 : ratio > .75 ? 3 : 1;
  attron(COLOR_PAIR(color));
  mvhline(y,start,ACS_CKBOARD,static_cast<int>(std::clamp(ratio,0.0,1.0)*bar));
  attroff(COLOR_PAIR(color));
}
#endif
}
bool ncurses_active() { return active.load(); }
DashboardAction take_dashboard_action() {
  auto& state = ui_state();
  std::lock_guard<std::mutex> guard(state.mutex);
  auto result = state.action; state.action = DashboardAction::None; return result;
}
void set_dashboard_status(const DashboardStatus& value) {
  auto& state = ui_state();
  std::lock_guard<std::mutex> guard(state.mutex); state.status = value;
}
void init_ncurses() {
#ifdef OPENHD_HAVE_CURSES
  auto& state = ui_state();
  std::lock_guard<std::mutex> guard(state.mutex);
  const char* term = std::getenv("TERM");
  if (active || !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) || !term || std::string(term)=="dumb") return;
  const int output_fd = dup(STDOUT_FILENO);
  if (output_fd < 0) return;
  state.terminal_output = fdopen(output_fd, "w");
  if (!state.terminal_output) { close(output_fd); return; }
  state.terminal = newterm(term, state.terminal_output, stdin);
  if (!state.terminal) {
    fclose(state.terminal_output); state.terminal_output = nullptr; return;
  }
  cbreak(); noecho(); keypad(stdscr,TRUE); nodelay(stdscr,TRUE); curs_set(0);
  if (has_colors()) {
    start_color();
    const int background = use_default_colors() == OK ? -1 : COLOR_BLACK;
    init_pair(1,COLOR_GREEN,background); init_pair(2,COLOR_RED,background);
    init_pair(3,COLOR_YELLOW,background); init_pair(4,COLOR_CYAN,background);
  }
  char session_template[] = "/tmp/openhd-ui-session-XXXXXX";
  const int session_fd = mkstemp(session_template);
  if (session_fd >= 0) {
    unlink(session_template);
    state.session_log = fdopen(session_fd, "w+");
    if (!state.session_log) close(session_fd);
  }
  // Curses owns a duplicate of the console output. Keep library and child
  // process stdout/stderr from overwriting the dashboard between refreshes.
  fflush(stdout); fflush(stderr);
  state.saved_stdout = dup(STDOUT_FILENO);
  state.saved_stderr = dup(STDERR_FILENO);
  int capture_pipe[2]{-1, -1};
  if (state.saved_stdout >= 0 && state.saved_stderr >= 0 && pipe(capture_pipe) == 0) {
    dup2(capture_pipe[1], STDOUT_FILENO);
    dup2(capture_pipe[1], STDERR_FILENO);
    close(capture_pipe[1]);
    state.capture_read = capture_pipe[0];
    state.capture_running = true;
    state.capture_thread = std::thread(capture_console, state.capture_read);
    setenv("OPENHD_TUI_DEBUG", "1", 1);
  }
  active = true;
  std::atexit(shutdown_ncurses);
#endif
}
void shutdown_ncurses() {
#ifdef OPENHD_HAVE_CURSES
  auto& state = ui_state();
  std::unique_lock<std::mutex> guard(state.mutex);
  if (active.exchange(false)) {
    endwin();
    if (state.terminal) { delscreen(state.terminal); state.terminal = nullptr; }
    if (state.terminal_output) { fclose(state.terminal_output); state.terminal_output = nullptr; }
    if (state.saved_stdout >= 0) {
      dup2(state.saved_stdout, STDOUT_FILENO); close(state.saved_stdout); state.saved_stdout = -1;
    }
    if (state.saved_stderr >= 0) {
      dup2(state.saved_stderr, STDERR_FILENO); close(state.saved_stderr); state.saved_stderr = -1;
    }
    state.capture_running = false;
    auto* capture_thread = state.capture_thread.joinable() ? &state.capture_thread : nullptr;
    guard.unlock();
    if (capture_thread) capture_thread->join();
    guard.lock();
    if (state.capture_read >= 0) { close(state.capture_read); state.capture_read = -1; }
    if (state.session_log) { fclose(state.session_log); state.session_log = nullptr; }
    unsetenv("OPENHD_TUI_DEBUG");
  }
#endif
}
void ncurses_log(const std::string& name, int level, const std::string& message) {
#ifdef OPENHD_HAVE_CURSES
  // Logging can occur from constructors in other translation units. Avoid
  // touching the dynamically initialized log deque until main enables the UI.
  if (!active.load()) return;
  auto& state = ui_state();
  std::lock_guard<std::mutex> guard(state.mutex);
  const auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::tm local{}; localtime_r(&time,&local);
  const char* levels[] = {"TRACE","DEBUG","INFO","WARN","ERROR","CRITICAL","OFF"};
  std::ostringstream prefix;
  prefix << '[' << std::put_time(&local,"%H:%M:%S") << '.' << std::setfill('0') << std::setw(3)
         << std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()%1000
         << "] [" << name << "] [" << levels[std::clamp(level,0,6)] << "] ";
  std::istringstream stream(message); std::string line;
  while (std::getline(stream,line)) {
    const auto text = prefix.str()+clean(line);
    state.logs.push_back({text, level, categorize(name, line)});
    if (state.session_log) {
      std::fwrite(text.data(), 1, text.size(), state.session_log);
      std::fputc('\n', state.session_log);
    }
    if (state.scroll) ++state.scroll;
    if (state.logs.size()>log_limit) state.logs.pop_front();
  }
  state.scroll = std::min(state.scroll,static_cast<int>(state.logs.size()));
#else
  (void)name; (void)level; (void)message;
#endif
}
void update_ncurses() {
#ifdef OPENHD_HAVE_CURSES
  auto& state = ui_state();
  std::lock_guard<std::mutex> guard(state.mutex);
  if (!active) return;
  for (int key = getch(); key != ERR; key = getch()) {
    if (state.page == 3 || state.page == 5) {
      if (key == 'y' || key == 'Y') {
        state.action = state.page == 3 ? DashboardAction::Restart : DashboardAction::Shutdown;
        state.page = 0;
      } else if (key == 27 || key == 'n' || key == 'N') state.page = 0;
      continue;
    }
    if (state.page == 2) {
      if (key == 27 || key == 'd' || key == 'D') { state.page = 0; continue; }
      if (key == KEY_UP) state.menu_cursor = std::max(0, state.menu_cursor - 1);
      if (key == KEY_DOWN) state.menu_cursor = std::min(8, state.menu_cursor + 1);
      if (key >= '1' && key <= '5') {
        const int category = key - '1';
        state.debug_mask ^= 1u << category;
        state.menu_cursor = category;
        state.scroll = 0;
        state.menu_message.clear();
        continue;
      }
      if (key == 'a' || key == 'A') {
        state.debug_mask = all_debug_categories; state.menu_cursor = 5;
        state.scroll = 0; state.menu_message.clear(); continue;
      }
      if (key == 'x' || key == 'X') {
        state.debug_mask = 0; state.menu_cursor = 6;
        state.scroll = 0; state.menu_message.clear(); continue;
      }
      if (key == 's' || key == 'S') {
        state.menu_cursor = 8; state.menu_message = save_logs_to_sd(state); continue;
      }
      if (key == 'l' || key == 'L') {
        state.menu_cursor = 7; state.menu_message = toggle_persistent_logs(); continue;
      }
      if (key == ' ' || key == '\n' || key == KEY_ENTER) {
        if (state.menu_cursor < 5) state.debug_mask ^= 1u << state.menu_cursor;
        else if (state.menu_cursor == 5) state.debug_mask = all_debug_categories;
        else if (state.menu_cursor == 6) state.debug_mask = 0;
        else if (state.menu_cursor == 7) state.menu_message = toggle_persistent_logs();
        else state.menu_message = save_logs_to_sd(state);
        state.scroll = 0;
      }
      continue;
    }
    if (key == 'h' || key == 'H') state.page = state.page == 1 ? 0 : 1;
    if (key == 'd' || key == 'D') state.page = state.page == 2 ? 0 : 2;
    if (key == 'r' || key == 'R') state.page = 3;
    if (key == 'p' || key == 'P') state.page = 5;
    if (key == 'n' || key == 'N') state.page = state.page == 4 ? 0 : 4;
    if (key == 27) state.page = 0;
    if (key == '\t') { state.logs_only = !state.logs_only; state.page = 0; }
    if (key == KEY_PPAGE) state.scroll += std::max(1,LINES-4);
    if (key == KEY_NPAGE) state.scroll = std::max(0,state.scroll-std::max(1,LINES-4));
    if (key == KEY_UP) ++state.scroll;
    if (key == KEY_DOWN) state.scroll = std::max(0,state.scroll-1);
    if (key == KEY_END) state.scroll = 0;
  }
  sample(); erase();
  if (COLS < 60 || LINES < 18) {
    put(0,0,state.status.version,4); put(2,0,"Dashboard needs 60 columns x 18 rows.");
    put(3,0,"Resize terminal. Ctrl+C exits."); refresh(); return;
  }
  int log_top = 1;
  if (!state.logs_only) {
    const bool wide = COLS >= 100 && LINES >= 22;
    const int header = wide ? 9 : 6;
    const int metrics_x = COLS-33;
    const int info_x = wide ? 40 : 2;
    if (wide) {
      const char* logo[] = {
        " ###  ####  ##### #   # #   # #### ",
        "#   # #   # #     ##  # #   # #   #",
        "#   # ####  ####  # # # ##### #   #",
        "#   # #     #     #  ## #   # #   #",
        " ###  #     ##### #   # #   # #### "};
      for (int i=0;i<5;++i) put(i+1,3,logo[i],4);
      put(7,3,"----------- OpenSource -----------",4);
    }
    put(1,info_x,state.status.version,4,metrics_x-info_x-1);
    put(2,info_x,"Platform: " + state.status.platform,0,metrics_x-info_x-1);
    put(3,info_x,"Camera: " + state.status.camera,0,metrics_x-info_x-1);
    put(4,info_x,"Uptime: " + state.uptime,0,metrics_x-info_x-1);
    auto time = std::time(nullptr); std::tm local{}; localtime_r(&time,&local);
    std::ostringstream date; date << std::put_time(&local,"%b %d %Y %H:%M:%S");
    panel(0,metrics_x,6,32,date.str());
    meter(1,metrics_x+2,28,"CPU ",state.cpu,state.cpu_ratio);
    meter(2,metrics_x+2,28,"RAM ",state.ram,state.ram_ratio);
    meter(3,metrics_x+2,28,"Disk",state.disk,state.disk_ratio);
    meter(4,metrics_x+2,28,"Temp",state.temp,state.temp_ratio);
    const int third = COLS/3;
    panel(header,0,7,third,"Link Status");
    panel(header,third,7,third,"Devices");
    panel(header,third*2,7,COLS-third*2,"Network");
    const char* names[] = {"WiFiBroadcast", "Artosyn", "Microhard", "Ethernet", "LTE"};
    const char* short_names[] = {"WiFi", "Art", "MH", "Eth", "LTE"};
    for (int i=0;i<5;++i) {
      put(header+i+1,2,third < 29 ? short_names[i] : names[i],0,third-4);
      const auto& value = state.status.links[i];
      const int color = value == "Configured" ? 1 : value == "Unknown" ? 3 : 0;
      attron(COLOR_PAIR(color));
      mvaddch(header+i+1,third-14,ACS_BULLET);
      attroff(COLOR_PAIR(color));
      put(header+i+1,third-12,value,color,10);
    }
    put(header+1,third+2,"Camera: " + state.status.camera,0,third-4);
    put(header+2,third+2,"FC: Unknown",0,third-4);
    put(header+3,third+2,"WiFi: " + state.status.wifi,0,third-4);
    put(header+4,third+2,"SDR: Unknown",0,third-4);
    if (state.network.empty()) put(header+1,third*2+2,"No IPv4 address",3,COLS-third*2-4);
    for (size_t i=0;i<state.network.size() && i<5;++i)
      put(header+1+static_cast<int>(i),third*2+2,state.network[i],0,COLS-third*2-4);
    log_top = header+7;
  }
  const int visible = std::max(0,LINES-log_top-3);
  std::vector<const LogLine*> filtered;
  filtered.reserve(state.logs.size());
  for (const auto& line : state.logs)
    if (matches_filter(line, state.debug_mask)) filtered.push_back(&line);
  state.scroll = std::clamp(state.scroll,0,std::max(0,static_cast<int>(filtered.size())-visible));
  const std::string log_title = std::string("Logs - ") +
      (state.scroll ? "paused" : "live") + " [" + selection_name(state.debug_mask) + "]" +
      (state.scroll ? " (End: live)" : "");
  panel(log_top,0,LINES-log_top-1,COLS,log_title);
  const int start = std::max(0,static_cast<int>(filtered.size())-visible-state.scroll);
  for (int i=0;i<visible && start+i<static_cast<int>(filtered.size());++i) {
    const auto& line = *filtered[start+i];
    put(log_top+1+i,2,line.text,line.level>=4 ? 2 : line.level==3 ? 3 : 0,COLS-4);
  }
  put(LINES-1,1,COLS >= 100 ? "[H] Help  [D] Debug  [R] Restart  [N] Network  [P] Power  [TAB] Switch View  [CTRL+C] Exit" : "H Help D Debug R Restart N Net P Power Tab View ^C",4);
  if (state.page) {
    const int width = std::min(COLS-4,76), left = (COLS-width)/2;
    const int top = state.page == 2 ? 2 : 3;
    const int height = state.page == 2 ? 15 : 11;
    for (int y=top;y<top+height;++y) put(y,left,std::string(width,' '));
    panel(top,left,height,width,state.page==3 ? "Restart OpenHD" : state.page==5 ? "Power off board" : state.page==4 ? "Network addresses" : state.page==2 ? "Debug menu" : "Help");
    if (state.page==3 || state.page==5) {
      put(5,left+2,state.page==3 ? "Restart OpenHD now? Video and telemetry will stop." : "Power off this board? Video and telemetry will stop.",3,width-4);
      put(7,left+2,"Y: confirm    N / Esc: cancel",0,width-4);
    } else if (state.page==4) {
      for (size_t i=0;i<state.network.size() && i<7;++i) put(5+static_cast<int>(i),left+2,state.network[i],0,width-4);
      if (state.network.empty()) put(5,left+2,"No IPv4 address assigned.");
    } else if (state.page == 2) {
      static constexpr const char* entries[] = {
          "Devourer / USB radio", "Camera / video pipelines", "WiFiBroadcast / radio link",
          "Telemetry / MAVLink / serial", "System / plugins / other",
          "Enable all debug sources", "Clear all selections", "Runtime OpenHD logs",
          "Save all logs to SD card"};
      for (int i = 0; i < 9; ++i) {
        const bool cursor = i == state.menu_cursor;
        std::string shortcut;
        if (i < 5) shortcut = std::string("[") + char('1'+i) + "]";
        else shortcut = i == 5 ? "[A]" : i == 6 ? "[X]" : i == 7 ? "[L]" : "[S]";
        const std::string checkbox = i < 5
            ? std::string((state.debug_mask & (1u << i)) ? "[x] " : "[ ] ")
            : i == 7 ? std::string(openhd::log::persistent_logging_enabled() ? "[x] " : "[ ] ") : "    ";
        put(4+i,left+2,std::string(cursor ? "> " : "  ") + checkbox + shortcut + " " + entries[i],
            cursor ? 4 : 0,width-4);
      }
      if (!state.menu_message.empty()) put(13,left+2,state.menu_message,3,width-4);
    } else {
      put(5,left+2,"Tab: dashboard / full-screen logs",0,width-4);
      put(6,left+2,"Up/Down, PgUp/PgDn: scroll logs; End: follow live",0,width-4);
      put(7,left+2,"D: choose debug sources; N: network addresses",0,width-4);
      put(8,left+2,"Configured means transport exists, not peer connected.",0,width-4);
      put(9,left+2,"Unknown means no live measurement is available.",0,width-4);
    }
    put(state.page == 2 ? 15 : 12,left+2,"Esc: close",4,width-4);
  }
  refresh();
#endif
}
}  // namespace openhd::ui
