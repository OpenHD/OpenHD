#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <ctime>
#include <set>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <filesystem>

#include <ncurses.h>

#define MAVLINK_USE_MESSAGE_INFO
extern "C" {
#include "mavlink/v2.0/openhd/mavlink.h"
#include "mavlink/v2.0/mavlink_get_info.h"
}

namespace {
volatile std::sig_atomic_t g_should_exit = 0;

void signal_handler(int) {
    g_should_exit = 1;
}

std::optional<speed_t> baudrate_to_constant(int baudrate) {
    static const std::map<int, speed_t> kBaudrateMap = {
        {50, B50},       {75, B75},       {110, B110},     {134, B134},
        {150, B150},     {200, B200},     {300, B300},     {600, B600},
        {1200, B1200},   {1800, B1800},   {2400, B2400},   {4800, B4800},
        {9600, B9600},   {19200, B19200}, {38400, B38400}, {57600, B57600},
        {115200, B115200}, {230400, B230400}, {460800, B460800},
#ifdef B500000
        {500000, B500000},
#endif
#ifdef B576000
        {576000, B576000},
#endif
        {921600, B921600},
#ifdef B1000000
        {1000000, B1000000},
#endif
#ifdef B1500000
        {1500000, B1500000},
#endif
#ifdef B2000000
        {2000000, B2000000},
#endif
    };

    auto it = kBaudrateMap.find(baudrate);
    if (it == kBaudrateMap.end()) {
        return std::nullopt;
    }
    return it->second;
}

void print_usage(const char *program) {
    std::cerr << "Usage: " << program
              << " [--device <path>] [--baud <baudrate>] [--output <file>]"
              << " [--sysid <id> --compid <id> --target-sys <id> --target-comp <id>]" << std::endl;
    std::cerr << "Defaults: baud=115200, device=/dev/serialX (first available)" << std::endl;
}

std::optional<std::string> find_default_serial_device() {
    namespace fs = std::filesystem;
    const fs::path dev_dir{"/dev"};

    std::error_code ec;
    if (!fs::exists(dev_dir, ec) || ec || !fs::is_directory(dev_dir, ec) || ec) {
        return std::nullopt;
    }

    std::vector<fs::path> candidates;
    for (const auto &entry : fs::directory_iterator(dev_dir, ec)) {
        if (ec) {
            break;
        }

        const auto &path = entry.path();
        const auto filename = path.filename().string();
        if (filename.rfind("serial", 0) != 0) {
            continue;
        }

        std::error_code status_ec;
        const auto status = entry.symlink_status(status_ec);
        if (status_ec) {
            continue;
        }

        if (fs::is_directory(status)) {
            continue;
        }

        if (!fs::is_character_file(status) && !fs::is_symlink(status)) {
            continue;
        }

        candidates.push_back(path);
    }

    if (ec || candidates.empty()) {
        return std::nullopt;
    }

    std::sort(candidates.begin(), candidates.end());
    return candidates.front().string();
}

bool configure_serial(int fd, speed_t speed_constant) {
    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return false;
    }

    cfmakeraw(&tty);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CS8;

    if (cfsetispeed(&tty, speed_constant) != 0 || cfsetospeed(&tty, speed_constant) != 0) {
        perror("cfsetispeed/cfsetospeed");
        return false;
    }

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return false;
    }

    return true;
}

std::string payload_to_hex(const mavlink_message_t &message) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    const auto *payload = reinterpret_cast<const uint8_t *>(message.payload64);
    for (uint16_t i = 0; i < message.len; ++i) {
        oss << std::setw(2) << static_cast<int>(payload[i]);
        if (i + 1 != message.len) {
            oss << ' ';
        }
    }
    return oss.str();
}

