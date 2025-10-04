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
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

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
              << " --device <path> --baud <baudrate> [--output <file>]"
              << " [--sysid <id> --compid <id> --target-sys <id> --target-comp <id>]" << std::endl;
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

}  // namespace

struct MessageEntry {
    uint64_t count = 0;
    std::string name;
    std::string last_timestamp;
    std::string payload_hex;
    uint8_t sysid = 0;
    uint8_t compid = 0;
    uint8_t len = 0;
};

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

void render_ui(const std::string &device_path, int baudrate, const std::map<uint32_t, MessageEntry> &messages,
               const std::string &output_path, const std::string &status_message) {
    erase();

    int max_y = 0;
    int max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    mvprintw(0, 0, "MAVLink UART Debugger - device: %s @ %d baud", device_path.c_str(), baudrate);
    mvprintw(1, 0, "Logging to: %s", output_path.empty() ? "<disabled>" : output_path.c_str());
    mvprintw(2, 0, "Controls: q=quit | h=send heartbeat | r=send reboot command | p=send ping");
    mvprintw(3, 0, "Status: %s", status_message.c_str());

    const int header_row = 5;
    mvprintw(header_row, 0, "%-6s %-20s %-8s %-4s %-6s %-6s %-27s %s", "MSGID", "NAME", "COUNT", "LEN",
             "SYS", "COMP", "LAST UPDATE", "PAYLOAD (hex)");

    int row = header_row + 1;
    const int max_rows = max_y - row - 1;

    std::vector<std::pair<uint32_t, const MessageEntry *>> sorted_entries;
    sorted_entries.reserve(messages.size());
    for (const auto &kv : messages) {
        sorted_entries.emplace_back(kv.first, &kv.second);
    }
    std::sort(sorted_entries.begin(), sorted_entries.end(),
              [](const auto &lhs, const auto &rhs) { return lhs.first < rhs.first; });

    int displayed = 0;
    for (const auto &[msgid, entry_ptr] : sorted_entries) {
        if (displayed >= max_rows) {
            break;
        }

        const auto &entry = *entry_ptr;
        std::string payload = entry.payload_hex;
        const int payload_start_col = 6 + 1 + 20 + 1 + 8 + 1 + 4 + 1 + 6 + 1 + 6 + 1 + 27 + 1;
        const int available_width = std::max(0, max_x - payload_start_col);
        if (static_cast<int>(payload.size()) > available_width && available_width > 3) {
            payload = payload.substr(0, available_width - 3) + "...";
        }

        mvprintw(row + displayed, 0, "%-6u %-20s %-8lu %-4u %-6u %-6u %-27s %s", msgid, entry.name.c_str(),
                 static_cast<unsigned long>(entry.count), static_cast<unsigned>(entry.len),
                 static_cast<unsigned>(entry.sysid), static_cast<unsigned>(entry.compid),
                 entry.last_timestamp.c_str(), payload.c_str());
        ++displayed;
    }

    if (displayed == 0) {
        mvprintw(row, 0, "Waiting for MAVLink traffic...");
    }

    refresh();
}

int main(int argc, char **argv) {
    std::string device_path;
    std::string output_path;
    int baudrate = 0;
    uint8_t sysid = 1;
    uint8_t compid = 1;
    uint8_t target_sys = 1;
    uint8_t target_comp = 1;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--device" && i + 1 < argc) {
            device_path = argv[++i];
        } else if (arg == "--baud" && i + 1 < argc) {
            baudrate = std::stoi(argv[++i]);
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

    if (device_path.empty() || baudrate == 0) {
        print_usage(argv[0]);
        return 1;
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
    std::map<uint32_t, MessageEntry> messages;
    std::string status_message = "Listening...";

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

                    auto &entry = messages[message.msgid];
                    entry.name = name;
                    entry.count++;
                    entry.last_timestamp = timestamp;
                    entry.payload_hex = payload_hex;
                    entry.sysid = message.sysid;
                    entry.compid = message.compid;
                    entry.len = message.len;

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
            render_ui(device_path, baudrate, messages, output_path, status_message);
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
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    render_ui(device_path, baudrate, messages, output_path, status_message);
    endwin();

    if (output.is_open()) {
        output.flush();
    }

    ::close(fd);
    return 0;
}
