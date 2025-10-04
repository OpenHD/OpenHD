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
#include <unistd.h>

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
              << " --device <path> --baud <baudrate> --output <file>" << std::endl;
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

int main(int argc, char **argv) {
    std::string device_path;
    std::string output_path;
    int baudrate = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--device" && i + 1 < argc) {
            device_path = argv[++i];
        } else if (arg == "--baud" && i + 1 < argc) {
            baudrate = std::stoi(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (device_path.empty() || output_path.empty() || baudrate == 0) {
        print_usage(argv[0]);
        return 1;
    }

    auto speed_constant = baudrate_to_constant(baudrate);
    if (!speed_constant.has_value()) {
        std::cerr << "Unsupported baudrate: " << baudrate << std::endl;
        return 1;
    }

    int fd = ::open(device_path.c_str(), O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (!configure_serial(fd, *speed_constant)) {
        ::close(fd);
        return 1;
    }

    std::ofstream output(output_path, std::ios::out | std::ios::app);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file: " << output_path << std::endl;
        ::close(fd);
        return 1;
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    mavlink_message_t message{};
    mavlink_status_t status{};

    std::cout << "Listening on " << device_path << " @ " << baudrate << " baud. Press Ctrl+C to exit." << std::endl;

    while (!g_should_exit) {
        uint8_t buffer[256];
        ssize_t nread = ::read(fd, buffer, sizeof(buffer));
        if (nread < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(10000);
                continue;
            }
            perror("read");
            break;
        } else if (nread == 0) {
            usleep(10000);
            continue;
        }

        for (ssize_t i = 0; i < nread; ++i) {
            if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &message, &status)) {
                const auto timestamp = current_timestamp_string();
                const auto name = message_name(message);
                const auto payload_hex = payload_to_hex(message);

                std::ostringstream line;
                line << timestamp << ", msgid=" << message.msgid << ", name=" << name
                     << ", sys=" << static_cast<int>(message.sysid)
                     << ", comp=" << static_cast<int>(message.compid)
                     << ", len=" << static_cast<int>(message.len)
                     << ", payload=" << payload_hex;

                const std::string output_line = line.str();
                output << output_line << '\n';
                output.flush();
                std::cout << output_line << std::endl;
            }
        }
    }

    std::cout << "Exiting." << std::endl;
    ::close(fd);
    return 0;
}