std::string current_timestamp_string() {
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto now_us = duration_cast<microseconds>(now.time_since_epoch());
    const auto seconds_part = duration_cast<std::chrono::seconds>(now_us);
    const auto micros = now_us - seconds_part;

    std::time_t t = seconds_part.count();
    std::tm tm{};
    gmtime_r(&t, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setw(6) << std::setfill('0') << micros.count() << "Z";
    return oss.str();
}

std::string message_name(const mavlink_message_t &message) {
    const mavlink_message_info_t *info = mavlink_get_message_info_by_id(message.msgid);
    if (!info || !info->name) {
        return "UNKNOWN";
    }
    return std::string(info->name);
}

using MessageKey = uint64_t;

MessageKey make_message_key(uint8_t sysid, uint8_t compid, uint32_t msgid) {
    return (static_cast<MessageKey>(sysid) << 40) | (static_cast<MessageKey>(compid) << 32) |
           static_cast<MessageKey>(msgid);
}

struct FilterState {
    std::optional<uint8_t> sysid;
    std::optional<uint8_t> compid;
    std::optional<uint32_t> msgid;
};

struct MessageEntry {
    uint64_t count = 0;
    std::string name;
    std::string last_timestamp;
    std::string payload_hex;
    uint8_t sysid = 0;
    uint8_t compid = 0;
    uint8_t len = 0;
    uint32_t msgid = 0;
};

std::vector<uint8_t> collect_sysids(const std::map<MessageKey, MessageEntry> &messages) {
    std::set<uint8_t> sysids;
    for (const auto &kv : messages) {
        sysids.insert(kv.second.sysid);
    }
    return std::vector<uint8_t>(sysids.begin(), sysids.end());
}

std::vector<uint8_t> collect_compids(const std::map<MessageKey, MessageEntry> &messages, uint8_t sysid) {
    std::set<uint8_t> compids;
    for (const auto &kv : messages) {
        if (kv.second.sysid == sysid) {
            compids.insert(kv.second.compid);
        }
    }
    return std::vector<uint8_t>(compids.begin(), compids.end());
}

std::vector<uint32_t> collect_msgids(const std::map<MessageKey, MessageEntry> &messages, uint8_t sysid,
                                     uint8_t compid) {
    std::set<uint32_t> msgids;
    for (const auto &kv : messages) {
        if (kv.second.sysid == sysid && kv.second.compid == compid) {
            msgids.insert(kv.second.msgid);
        }
    }
    return std::vector<uint32_t>(msgids.begin(), msgids.end());
}

int render_table(int start_row, int max_y, int max_x, const std::string &title,
                 const std::vector<const MessageEntry *> &entries) {
    if (start_row >= max_y) {
        return max_y;
    }

    mvprintw(start_row++, 0, "%s", title.c_str());
    if (start_row >= max_y) {
        return max_y;
    }

    mvprintw(start_row++, 0, "%-5s %-5s %-6s %-20s %-8s %-4s %-27s %s", "SYS", "COMP", "MSG", "NAME",
             "COUNT", "LEN", "LAST UPDATE", "PAYLOAD (hex)");
    if (start_row >= max_y) {
        return max_y;
    }

    int available_rows = max_y - start_row - 1;
    if (available_rows < 0) {
        available_rows = 0;
    }

    int displayed = 0;
    for (const auto *entry : entries) {
        if (displayed >= available_rows) {
            break;
        }

        std::string payload = entry->payload_hex;
        const int payload_start_col = 5 + 1 + 5 + 1 + 6 + 1 + 20 + 1 + 8 + 1 + 4 + 1 + 27 + 1;
        const int available_width = std::max(0, max_x - payload_start_col);
        if (static_cast<int>(payload.size()) > available_width && available_width > 3) {
            payload = payload.substr(0, available_width - 3) + "...";
        }

        mvprintw(start_row + displayed, 0, "%-5u %-5u %-6u %-20s %-8lu %-4u %-27s %s",
                 static_cast<unsigned>(entry->sysid), static_cast<unsigned>(entry->compid),
                 static_cast<unsigned>(entry->msgid), entry->name.c_str(),
                 static_cast<unsigned long>(entry->count), static_cast<unsigned>(entry->len),
                 entry->last_timestamp.c_str(), payload.c_str());
        ++displayed;
    }

    if (displayed == 0) {
        mvprintw(start_row, 0, "<no entries>");
        start_row += 1;
    } else {
        start_row += displayed;
    }

    if (start_row < max_y) {
        ++start_row;
    }

    return start_row;
}

template <typename T>
std::optional<T> cycle_optional(const std::vector<T> &values, const std::optional<T> &current) {
    if (values.empty()) {
        return std::nullopt;
    }

    if (!current.has_value()) {
        return values.front();
    }

    auto it = std::find(values.begin(), values.end(), *current);
    if (it == values.end()) {
        return values.front();
    }

    ++it;
    if (it == values.end()) {
        return std::nullopt;
    }
    return *it;
}

bool write_message(int fd, const mavlink_message_t &message) {
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    const auto length = mavlink_msg_to_send_buffer(buffer, &message);
    ssize_t written = ::write(fd, buffer, length);
    return written == static_cast<ssize_t>(length);
}

bool send_heartbeat(int fd, uint8_t sysid, uint8_t compid, uint8_t target_sys, uint8_t target_comp) {
    (void)target_sys;
    (void)target_comp;
    mavlink_message_t msg{};
    mavlink_msg_heartbeat_pack(sysid, compid, &msg, MAV_TYPE_GENERIC, MAV_AUTOPILOT_INVALID,
                               MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 0, MAV_STATE_ACTIVE);
    return write_message(fd, msg);
}

bool send_reboot(int fd, uint8_t sysid, uint8_t compid, uint8_t target_sys, uint8_t target_comp) {
    mavlink_message_t msg{};
    constexpr float kParam1RebootAutopilot = 1.0f;
    mavlink_msg_command_long_pack(sysid, compid, &msg, target_sys, target_comp,
                                  MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN, 0, kParam1RebootAutopilot,
                                  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    return write_message(fd, msg);
}

bool send_ping(int fd, uint8_t sysid, uint8_t compid, uint8_t target_sys, uint8_t target_comp) {
    static uint32_t sequence = 0;
    mavlink_message_t msg{};
    const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch());
    mavlink_msg_ping_pack(sysid, compid, &msg, now.count(), target_sys, target_comp, sequence++);
    return write_message(fd, msg);
}

void render_ui(const std::string &device_path, int baudrate,
               const std::map<MessageKey, MessageEntry> &messages, const std::string &output_path,
               const std::string &status_message, const FilterState &filter) {
    erase();

    int max_y = 0;
    int max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    mvprintw(0, 0, "MAVLink UART Debugger - device: %s @ %d baud", device_path.c_str(), baudrate);
    mvprintw(1, 0, "Logging to: %s", output_path.empty() ? "<disabled>" : output_path.c_str());
    mvprintw(2, 0, "Controls: q=quit | h=send heartbeat | r=send reboot command | p=send ping");
    mvprintw(3, 0, "Status: %s", status_message.c_str());
    mvprintw(4, 0, "Filter controls: s=cycle sysid | c=cycle comp | m=cycle message | f=clear filters");

    const std::string sys_str = filter.sysid ? std::to_string(*filter.sysid) : std::string("All");
    const std::string comp_str = filter.compid ? std::to_string(*filter.compid) : std::string("All");
    const std::string msg_str = filter.msgid ? std::to_string(*filter.msgid) : std::string("All");
    mvprintw(5, 0, "Active filter: sys=%s comp=%s msg=%s", sys_str.c_str(), comp_str.c_str(),
             msg_str.c_str());

    int row = 7;

    std::vector<const MessageEntry *> all_entries;
    all_entries.reserve(messages.size());
    for (const auto &kv : messages) {
        all_entries.push_back(&kv.second);
    }

    std::sort(all_entries.begin(), all_entries.end(), [](const auto *lhs, const auto *rhs) {
        if (lhs->sysid != rhs->sysid) {
            return lhs->sysid < rhs->sysid;
        }
        if (lhs->compid != rhs->compid) {
            return lhs->compid < rhs->compid;
        }
        if (lhs->msgid != rhs->msgid) {
            return lhs->msgid < rhs->msgid;
        }
        return lhs->name < rhs->name;
    });

    row = render_table(row, max_y, max_x, "All Messages", all_entries);

    if (row >= max_y) {
        refresh();
        return;
    }

    std::vector<const MessageEntry *> filtered_entries;
    filtered_entries.reserve(all_entries.size());
    for (const auto *entry : all_entries) {
        if (filter.sysid && entry->sysid != *filter.sysid) {
            continue;
        }
        if (filter.compid && entry->compid != *filter.compid) {
            continue;
        }
        if (filter.msgid && entry->msgid != *filter.msgid) {
            continue;
        }
        filtered_entries.push_back(entry);
    }

    if (!filter.sysid && !filter.compid && !filter.msgid) {
        if (row < max_y) {
            mvprintw(row, 0, "Filtered Messages: (press s/c/m to apply filters)");
            if (row + 1 < max_y) {
                row += 2;
            } else {
                row = max_y;
            }
        }
    } else if (row < max_y) {
        row = render_table(row, max_y, max_x, "Filtered Messages", filtered_entries);
    }

    refresh();
}

}  // namespace

