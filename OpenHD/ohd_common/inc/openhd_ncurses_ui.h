#pragma once
#include <string>
#include <memory>
namespace openhd {
namespace ui {
    void init_ncurses();
    void shutdown_ncurses();
    void ncurses_log(const std::string& logger_name, int level, const std::string& msg);
    void update_ncurses();
}
}
