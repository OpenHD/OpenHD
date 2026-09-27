#include "openhd_ncurses_ui.h"

#include <atomic>
#include <mutex>
#ifdef OPENHD_HAVE_CURSES
#include <curses.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/statvfs.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#endif

namespace openhd::ui {
namespace {
std::atomic<bool> active{false};
std::mutex mutex;
DashboardStatus status;
DashboardAction action = DashboardAction::None;
#ifdef OPENHD_HAVE_CURSES
struct LogLine { std::string text; int level; };
std::deque<LogLine>& log_buffer() {
  // Keep the buffer alive for the process lifetime and initialize it on first
  // use. Logging starts in global constructors on some targets, so a namespace
  // scope deque can otherwise be accessed before its constructor has run.
  static auto* value = new std::deque<LogLine>();
  return *value;
}
int scroll = 0;
bool logs_only = false;
int page = 0;
constexpr size_t log_limit = 1000;
std::vector<std::string> network;
std::string uptime = "--", cpu = "--", ram = "--", disk = "--", temp = "--";
double cpu_ratio = -1, ram_ratio = -1, disk_ratio = -1, temp_ratio = -1;

std::string clean(std::string text) {
  for (auto& c : text) if (static_cast<unsigned char>(c) < 32 || c == 127) c = ' ';
  return text;
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
    uptime = out.str();
  }
  std::ifstream stat("/proc/stat");
  std::string label;
  unsigned long long user, nice, system, idle, wait, irq, soft, steal;
  if (stat >> label >> user >> nice >> system >> idle >> wait >> irq >> soft >> steal) {
    static unsigned long long old_total = 0, old_idle = 0;
    auto total = user+nice+system+idle+wait+irq+soft+steal;
    auto idle_total = idle+wait;
    if (old_total && total > old_total && idle_total >= old_idle) {
      cpu_ratio = std::clamp(1.0-double(idle_total-old_idle)/double(total-old_total),0.0,1.0);
      cpu = std::to_string(static_cast<int>(cpu_ratio*100)) + '%';
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
    ram_ratio = (total_mem-available)/total_mem;
    ram = amount(total_mem-available) + '/' + amount(total_mem);
  }
  struct statvfs fs{};
  if (statvfs("/", &fs) == 0 && fs.f_blocks) {
    const double total = double(fs.f_blocks)*fs.f_frsize;
    const double used = double(fs.f_blocks-fs.f_bfree)*fs.f_frsize;
    disk_ratio = used/total; disk = amount(used) + '/' + amount(total);
  }
  double temperature;
  if (std::ifstream("/sys/class/thermal/thermal_zone0/temp") >> temperature) {
    temp = std::to_string(static_cast<int>(temperature/1000)) + " C";
    temp_ratio = temperature/100000.;
  }
  network.clear();
  ifaddrs* addresses = nullptr;
  if (getifaddrs(&addresses) == 0) {
    for (auto* p = addresses; p; p = p->ifa_next) {
      if (!p->ifa_addr || p->ifa_addr->sa_family != AF_INET || (p->ifa_flags & IFF_LOOPBACK)) continue;
      char address[INET_ADDRSTRLEN]{};
      auto* in = reinterpret_cast<sockaddr_in*>(p->ifa_addr);
      if (inet_ntop(AF_INET,&in->sin_addr,address,sizeof(address)))
        network.push_back(std::string(p->ifa_name) + "  " + address);
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
  std::lock_guard<std::mutex> guard(mutex);
  auto result = action; action = DashboardAction::None; return result;
}
void set_dashboard_status(const DashboardStatus& value) {
  std::lock_guard<std::mutex> guard(mutex); status = value;
}
void init_ncurses() {
#ifdef OPENHD_HAVE_CURSES
  std::lock_guard<std::mutex> guard(mutex);
  const char* term = std::getenv("TERM");
  if (active || !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) || !term || std::string(term)=="dumb") return;
  if (!initscr()) return;
  // Complete dynamic log storage initialization before worker threads may see
  // active=true and begin forwarding messages to the dashboard.
  (void)log_buffer();
  cbreak(); noecho(); keypad(stdscr,TRUE); nodelay(stdscr,TRUE); curs_set(0);
  if (has_colors()) {
    start_color();
    const int background = use_default_colors() == OK ? -1 : COLOR_BLACK;
    init_pair(1,COLOR_GREEN,background); init_pair(2,COLOR_RED,background);
    init_pair(3,COLOR_YELLOW,background); init_pair(4,COLOR_CYAN,background);
  }
  active = true;
  std::atexit(shutdown_ncurses);
#endif
}
void shutdown_ncurses() {
#ifdef OPENHD_HAVE_CURSES
  std::lock_guard<std::mutex> guard(mutex);
  if (active.exchange(false)) {
    endwin();
  }
#endif
}
void ncurses_log(const std::string& name, int level, const std::string& message) {
#ifdef OPENHD_HAVE_CURSES
  // Logging can occur from constructors in other translation units. Avoid
  // touching the dynamically initialized log deque until main enables the UI.
  if (!active.load()) return;
  std::lock_guard<std::mutex> guard(mutex);
  auto& logs = log_buffer();
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
    logs.push_back({prefix.str()+clean(line),level});
    if (scroll) ++scroll;
    if (logs.size()>log_limit) logs.pop_front();
  }
  scroll = std::min(scroll,static_cast<int>(logs.size()));
#else
  (void)name; (void)level; (void)message;
#endif
}
void update_ncurses() {
#ifdef OPENHD_HAVE_CURSES
  std::lock_guard<std::mutex> guard(mutex);
  if (!active) return;
  auto& logs = log_buffer();
  for (int key = getch(); key != ERR; key = getch()) {
    if (page == 3 || page == 5) {
      if (key == 'y' || key == 'Y') {
        action = page == 3 ? DashboardAction::Restart : DashboardAction::Shutdown;
        page = 0;
      } else if (key == 27 || key == 'n' || key == 'N') page = 0;
      continue;
    }
    if (key == KEY_F(1)) page = page == 1 ? 0 : 1;
    if (key == KEY_F(2)) page = page == 2 ? 0 : 2;
    if (key == KEY_F(3)) page = 3;
    if (key == KEY_F(5)) page = 5;
    if (key == KEY_F(4)) page = page == 4 ? 0 : 4;
    if (key == 27) page = 0;
    if (key == '\t') { logs_only = !logs_only; page = 0; }
    if (key == KEY_PPAGE) scroll += std::max(1,LINES-4);
    if (key == KEY_NPAGE) scroll = std::max(0,scroll-std::max(1,LINES-4));
    if (key == KEY_UP) ++scroll;
    if (key == KEY_DOWN) scroll = std::max(0,scroll-1);
    if (key == KEY_END) scroll = 0;
  }
  sample(); erase();
  if (COLS < 60 || LINES < 18) {
    put(0,0,status.version,4); put(2,0,"Dashboard needs 60 columns x 18 rows.");
    put(3,0,"Resize terminal. Ctrl+C exits."); refresh(); return;
  }
  int log_top = 1;
  if (!logs_only) {
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
    put(1,info_x,status.version,4,metrics_x-info_x-1);
    put(2,info_x,"Platform: " + status.platform,0,metrics_x-info_x-1);
    put(3,info_x,"Camera: " + status.camera,0,metrics_x-info_x-1);
    put(4,info_x,"Uptime: " + uptime,0,metrics_x-info_x-1);
    auto time = std::time(nullptr); std::tm local{}; localtime_r(&time,&local);
    std::ostringstream date; date << std::put_time(&local,"%b %d %Y %H:%M:%S");
    panel(0,metrics_x,6,32,date.str());
    meter(1,metrics_x+2,28,"CPU ",cpu,cpu_ratio);
    meter(2,metrics_x+2,28,"RAM ",ram,ram_ratio);
    meter(3,metrics_x+2,28,"Disk",disk,disk_ratio);
    meter(4,metrics_x+2,28,"Temp",temp,temp_ratio);
    const int third = COLS/3;
    panel(header,0,7,third,"Link Status");
    panel(header,third,7,third,"Devices");
    panel(header,third*2,7,COLS-third*2,"Network");
    const char* names[] = {"WiFiBroadcast", "Artosyn", "Microhard", "Ethernet", "LTE"};
    const char* short_names[] = {"WiFi", "Art", "MH", "Eth", "LTE"};
    for (int i=0;i<5;++i) {
      put(header+i+1,2,third < 29 ? short_names[i] : names[i],0,third-4);
      const auto& value = status.links[i];
      const int color = value == "Configured" ? 1 : value == "Unknown" ? 3 : 0;
      attron(COLOR_PAIR(color));
      mvaddch(header+i+1,third-14,ACS_BULLET);
      attroff(COLOR_PAIR(color));
      put(header+i+1,third-12,value,color,10);
    }
    put(header+1,third+2,"Camera: " + status.camera,0,third-4);
    put(header+2,third+2,"FC: Unknown",0,third-4);
    put(header+3,third+2,"WiFi: " + status.wifi,0,third-4);
    put(header+4,third+2,"SDR: Unknown",0,third-4);
    if (network.empty()) put(header+1,third*2+2,"No IPv4 address",3,COLS-third*2-4);
    for (size_t i=0;i<network.size() && i<5;++i)
      put(header+1+static_cast<int>(i),third*2+2,network[i],0,COLS-third*2-4);
    log_top = header+7;
  }
  const int visible = std::max(0,LINES-log_top-3);
  scroll = std::clamp(scroll,0,std::max(0,static_cast<int>(logs.size())-visible));
  panel(log_top,0,LINES-log_top-1,COLS,scroll ? "Logs - paused (End: live)" : "Logs - live");
  const int start = std::max(0,static_cast<int>(logs.size())-visible-scroll);
  for (int i=0;i<visible && start+i<static_cast<int>(logs.size());++i) {
    const auto& line = logs[start+i];
    put(log_top+1+i,2,line.text,line.level>=4 ? 2 : line.level==3 ? 3 : 0,COLS-4);
  }
  put(LINES-1,1,COLS >= 100 ? "[F1] Help  [F2] Menu  [F3] Restart  [F4] Network  [F5] Power  [TAB] Switch View  [CTRL+C] Exit" : "F1 Help F2 Menu F3 Restart F4 Net F5 Power Tab View ^C",4);
  if (page) {
    const int width = std::min(COLS-4,76), left = (COLS-width)/2;
    for (int y=3;y<14;++y) put(y,left,std::string(width,' '));
    panel(3,left,11,width,page==3 ? "Restart OpenHD" : page==5 ? "Power off board" : page==4 ? "Network addresses" : page==2 ? "Menu" : "Help");
    if (page==3 || page==5) {
      put(5,left+2,page==3 ? "Restart OpenHD now? Video and telemetry will stop." : "Power off this board? Video and telemetry will stop.",3,width-4);
      put(7,left+2,"Y: confirm    N / Esc: cancel",0,width-4);
    } else if (page==4) {
      for (size_t i=0;i<network.size() && i<7;++i) put(5+static_cast<int>(i),left+2,network[i],0,width-4);
      if (network.empty()) put(5,left+2,"No IPv4 address assigned.");
    } else {
      put(5,left+2,"Tab: dashboard / full-screen logs",0,width-4);
      put(6,left+2,"Up/Down, PgUp/PgDn: scroll logs; End: follow live",0,width-4);
      put(7,left+2,"F4: network interfaces and IPv4 addresses",0,width-4);
      put(8,left+2,"Configured means transport exists, not peer connected.",0,width-4);
      put(9,left+2,"Unknown means no live measurement is available.",0,width-4);
    }
    put(12,left+2,"Esc: close",4,width-4);
  }
  refresh();
#endif
}
}  // namespace openhd::ui
