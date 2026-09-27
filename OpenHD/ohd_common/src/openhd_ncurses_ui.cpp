#include "openhd_ncurses_ui.h"
#include <curses.h>
#include <unistd.h>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>

namespace openhd {
namespace ui {

static bool initialized = false;
static WINDOW* log_win = nullptr;
static WINDOW* stat_win = nullptr;
static std::mutex ui_mutex;
static std::vector<std::string> log_buffer;
static int max_log_lines = 1000;
static int log_scroll_offset = 0;

void init_ncurses() {
    if (!isatty(STDOUT_FILENO)) return;
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(4, COLOR_CYAN, -1);

    int h, w;
    getmaxyx(stdscr, h, w);
    stat_win = newwin(10, w, 0, 0);
    log_win = newwin(h - 10, w, 10, 0);
    initialized = true;
    update_ncurses();
}

void shutdown_ncurses() {
    if (!initialized) return;
    endwin();
    initialized = false;
}

void ncurses_log(const std::string& logger_name, int level, const std::string& msg) {
    if (!initialized) return;
    std::lock_guard<std::mutex> lock(ui_mutex);
    
    std::stringstream ss(msg);
    std::string line;
    while(std::getline(ss, line)) {
        log_buffer.push_back("[" + logger_name + "] " + line);
        if (log_buffer.size() > max_log_lines) {
            log_buffer.erase(log_buffer.begin());
        }
    }
}

void update_ncurses() {
    if (!initialized) return;
    std::lock_guard<std::mutex> lock(ui_mutex);
    
    int h, w;
    getmaxyx(stdscr, h, w);
    
    // Check for resize
    int sh, sw;
    getmaxyx(stat_win, sh, sw);
    if (w != sw || h != sh + getmaxy(log_win)) {
        wresize(stat_win, 10, w);
        mvwin(log_win, 10, 0);
        wresize(log_win, h - 10, w);
    }
    
    werase(stat_win);
    box(stat_win, 0, 0);
    mvwprintw(stat_win, 0, 2, " OpenHD 3.0.0-evo ");
    mvwprintw(stat_win, 1, 2, "------- OpenSource -------");
    mvwprintw(stat_win, 2, 2, "Platform : RPI 4");
    mvwprintw(stat_win, 3, 2, "Camera   : VEYE_2MP");
    mvwprintw(stat_win, 4, 2, "Uptime   : ...");
    
    mvwprintw(stat_win, 1, 30, "Link Status");
    mvwprintw(stat_win, 2, 30, "WiFiBroadcast : N/A");
    mvwprintw(stat_win, 3, 30, "Artosyn       : N/A");
    mvwprintw(stat_win, 4, 30, "Ethernet      : N/A");
    mvwprintw(stat_win, 5, 30, "LTE           : N/A");
    
    mvwprintw(stat_win, 8, 2, "[F1] Help");
    wrefresh(stat_win);
    
    werase(log_win);
    box(log_win, 0, 0);
    mvwprintw(log_win, 0, 2, " Logs ");
    
    int log_h, log_w;
    getmaxyx(log_win, log_h, log_w);
    int visible_lines = log_h - 2;
    int start_idx = (int)log_buffer.size() - visible_lines;
    if (start_idx < 0) start_idx = 0;
    
    int y = 1;
    for (int i = start_idx; i < log_buffer.size() && y <= visible_lines; ++i) {
        mvwprintw(log_win, y++, 2, "%.*s", log_w - 4, log_buffer[i].c_str());
    }
    wrefresh(log_win);
}

}
}