int main(int argc, char **argv) {
    std::string device_path;
    std::string output_path;
    int baudrate = 0;
    constexpr int kDefaultBaudrate = 115200;
    bool device_user_specified = false;
    bool baud_user_specified = false;
    uint8_t sysid = 1;
    uint8_t compid = 1;
    uint8_t target_sys = 1;
    uint8_t target_comp = 1;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--device" && i + 1 < argc) {
            device_path = argv[++i];
            device_user_specified = true;
        } else if (arg == "--baud" && i + 1 < argc) {
            baudrate = std::stoi(argv[++i]);
            baud_user_specified = true;
        } else if (arg == "--output" && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg == "--sysid" && i + 1 < argc) {
            sysid = static_cast<uint8_t>(std::stoi(argv[++i]));
        } else if (arg == "--compid" && i + 1 < argc) {
            compid = static_cast<uint8_t>(std::stoi(argv[++i]));
        } else if (arg == "--target-sys" && i + 1 < argc) {
            target_sys = static_cast<uint8_t>(std::stoi(argv[++i]));
        } else if (arg == "--target-comp" && i + 1 < argc) {
            target_comp = static_cast<uint8_t>(std::stoi(argv[++i]));
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            std::cerr << "Additional options: --sysid <id> --compid <id> --target-sys <id> --target-comp <id>"
                      << std::endl;
            return 0;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (device_path.empty()) {
        if (auto detected_device = find_default_serial_device()) {
            device_path = *detected_device;
            std::cerr << "No device specified, using " << device_path << std::endl;
        } else {
            std::cerr << "No device specified and unable to find a /dev/serialX device" << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    if (baudrate == 0) {
        baudrate = kDefaultBaudrate;
        std::cerr << "No baudrate specified, defaulting to " << kDefaultBaudrate << std::endl;
    }

    auto speed_constant = baudrate_to_constant(baudrate);
    if (!speed_constant.has_value()) {
        std::cerr << "Unsupported baudrate: " << baudrate << std::endl;
        return 1;
    }

    int fd = ::open(device_path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (!configure_serial(fd, *speed_constant)) {
        ::close(fd);
        return 1;
    }

    std::ofstream output;
    if (!output_path.empty()) {
        output.open(output_path, std::ios::out | std::ios::app);
        if (!output.is_open()) {
            std::cerr << "Failed to open output file: " << output_path << std::endl;
            ::close(fd);
            return 1;
        }
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    initscr();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    mavlink_message_t message{};
    mavlink_status_t status{};
    std::map<MessageKey, MessageEntry> messages;
    std::string status_message = "Listening...";
    FilterState filter;
    if (!device_user_specified || !baud_user_specified) {
        status_message += " (auto:";
        bool need_separator = false;
        if (!device_user_specified) {
            status_message += " device=" + device_path;
            need_separator = true;
        }
        if (!baud_user_specified) {
            if (need_separator) {
                status_message += ',';
            }
            status_message += " baud=" + std::to_string(baudrate);
        }
        status_message += ')';
    }

    const auto start_time = std::chrono::steady_clock::now();
    auto last_ui_update = start_time - std::chrono::milliseconds(200);

    while (!g_should_exit) {
        uint8_t buffer[256];
        ssize_t nread = ::read(fd, buffer, sizeof(buffer));
        if (nread < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                status_message = std::string("read error: ") + std::strerror(errno);
                break;
            }
        } else if (nread > 0) {
            for (ssize_t i = 0; i < nread; ++i) {
                if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &message, &status)) {
                    const auto timestamp = current_timestamp_string();
                    const auto name = message_name(message);
                    const auto payload_hex = payload_to_hex(message);

                    const MessageKey key =
                        make_message_key(message.sysid, message.compid, message.msgid);
                    auto &entry = messages[key];
                    entry.name = name;
                    entry.count++;
                    entry.last_timestamp = timestamp;
                    entry.payload_hex = payload_hex;
                    entry.sysid = message.sysid;
                    entry.compid = message.compid;
                    entry.len = message.len;
                    entry.msgid = message.msgid;

                    if (output.is_open()) {
                        output << timestamp << ", msgid=" << message.msgid << ", name=" << name
                               << ", sys=" << static_cast<int>(message.sysid)
                               << ", comp=" << static_cast<int>(message.compid)
                               << ", len=" << static_cast<int>(message.len)
                               << ", payload=" << payload_hex << '\n';
                        output.flush();
                    }
                }
            }
        }

        const auto now = std::chrono::steady_clock::now();
        if (now - last_ui_update > std::chrono::milliseconds(100)) {
            render_ui(device_path, baudrate, messages, output_path, status_message, filter);
            last_ui_update = now;
        }

        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q' || ch == 'Q') {
                g_should_exit = 1;
            } else if (ch == 'h' || ch == 'H') {
                status_message = send_heartbeat(fd, sysid, compid, target_sys, target_comp)
                                     ? "Heartbeat sent"
                                     : "Failed to send heartbeat";
            } else if (ch == 'r' || ch == 'R') {
                status_message = send_reboot(fd, sysid, compid, target_sys, target_comp)
                                     ? "Reboot command sent"
                                     : "Failed to send reboot command";
            } else if (ch == 'p' || ch == 'P') {
                status_message = send_ping(fd, sysid, compid, target_sys, target_comp)
                                     ? "Ping sent"
                                     : "Failed to send ping";
            } else if (ch == 's' || ch == 'S') {
                auto sysids = collect_sysids(messages);
                if (sysids.empty()) {
                    status_message = "No system IDs available to filter";
                } else {
                    auto next = cycle_optional(sysids, filter.sysid);
                    if (!next.has_value()) {
                        filter.sysid.reset();
                        filter.compid.reset();
                        filter.msgid.reset();
                        status_message = "System ID filter cleared";
                    } else {
                        filter.sysid = next;
                        filter.compid.reset();
                        filter.msgid.reset();
                        status_message = "Filtering system ID " + std::to_string(*filter.sysid);
                    }
                }
            } else if (ch == 'c' || ch == 'C') {
                if (!filter.sysid) {
                    status_message = "Select a system ID first (press 's')";
                } else {
                    auto compids = collect_compids(messages, *filter.sysid);
                    if (compids.empty()) {
                        filter.compid.reset();
                        filter.msgid.reset();
                        status_message = "No component IDs for system " + std::to_string(*filter.sysid);
                    } else {
                        auto next = cycle_optional(compids, filter.compid);
                        if (!next.has_value()) {
                            filter.compid.reset();
                            filter.msgid.reset();
                            status_message = "Component ID filter cleared";
                        } else {
                            filter.compid = next;
                            filter.msgid.reset();
                            status_message = "Filtering system " + std::to_string(*filter.sysid) +
                                             " component " + std::to_string(*filter.compid);
                        }
                    }
                }
            } else if (ch == 'm' || ch == 'M') {
                if (!filter.sysid || !filter.compid) {
                    status_message = "Select system and component filters first (press 's' then 'c')";
                } else {
                    auto msgids = collect_msgids(messages, *filter.sysid, *filter.compid);
                    if (msgids.empty()) {
                        filter.msgid.reset();
                        status_message = "No messages for sys " + std::to_string(*filter.sysid) +
                                         " comp " + std::to_string(*filter.compid);
                    } else {
                        auto next = cycle_optional(msgids, filter.msgid);
                        if (!next.has_value()) {
                            filter.msgid.reset();
                            status_message = "Message filter cleared";
                        } else {
                            filter.msgid = next;
                            status_message = "Filtering msgid " + std::to_string(*filter.msgid) +
                                             " (sys " + std::to_string(*filter.sysid) + ", comp " +
                                             std::to_string(*filter.compid) + ')';
                        }
                    }
                }
            } else if (ch == 'f' || ch == 'F') {
                filter.sysid.reset();
                filter.compid.reset();
                filter.msgid.reset();
                status_message = "Filters cleared";
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    render_ui(device_path, baudrate, messages, output_path, status_message, filter);
    endwin();

    if (output.is_open()) {
        output.flush();
    }

    ::close(fd);
    return 0;
}
