#pragma once
#include <string>
#include <memory>
namespace openhd {
namespace ui {
    enum class DashboardAction { None, Restart, Shutdown };
    DashboardAction take_dashboard_action();
    struct DashboardStatus {
        std::string version = "OpenHD";
        std::string platform = "Unknown";
        std::string camera = "N/A";
        std::string wifi = "Unknown";
        std::string links[5] = {"Unknown", "Unknown", "Unknown", "Unknown", "Unknown"};
    };
    void set_dashboard_status(const DashboardStatus& status);
    bool ncurses_active();
    void init_ncurses();
    void shutdown_ncurses();
    void ncurses_log(const std::string& logger_name, int level, const std::string& msg);
    void update_ncurses();
}
}
