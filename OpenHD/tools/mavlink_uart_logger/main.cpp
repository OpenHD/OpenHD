#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <deque>
#include <filesystem>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <ctime>
#include <string>
#include <sys/socket.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

#include <ncurses.h>

#define MAVLINK_USE_MESSAGE_INFO
extern "C" {
#include "mavlink/v2.0/openhd/mavlink.h"
#include "mavlink/v2.0/mavlink_get_info.h"
}

namespace {
volatile std::sig_atomic_t g_should_exit = 0;

constexpr short kHighlightColorPair = 1;
constexpr short kOpenHDBlueColorSlot = 10;
bool g_highlight_colors_available = false;

void signal_handler(int) {
    g_should_exit = 1;
}

enum class TransportMode { Serial, UDP };

struct Transport {
    TransportMode mode = TransportMode::Serial;
    int fd = -1;
    sockaddr_in udp_default_destination{};
    bool udp_has_default_destination = false;
    std::optional<sockaddr_in> udp_last_sender;
};

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
              << " [--udp [port]]"
              << " [--sysid <id> --compid <id> --target-sys <id> --target-comp <id>]"
              << " [--transmit] [--loop] [--raw]" << std::endl;
    std::cerr << "Defaults: baud=115200, device=/dev/serialX (first available), UDP port=5920" << std::endl;
}

std::optional<std::string> find_default_serial_device() {
    namespace fs = std::filesystem;
    const fs::path dev_dir{"/dev"};

    std::error_code ec;
    if (!fs::exists(dev_dir, ec) || ec || !fs::is_directory(dev_dir, ec) || ec) {
        return std::nullopt;
    }

    std::vector<fs::path> serial_candidates;
    std::vector<fs::path> ttyS_candidates;
    for (const auto &entry : fs::directory_iterator(dev_dir, ec)) {
        if (ec) {
            break;
        }

        const auto &path = entry.path();
        const auto filename = path.filename().string();
        if (filename.rfind("serial", 0) != 0 && filename.rfind("ttyS", 0) != 0) {
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

        if (filename.rfind("serial", 0) == 0) {
            serial_candidates.push_back(path);
        } else if (filename.rfind("ttyS", 0) == 0) {
            ttyS_candidates.push_back(path);
        }
    }

    if (ec) {
        return std::nullopt;
    }

    auto sort_and_pick = [](std::vector<fs::path> &paths) -> std::optional<std::string> {
        if (paths.empty()) {
            return std::nullopt;
        }
        std::sort(paths.begin(), paths.end());
        return paths.front().string();
    };

    if (auto result = sort_and_pick(serial_candidates)) {
        return result;
    }
    return sort_and_pick(ttyS_candidates);
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

std::size_t field_type_size(mavlink_message_type_t type) {
    switch (type) {
    case MAVLINK_TYPE_CHAR:
    case MAVLINK_TYPE_UINT8_T:
    case MAVLINK_TYPE_INT8_T:
        return 1;
    case MAVLINK_TYPE_UINT16_T:
    case MAVLINK_TYPE_INT16_T:
        return 2;
    case MAVLINK_TYPE_UINT32_T:
    case MAVLINK_TYPE_INT32_T:
    case MAVLINK_TYPE_FLOAT:
        return 4;
    case MAVLINK_TYPE_UINT64_T:
    case MAVLINK_TYPE_INT64_T:
    case MAVLINK_TYPE_DOUBLE:
        return 8;
    default:
        return 0;
    }
}

template <typename T>
T read_scalar(const uint8_t *data) {
    T value{};
    std::memcpy(&value, data, sizeof(T));
    return value;
}

std::string format_scalar(mavlink_message_type_t type, const uint8_t *data) {
    std::ostringstream value_stream;
    value_stream << std::dec;
    switch (type) {
    case MAVLINK_TYPE_CHAR: {
        const char c = read_scalar<char>(data);
        const unsigned char uc = static_cast<unsigned char>(c);
        if (std::isprint(uc)) {
            value_stream << '\'' << c << '\'';
        } else {
            value_stream << static_cast<int>(uc);
        }
        break;
    }
    case MAVLINK_TYPE_UINT8_T:
        value_stream << static_cast<unsigned int>(read_scalar<uint8_t>(data));
        break;
    case MAVLINK_TYPE_INT8_T:
        value_stream << static_cast<int>(read_scalar<int8_t>(data));
        break;
    case MAVLINK_TYPE_UINT16_T:
        value_stream << read_scalar<uint16_t>(data);
        break;
    case MAVLINK_TYPE_INT16_T:
        value_stream << read_scalar<int16_t>(data);
        break;
    case MAVLINK_TYPE_UINT32_T:
        value_stream << read_scalar<uint32_t>(data);
        break;
    case MAVLINK_TYPE_INT32_T:
        value_stream << read_scalar<int32_t>(data);
        break;
    case MAVLINK_TYPE_UINT64_T:
        value_stream << read_scalar<uint64_t>(data);
        break;
    case MAVLINK_TYPE_INT64_T:
        value_stream << read_scalar<int64_t>(data);
        break;
    case MAVLINK_TYPE_FLOAT:
        value_stream << read_scalar<float>(data);
        break;
    case MAVLINK_TYPE_DOUBLE:
        value_stream << read_scalar<double>(data);
        break;
    default:
        value_stream << '?';
        break;
    }
    return value_stream.str();
}

std::string decode_payload(const mavlink_message_t &message) {
    const mavlink_message_info_t *info = mavlink_get_message_info(&message);
    if (!info || info->num_fields == 0) {
        return payload_to_hex(message);
    }

    const auto *payload = reinterpret_cast<const uint8_t *>(message.payload64);
    std::ostringstream oss;
    bool has_field_output = false;

    for (unsigned int i = 0; i < info->num_fields; ++i) {
        const auto &field = info->fields[i];
        const std::size_t element_size = field_type_size(field.type);
        if (element_size == 0) {
            continue;
        }

        if (has_field_output) {
            oss << ", ";
        }
        has_field_output = true;

        oss << field.name << '=';

        if (field.wire_offset >= message.len) {
            oss << "<missing>";
            continue;
        }

        const uint8_t *field_ptr = payload + field.wire_offset;
        const std::size_t available_bytes = message.len - field.wire_offset;

        if (field.array_length > 0) {
            if (field.type == MAVLINK_TYPE_CHAR) {
                const std::size_t max_len = std::min<std::size_t>(field.array_length, available_bytes);
                std::string value(reinterpret_cast<const char *>(field_ptr), max_len);
                const auto null_pos = value.find('\0');
                if (null_pos != std::string::npos) {
                    value.resize(null_pos);
                }
                oss << '"' << value << '"';
            } else {
                const std::size_t max_count = available_bytes / element_size;
                const std::size_t count = std::min<std::size_t>(field.array_length, max_count);
                oss << '[';
                for (std::size_t idx = 0; idx < count; ++idx) {
                    if (idx > 0) {
                        oss << ", ";
                    }
                    oss << format_scalar(field.type, field_ptr + idx * element_size);
                }
                if (count < field.array_length) {
                    if (count > 0) {
                        oss << ", ";
                    }
                    oss << "...";
                }
                oss << ']';
            }
        } else {
            if (available_bytes < element_size) {
                oss << "<truncated>";
            } else {
                oss << format_scalar(field.type, field_ptr);
            }
        }
    }

    if (!has_field_output) {
        return payload_to_hex(message);
    }

    return oss.str();
}

std::string bytes_to_hex(const uint8_t *data, std::size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
        if (i + 1 != len) {
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
    std::string payload_text;
    uint8_t sysid = 0;
    uint8_t compid = 0;
    uint8_t len = 0;
    uint32_t msgid = 0;
};

struct NavigationState {
    bool interactive_mode = false;
    bool focus_filtered = false;
    std::size_t selected_all_index = 0;
    std::size_t selected_filtered_index = 0;
};

struct TableRenderResult {
    int next_row = 0;
    const MessageEntry *highlighted_entry = nullptr;
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

void record_message(const mavlink_message_t &message, const std::string &timestamp,
                    const std::string &name, const std::string &payload_text,
                    std::map<MessageKey, MessageEntry> &messages, std::ofstream *output) {
    const MessageKey key = make_message_key(message.sysid, message.compid, message.msgid);
    auto &entry = messages[key];
    entry.name = name;
    entry.count++;
    entry.payload_text = payload_text;
    entry.sysid = message.sysid;
    entry.compid = message.compid;
    entry.len = message.len;
    entry.msgid = message.msgid;

    if (output && output->is_open()) {
        *output << timestamp << ", msgid=" << message.msgid << ", name=" << name
                << ", sys=" << static_cast<int>(message.sysid)
                << ", comp=" << static_cast<int>(message.compid)
                << ", len=" << static_cast<int>(message.len)
                << ", payload=" << payload_text << '\n';
        output->flush();
    }
}

std::vector<const MessageEntry *> gather_sorted_entries(const std::map<MessageKey, MessageEntry> &messages) {
    std::vector<const MessageEntry *> entries;
    entries.reserve(messages.size());
    for (const auto &kv : messages) {
        entries.push_back(&kv.second);
    }

    std::sort(entries.begin(), entries.end(), [](const auto *lhs, const auto *rhs) {
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

    return entries;
}

std::vector<const MessageEntry *> filter_entries(const std::vector<const MessageEntry *> &entries,
                                                const FilterState &filter) {
    std::vector<const MessageEntry *> filtered;
    filtered.reserve(entries.size());
    for (const auto *entry : entries) {
        if (filter.sysid && entry->sysid != *filter.sysid) {
            continue;
        }
        if (filter.compid && entry->compid != *filter.compid) {
            continue;
        }
        if (filter.msgid && entry->msgid != *filter.msgid) {
            continue;
        }
        filtered.push_back(entry);
    }
    return filtered;
}

std::vector<std::string> wrap_text(const std::string &text, int width) {
    std::vector<std::string> lines;
    if (width <= 0) {
        lines.push_back(text);
        return lines;
    }

    std::size_t start = 0;
    const std::size_t length = text.size();
    while (start < length) {
        std::size_t end = std::min<std::size_t>(start + static_cast<std::size_t>(width), length);
        if (end < length) {
            std::size_t last_space = text.rfind(' ', end - 1);
            if (last_space != std::string::npos && last_space >= start) {
                end = last_space + 1;
            }
        }

        if (end <= start) {
            end = std::min<std::size_t>(start + static_cast<std::size_t>(width), length);
        }

        std::string line = text.substr(start, end - start);
        while (!line.empty() && line.back() == ' ') {
            line.pop_back();
        }
        lines.push_back(std::move(line));
        start = end;
        while (start < length && text[start] == ' ') {
            ++start;
        }
    }

    if (lines.empty()) {
        lines.emplace_back();
    }

    return lines;
}

struct GeneratedMessage {
    mavlink_message_t message;
    std::string description;
};

GeneratedMessage generate_random_openhd_message(std::mt19937 &rng, uint8_t sysid, uint8_t compid) {
    using MessageGenerator = std::function<GeneratedMessage(std::mt19937 &, uint8_t, uint8_t)>;

    static const std::vector<MessageGenerator> generators = {
        [](std::mt19937 &rng, uint8_t sysid, uint8_t compid) {
            std::uniform_int_distribution<int> version_dist(0, 5);
            std::uniform_int_distribution<int> release_dist(0, 3);
            mavlink_message_t msg{};
            const uint8_t major = static_cast<uint8_t>(version_dist(rng) + 1);
            const uint8_t minor = static_cast<uint8_t>(version_dist(rng));
            const uint8_t patch = static_cast<uint8_t>(version_dist(rng));
            const uint8_t release_type = static_cast<uint8_t>(release_dist(rng));
            const uint8_t dummy = 0;
            mavlink_msg_openhd_version_message_pack(sysid, compid, &msg, major, minor, patch, release_type, dummy);
            std::ostringstream oss;
            oss << "openhd_version_message major=" << static_cast<int>(major)
                << " minor=" << static_cast<int>(minor)
                << " patch=" << static_cast<int>(patch)
                << " release=" << static_cast<int>(release_type);
            return GeneratedMessage{msg, oss.str()};
        },
        [](std::mt19937 &rng, uint8_t sysid, uint8_t compid) {
            std::uniform_int_distribution<int> platform_dist(0, 4);
            std::uniform_int_distribution<int> devices_dist(0, 6);
            std::uniform_int_distribution<int> hotspot_state_dist(0, 2);
            std::uniform_int_distribution<int> ethernet_state_dist(0, 2);
            std::uniform_int_distribution<int> freq_dist(2400, 5900);
            mavlink_message_t msg{};
            const uint8_t platform = static_cast<uint8_t>(platform_dist(rng));
            const uint8_t devices = static_cast<uint8_t>(devices_dist(rng));
            const uint8_t hotspot = static_cast<uint8_t>(hotspot_state_dist(rng));
            const uint16_t freq = static_cast<uint16_t>(freq_dist(rng));
            const uint8_t ethernet = static_cast<uint8_t>(ethernet_state_dist(rng));
            const uint8_t dummy0 = 0;
            const int32_t dummy1 = 0;
            const int32_t dummy2 = 0;
            mavlink_msg_openhd_sys_status1_pack(sysid, compid, &msg, platform, devices, hotspot, freq, ethernet, dummy0, dummy1,
                                                dummy2);
            std::ostringstream oss;
            oss << "openhd_sys_status1 platform=" << static_cast<int>(platform)
                << " devices=" << static_cast<int>(devices)
                << " wifi_state=" << static_cast<int>(hotspot)
                << " freq=" << freq
                << " ethernet=" << static_cast<int>(ethernet);
            return GeneratedMessage{msg, oss.str()};
        },
        [](std::mt19937 &rng, uint8_t sysid, uint8_t compid) {
            std::uniform_int_distribution<int> mode_dist(0, 2);
            std::bernoulli_distribution passive_dist(0.3);
            std::uniform_int_distribution<int> dummy0_dist(0, 1000);
            mavlink_message_t msg{};
            const uint8_t mode = static_cast<uint8_t>(mode_dist(rng));
            const int8_t passive = static_cast<int8_t>(passive_dist(rng) ? 1 : 0);
            const uint16_t dummy0 = static_cast<uint16_t>(dummy0_dist(rng));
            const int32_t dummy1 = 0;
            const int32_t dummy2 = 0;
            mavlink_msg_openhd_wifbroadcast_gnd_operating_mode_pack(sysid, compid, &msg, mode, passive, dummy0, dummy1, dummy2);
            std::ostringstream oss;
            oss << "openhd_wifbroadcast_gnd_operating_mode mode=" << static_cast<int>(mode)
                << " passive=" << static_cast<int>(passive)
                << " dummy0=" << dummy0;
            return GeneratedMessage{msg, oss.str()};
        }
    };

    std::uniform_int_distribution<std::size_t> selector(0, generators.size() - 1);
    return generators[selector(rng)](rng, sysid, compid);
}

TableRenderResult render_table(int start_row, int max_y, int max_x, const std::string &title,
                               const std::vector<const MessageEntry *> &entries,
                               bool interactive_mode, bool has_focus, std::size_t selected_index,
                               short highlight_pair, bool highlight_enabled) {
    TableRenderResult result;
    result.next_row = std::min(start_row, max_y);
    if (start_row >= max_y) {
        return result;
    }

    mvprintw(start_row++, 0, "%s", title.c_str());
    if (start_row >= max_y) {
        result.next_row = max_y;
        return result;
    }

    mvprintw(start_row++, 0, "%-5s %-5s %-6s %-20s %-8s %-4s %s", "SYS", "COMP", "MSG", "NAME",
             "COUNT", "LEN", "PAYLOAD");
    if (start_row >= max_y) {
        result.next_row = max_y;
        return result;
    }

    int available_rows = max_y - start_row - 1;
    if (available_rows < 0) {
        available_rows = 0;
    }

    std::size_t clamped_selected = selected_index;
    if (!entries.empty()) {
        if (clamped_selected >= entries.size()) {
            clamped_selected = entries.size() - 1;
        }
    } else {
        clamped_selected = 0;
    }

    std::size_t start_index = 0;
    if (interactive_mode && has_focus && available_rows > 0 && entries.size() > static_cast<std::size_t>(available_rows)) {
        if (clamped_selected >= static_cast<std::size_t>(available_rows)) {
            start_index = clamped_selected - static_cast<std::size_t>(available_rows) + 1;
        }
    }

    int displayed = 0;
    for (std::size_t i = start_index; i < entries.size(); ++i) {
        if (displayed >= available_rows) {
            break;
        }

        const auto *entry = entries[i];
        std::string payload = entry->payload_text;
        const int payload_start_col = 5 + 1 + 5 + 1 + 6 + 1 + 20 + 1 + 8 + 1 + 4 + 1;
        const int available_width = std::max(0, max_x - payload_start_col);
        if (static_cast<int>(payload.size()) > available_width && available_width > 3) {
            payload = payload.substr(0, available_width - 3) + "...";
        }

        const bool highlight_this = interactive_mode && has_focus && i == clamped_selected;
        if (highlight_this) {
            attron(A_BOLD);
            if (highlight_enabled) {
                attron(COLOR_PAIR(highlight_pair));
            }
            result.highlighted_entry = entry;
        }

        mvprintw(start_row + displayed, 0, "%-5u %-5u %-6u %-20s %-8lu %-4u %s",
                 static_cast<unsigned>(entry->sysid), static_cast<unsigned>(entry->compid),
                 static_cast<unsigned>(entry->msgid), entry->name.c_str(),
                 static_cast<unsigned long>(entry->count), static_cast<unsigned>(entry->len),
                 payload.c_str());

        if (highlight_this) {
            if (highlight_enabled) {
                attroff(COLOR_PAIR(highlight_pair));
            }
            attroff(A_BOLD);
        }

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

    result.next_row = start_row;
    return result;
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

ssize_t transport_read(Transport &transport, uint8_t *buffer, std::size_t length) {
    if (transport.mode == TransportMode::Serial) {
        return ::read(transport.fd, buffer, length);
    }

    sockaddr_in sender{};
    socklen_t sender_len = sizeof(sender);
    ssize_t received = ::recvfrom(transport.fd, buffer, length, 0, reinterpret_cast<sockaddr *>(&sender), &sender_len);
    if (received >= 0) {
        transport.udp_last_sender = sender;
    }
    return received;
}

bool transport_write(Transport &transport, const uint8_t *buffer, std::size_t length) {
    if (transport.mode == TransportMode::Serial) {
        ssize_t written = ::write(transport.fd, buffer, length);
        return written == static_cast<ssize_t>(length);
    }

    sockaddr_in destination{};
    if (transport.udp_last_sender.has_value()) {
        destination = *transport.udp_last_sender;
    } else if (transport.udp_has_default_destination) {
        destination = transport.udp_default_destination;
    } else {
        return false;
    }

    ssize_t sent = ::sendto(transport.fd, buffer, length, 0, reinterpret_cast<const sockaddr *>(&destination),
                            sizeof(destination));
    return sent == static_cast<ssize_t>(length);
}

bool write_message(Transport &transport, const mavlink_message_t &message) {
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    const auto length = mavlink_msg_to_send_buffer(buffer, &message);
    return transport_write(transport, buffer, length);
}

bool send_heartbeat(Transport &transport, uint8_t sysid, uint8_t compid, uint8_t target_sys, uint8_t target_comp) {
    (void)target_sys;
    (void)target_comp;
    mavlink_message_t msg{};
    mavlink_msg_heartbeat_pack(sysid, compid, &msg, MAV_TYPE_GENERIC, MAV_AUTOPILOT_INVALID,
                               MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 0, MAV_STATE_ACTIVE);
    return write_message(transport, msg);
}

bool send_reboot(Transport &transport, uint8_t sysid, uint8_t compid, uint8_t target_sys, uint8_t target_comp) {
    mavlink_message_t msg{};
    constexpr float kParam1RebootAutopilot = 1.0f;
    mavlink_msg_command_long_pack(sysid, compid, &msg, target_sys, target_comp,
                                  MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN, 0, kParam1RebootAutopilot,
                                  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    return write_message(transport, msg);
}

bool send_ping(Transport &transport, uint8_t sysid, uint8_t compid, uint8_t target_sys, uint8_t target_comp) {
    static uint32_t sequence = 0;
    mavlink_message_t msg{};
    const auto now = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch());
    mavlink_msg_ping_pack(sysid, compid, &msg, now.count(), target_sys, target_comp, sequence++);
    return write_message(transport, msg);
}

struct LoopStats {
    uint64_t sent = 0;
    uint64_t received = 0;
    uint64_t matched = 0;
    uint64_t lost = 0;
    uint64_t unexpected = 0;
    std::size_t inflight = 0;

    double loss_percentage() const {
        if (sent == 0) {
            return 0.0;
        }
        return (static_cast<double>(lost) / static_cast<double>(sent)) * 100.0;
    }
};

void render_ui(const std::string &connection_line, const std::string &connection_details,
               const std::map<MessageKey, MessageEntry> &messages, const std::string &output_path,
               const std::string &status_message, const FilterState &filter, bool transmit_mode,
               bool loop_mode, const LoopStats *loop_stats, const std::deque<std::string> &transmit_log,
               bool raw_mode, const std::deque<std::string> &raw_log, const NavigationState &nav_state,
               const std::vector<const MessageEntry *> &all_entries,
               const std::vector<const MessageEntry *> &filtered_entries) {
    erase();

    int max_y = 0;
    int max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    int row = 0;
    attron(A_BOLD);
    mvprintw(row++, 0, "OpenHD MAVLink Monitor");
    attroff(A_BOLD);

    if (row < max_y) {
        mvprintw(row++, 0, "%s", connection_line.c_str());
    }
    if (!connection_details.empty() && row < max_y) {
        mvprintw(row++, 0, "%s", connection_details.c_str());
    }
    if (row < max_y) {
        mvprintw(row++, 0, "Logging: %s", output_path.empty() ? "<disabled>" : output_path.c_str());
    }

    if (row < max_y) {
        mvhline(row++, 0, ACS_HLINE, max_x);
    }

    if (row < max_y) {
        attron(A_BOLD);
        mvprintw(row++, 0, "Controls");
        attroff(A_BOLD);
    }

    const std::vector<std::string> control_lines = {
        "q Quit   h Heartbeat   p Ping   r Reboot   t Send test/loop message",
        "s Cycle system   c Cycle component   m Cycle message   f Clear filters",
        "Arrow keys navigate lists   Tab/Shift+Tab switch focus   v Toggle view",
        raw_mode ? "Raw mode active: showing received bytes while idle"
                  : "Interactive view starts enabled (press 'v' to toggle)"};

    for (const auto &line : control_lines) {
        if (row >= max_y) {
            break;
        }
        mvprintw(row++, 0, "%s", line.c_str());
    }

    if (row < max_y) {
        mvhline(row++, 0, ACS_HLINE, max_x);
    }

    std::string mode_string;
    if (loop_mode) {
        mode_string += "Mode: Loop test";
        if (transmit_mode) {
            mode_string += " + random TX";
        }
    } else if (transmit_mode) {
        mode_string += "Mode: Random OpenHD TX";
    } else {
        mode_string += "Mode: Listen";
    }

    if (!mode_string.empty() && row < max_y) {
        mvprintw(row++, 0, "%s", mode_string.c_str());
    }

    if (row < max_y) {
        mvprintw(row++, 0, "Status: %s", status_message.c_str());
    }

    const std::string sys_str = filter.sysid ? std::to_string(*filter.sysid) : std::string("All");
    const std::string comp_str = filter.compid ? std::to_string(*filter.compid) : std::string("All");
    const std::string msg_str = filter.msgid ? std::to_string(*filter.msgid) : std::string("All");
    if (row < max_y) {
        mvprintw(row++, 0, "Active filter: sys=%s comp=%s msg=%s", sys_str.c_str(), comp_str.c_str(), msg_str.c_str());
    }

    if (row < max_y) {
        mvhline(row++, 0, ACS_HLINE, max_x);
    }

    if (loop_mode && loop_stats && row < max_y) {
        mvprintw(row++, 0,
                 "Loop stats: sent=%llu received=%llu matched=%llu lost=%llu unexpected=%llu inflight=%zu loss=%.2f%%",
                 static_cast<unsigned long long>(loop_stats->sent),
                 static_cast<unsigned long long>(loop_stats->received),
                 static_cast<unsigned long long>(loop_stats->matched),
                 static_cast<unsigned long long>(loop_stats->lost),
                 static_cast<unsigned long long>(loop_stats->unexpected), loop_stats->inflight,
                 loop_stats->loss_percentage());
        if (row < max_y) {
            ++row;
        }
    }

    if ((transmit_mode || loop_mode || !transmit_log.empty()) && row < max_y) {
        mvprintw(row++, 0, "Recent TX messages:");
        for (const auto &entry : transmit_log) {
            if (row >= max_y) {
                break;
            }
            mvprintw(row++, 0, "  %s", entry.c_str());
        }
        if (row < max_y) {
            ++row;
        }
    }

    if (raw_mode && !transmit_mode && row < max_y) {
        mvprintw(row++, 0, "Raw RX data:");
        if (raw_log.empty()) {
            if (row < max_y) {
                mvprintw(row++, 0, "  <no data>");
            }
        } else {
            const int available_width = std::max(0, max_x - 2);
            for (const auto &entry : raw_log) {
                if (row >= max_y) {
                    break;
                }
                std::string display_entry = entry;
                if (available_width > 3 && static_cast<int>(display_entry.size()) > available_width) {
                    display_entry = display_entry.substr(0, available_width - 3) + "...";
                }
                mvprintw(row++, 0, "  %s", display_entry.c_str());
            }
        }
        if (row < max_y) {
            ++row;
        }
    } else if (raw_mode && transmit_mode && row < max_y) {
        mvprintw(row++, 0, "Raw mode enabled (only active while listening)");
        if (row < max_y) {
            ++row;
        }
    }

    const bool all_focus = nav_state.interactive_mode && !nav_state.focus_filtered;
    const bool filtered_focus = nav_state.interactive_mode && nav_state.focus_filtered;

    std::string all_title = "All Messages";
    if (all_focus) {
        all_title += " [active]";
    }

    TableRenderResult all_result =
        render_table(row, max_y, max_x, all_title, all_entries, nav_state.interactive_mode, all_focus,
                     nav_state.selected_all_index, kHighlightColorPair, g_highlight_colors_available);
    row = all_result.next_row;

    if (row >= max_y) {
        refresh();
        return;
    }

    TableRenderResult filtered_result;
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
        std::string filtered_title = "Filtered Messages";
        if (filtered_focus) {
            filtered_title += " [active]";
        }
        filtered_result = render_table(row, max_y, max_x, filtered_title, filtered_entries,
                                       nav_state.interactive_mode, filtered_focus,
                                       nav_state.selected_filtered_index, kHighlightColorPair,
                                       g_highlight_colors_available);
        row = filtered_result.next_row;
    }

    if (nav_state.interactive_mode && row < max_y) {
        const MessageEntry *selected_entry = nullptr;
        if (filtered_focus && filtered_result.highlighted_entry) {
            selected_entry = filtered_result.highlighted_entry;
        } else if (all_result.highlighted_entry) {
            selected_entry = all_result.highlighted_entry;
        }

        if (selected_entry) {
            mvprintw(row++, 0,
                     "Selected message: sys=%u comp=%u msg=%u (%s) count=%lu len=%u",
                     static_cast<unsigned>(selected_entry->sysid),
                     static_cast<unsigned>(selected_entry->compid),
                     static_cast<unsigned>(selected_entry->msgid), selected_entry->name.c_str(),
                     static_cast<unsigned long>(selected_entry->count),
                     static_cast<unsigned>(selected_entry->len));
            if (row < max_y) {
                mvprintw(row++, 0, "Payload:");
            }
            if (row < max_y) {
                const int wrap_width = std::max(0, max_x - 2);
                auto wrapped_lines = wrap_text(selected_entry->payload_text, wrap_width);
                for (const auto &line : wrapped_lines) {
                    if (row >= max_y) {
                        break;
                    }
                    mvprintw(row++, 2, "%s", line.c_str());
                }
            }
        } else {
            mvprintw(row++, 0, "Interactive view active: no entry selected");
        }
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
    bool transmit_mode = false;
    bool loop_mode = false;
    bool raw_mode = false;
    bool use_udp = false;
    int udp_port = 5920;
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
        } else if (arg == "--transmit") {
            transmit_mode = true;
        } else if (arg == "--raw") {
            raw_mode = true;
        } else if (arg == "--loop") {
            loop_mode = true;
        } else if (arg == "--udp") {
            use_udp = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                udp_port = std::stoi(argv[++i]);
            }
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            std::cerr << "Additional options: --sysid <id> --compid <id> --target-sys <id> --target-comp <id>"
                      << " --transmit --loop --raw --udp [port]"
                      << std::endl;
            return 0;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (transmit_mode && loop_mode) {
        std::cerr << "--transmit and --loop cannot be used at the same time" << std::endl;
        return 1;
    }

    if (use_udp && (udp_port <= 0 || udp_port > 65535)) {
        std::cerr << "Invalid UDP port: " << udp_port << std::endl;
        return 1;
    }

    Transport transport;
    std::string connection_line;
    std::string connection_details;

    if (use_udp) {
        if (device_user_specified || baud_user_specified) {
            std::cerr << "Warning: --device/--baud ignored when using --udp" << std::endl;
        }

        transport.mode = TransportMode::UDP;
        transport.fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (transport.fd < 0) {
            perror("socket");
            return 1;
        }

        int reuse = 1;
        if (setsockopt(transport.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            perror("setsockopt");
        }

        sockaddr_in bind_addr{};
        bind_addr.sin_family = AF_INET;
        bind_addr.sin_port = htons(static_cast<uint16_t>(udp_port));
        if (inet_pton(AF_INET, "127.0.0.1", &bind_addr.sin_addr) != 1) {
            std::cerr << "Failed to parse localhost address" << std::endl;
            ::close(transport.fd);
            return 1;
        }

        if (bind(transport.fd, reinterpret_cast<sockaddr *>(&bind_addr), sizeof(bind_addr)) < 0) {
            perror("bind");
            ::close(transport.fd);
            return 1;
        }

        int flags = fcntl(transport.fd, F_GETFL, 0);
        if (flags >= 0) {
            (void)fcntl(transport.fd, F_SETFL, flags | O_NONBLOCK);
        }

        transport.udp_default_destination = bind_addr;
        transport.udp_has_default_destination = false;

        connection_line = "Link: UDP 127.0.0.1:" + std::to_string(udp_port) + " (listening)";
        connection_details = "Bind address: 127.0.0.1, replies once traffic is seen";
        std::cerr << "UDP mode: listening on 127.0.0.1:" << udp_port << std::endl;
    } else {
        if (device_path.empty()) {
            if (auto detected_device = find_default_serial_device()) {
                device_path = *detected_device;
                std::cerr << "No device specified, using " << device_path << std::endl;
            } else {
                std::cerr << "No device specified and unable to find a /dev/serialX or /dev/ttySX device" << std::endl;
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

        transport.mode = TransportMode::Serial;
        transport.fd = ::open(device_path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (transport.fd < 0) {
            perror("open");
            return 1;
        }

        if (!configure_serial(transport.fd, *speed_constant)) {
            ::close(transport.fd);
            return 1;
        }

        connection_line = "Link: Serial " + device_path + " @ " + std::to_string(baudrate) + " baud";
        if (!device_user_specified || !baud_user_specified) {
            std::string details = "Auto-detected";
            if (!device_user_specified && !baud_user_specified) {
                details += " device & baud";
            } else if (!device_user_specified) {
                details += " device";
            } else {
                details += " baud";
            }
            connection_details = details;
        } else {
            connection_details = "Manual serial configuration";
        }
    }

    std::ofstream output;
    if (!output_path.empty()) {
        output.open(output_path, std::ios::out | std::ios::app);
        if (!output.is_open()) {
            std::cerr << "Failed to open output file: " << output_path << std::endl;
            if (transport.fd >= 0) {
                ::close(transport.fd);
            }
            return 1;
        }
    }
    std::ofstream *output_ptr = output.is_open() ? &output : nullptr;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    initscr();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        use_default_colors();
        short blue_slot = COLOR_BLUE;
        if (can_change_color() && kOpenHDBlueColorSlot < COLORS) {
            init_color(kOpenHDBlueColorSlot, 0, 0, 1000);
            blue_slot = kOpenHDBlueColorSlot;
        }
        init_pair(kHighlightColorPair, COLOR_WHITE, blue_slot);
        g_highlight_colors_available = true;
    }

    mavlink_message_t message{};
    mavlink_status_t status{};
    std::map<MessageKey, MessageEntry> messages;
    std::string status_message;
    if (loop_mode) {
        status_message = "Loop mode active";
    } else if (transmit_mode) {
        status_message = "Transmit mode active";
    } else {
        status_message = use_udp ? "Listening on UDP" : "Listening";
    }

    bool appended_detail = false;
    auto append_detail = [&](const std::string &detail) {
        if (!appended_detail) {
            status_message += " (";
            appended_detail = true;
        } else {
            status_message += ", ";
        }
        status_message += detail;
    };

    if (use_udp) {
        append_detail("UDP 127.0.0.1:" + std::to_string(udp_port));
    } else if (!device_user_specified || !baud_user_specified) {
        std::string auto_detail = "auto:";
        bool need_sep = false;
        if (!device_user_specified) {
            auto_detail += " device=" + device_path;
            need_sep = true;
        }
        if (!baud_user_specified) {
            if (need_sep) {
                auto_detail += ',';
            }
            auto_detail += " baud=" + std::to_string(baudrate);
        }
        append_detail(auto_detail);
    }

    if (raw_mode) {
        if (loop_mode) {
            append_detail("raw display enabled");
        } else {
            append_detail(transmit_mode ? "raw display available in listen mode" : "raw display enabled");
        }
    }

    if (appended_detail) {
        status_message += ')';
    }

    FilterState filter;
    NavigationState nav_state;
    nav_state.interactive_mode = true;
    std::deque<std::string> transmit_log;
    std::deque<std::string> raw_log;
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> transmit_delay_dist(750, 1500);
    LoopStats loop_stats;
    std::map<uint32_t, std::chrono::steady_clock::time_point> loop_inflight;
    uint32_t loop_sequence = 0;

    const auto start_time = std::chrono::steady_clock::now();
    auto last_ui_update = start_time - std::chrono::milliseconds(200);
    auto next_transmit = start_time;
    if (transmit_mode) {
        next_transmit += std::chrono::milliseconds(transmit_delay_dist(rng));
    }
    const auto loop_interval = std::chrono::milliseconds(250);
    const auto loop_timeout = std::chrono::milliseconds(1000);
    auto next_loop_transmit = start_time;
    if (loop_mode) {
        next_loop_transmit += loop_interval;
    }

    auto transmit_random_message = [&](const std::string &origin) {
        GeneratedMessage generated = generate_random_openhd_message(rng, sysid, compid);
        const auto timestamp = current_timestamp_string();
        const auto name = message_name(generated.message);
        const auto payload_text = decode_payload(generated.message);
        const bool sent = write_message(transport, generated.message);
        record_message(generated.message, timestamp, name, payload_text, messages, output_ptr);

        std::ostringstream oss;
        oss << timestamp << ' ' << (sent ? "[sent] " : "[failed] ") << generated.description;
        if (!origin.empty()) {
            oss << " (" << origin << ')';
        }
        transmit_log.push_front(oss.str());
        if (transmit_log.size() > 10) {
            transmit_log.pop_back();
        }

        status_message = (sent ? "Sent " : "Failed to send ") + generated.description;
        if (!origin.empty()) {
            status_message += " [" + origin + ']';
        }
    };

    auto send_loop_ping = [&](const std::string &origin) {
        if (!loop_mode) {
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto now_us =
            std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        mavlink_message_t msg{};
        const uint32_t seq = loop_sequence++;
        mavlink_msg_ping_pack(sysid, compid, &msg, now_us, target_sys, target_comp, seq);
        const bool sent = write_message(transport, msg);

        const auto timestamp = current_timestamp_string();
        std::ostringstream oss;
        oss << timestamp << ' ' << (sent ? "[sent] " : "[failed] ") << "loop ping seq=" << seq;
        if (!origin.empty()) {
            oss << " (" << origin << ')';
        }
        transmit_log.push_front(oss.str());
        if (transmit_log.size() > 10) {
            transmit_log.pop_back();
        }

        if (sent) {
            loop_inflight[seq] = now;
            loop_stats.sent++;
            loop_stats.inflight = loop_inflight.size();
            status_message = "Sent loop ping seq=" + std::to_string(seq);
        } else {
            status_message = "Failed to send loop ping seq=" + std::to_string(seq);
        }
    };

    while (!g_should_exit) {
        uint8_t buffer[256];
        ssize_t nread = transport_read(transport, buffer, sizeof(buffer));
        if (nread < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                status_message = std::string("read error: ") + std::strerror(errno);
                break;
            }
        } else if (nread > 0) {
            if (raw_mode && !transmit_mode) {
                const auto timestamp = current_timestamp_string();
                std::string hex = bytes_to_hex(buffer, static_cast<std::size_t>(nread));
                constexpr std::size_t kMaxRawEntryLength = 256;
                if (hex.size() > kMaxRawEntryLength) {
                    hex = hex.substr(0, kMaxRawEntryLength - 3) + "...";
                }
                std::ostringstream oss;
                oss << timestamp << " [" << nread << " bytes] " << hex;
                raw_log.push_front(oss.str());
                constexpr std::size_t kMaxRawEntries = 50;
                if (raw_log.size() > kMaxRawEntries) {
                    raw_log.pop_back();
                }
            }
            for (ssize_t i = 0; i < nread; ++i) {
                if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &message, &status)) {
                    const auto timestamp = current_timestamp_string();
                    const auto name = message_name(message);
                    const auto payload_text = decode_payload(message);
                    record_message(message, timestamp, name, payload_text, messages, output_ptr);

                    if (loop_mode && message.msgid == MAVLINK_MSG_ID_PING) {
                        loop_stats.received++;
                        const uint32_t seq = mavlink_msg_ping_get_seq(&message);
                        auto it = loop_inflight.find(seq);
                        if (it != loop_inflight.end()) {
                            loop_stats.matched++;
                            loop_inflight.erase(it);
                            status_message = "Loop ping received seq=" + std::to_string(seq);
                        } else {
                            loop_stats.unexpected++;
                            std::ostringstream oss;
                            oss << current_timestamp_string() << " [unexpected] loop ping seq=" << seq;
                            transmit_log.push_front(oss.str());
                            if (transmit_log.size() > 10) {
                                transmit_log.pop_back();
                            }
                            status_message = "Unexpected loop ping seq=" + std::to_string(seq);
                        }
                        loop_stats.inflight = loop_inflight.size();
                    }
                }
            }
        }

        const auto now = std::chrono::steady_clock::now();
        if (transmit_mode && now >= next_transmit) {
            transmit_random_message("auto");
            next_transmit = now + std::chrono::milliseconds(transmit_delay_dist(rng));
        }

        if (loop_mode && now >= next_loop_transmit) {
            send_loop_ping("auto");
            next_loop_transmit = now + loop_interval;
        }

        if (loop_mode) {
            for (auto it = loop_inflight.begin(); it != loop_inflight.end();) {
                if (now - it->second > loop_timeout) {
                    const uint32_t seq = it->first;
                    loop_stats.lost++;
                    std::ostringstream oss;
                    oss << current_timestamp_string() << " [timeout] loop ping seq=" << seq;
                    transmit_log.push_front(oss.str());
                    if (transmit_log.size() > 10) {
                        transmit_log.pop_back();
                    }
                    status_message = "Loop ping timeout seq=" + std::to_string(seq);
                    it = loop_inflight.erase(it);
                } else {
                    ++it;
                }
            }
            loop_stats.inflight = loop_inflight.size();
        }

        auto all_entries = gather_sorted_entries(messages);
        auto filtered_entries = filter_entries(all_entries, filter);

        if (nav_state.selected_all_index >= all_entries.size()) {
            nav_state.selected_all_index = all_entries.empty() ? 0 : all_entries.size() - 1;
        }
        if (nav_state.selected_filtered_index >= filtered_entries.size()) {
            nav_state.selected_filtered_index = filtered_entries.empty() ? 0 : filtered_entries.size() - 1;
        }
        if (nav_state.focus_filtered && filtered_entries.empty()) {
            nav_state.focus_filtered = false;
        }

        if (now - last_ui_update > std::chrono::milliseconds(100)) {
            render_ui(connection_line, connection_details, messages, output_path, status_message, filter,
                      transmit_mode, loop_mode, loop_mode ? &loop_stats : nullptr, transmit_log, raw_mode,
                      raw_log, nav_state, all_entries, filtered_entries);
            last_ui_update = now;
        }

        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q' || ch == 'Q') {
                g_should_exit = 1;
            } else if (ch == 'h' || ch == 'H') {
                status_message = send_heartbeat(transport, sysid, compid, target_sys, target_comp)
                                     ? "Heartbeat sent"
                                     : "Failed to send heartbeat";
            } else if (ch == 'r' || ch == 'R') {
                status_message = send_reboot(transport, sysid, compid, target_sys, target_comp)
                                     ? "Reboot command sent"
                                     : "Failed to send reboot command";
            } else if (ch == 'p' || ch == 'P') {
                status_message = send_ping(transport, sysid, compid, target_sys, target_comp)
                                     ? "Ping sent"
                                     : "Failed to send ping";
            } else if (ch == 'v' || ch == 'V') {
                nav_state.interactive_mode = !nav_state.interactive_mode;
                if (!nav_state.interactive_mode) {
                    nav_state.focus_filtered = false;
                } else if (nav_state.focus_filtered && filtered_entries.empty()) {
                    nav_state.focus_filtered = false;
                }
                status_message = nav_state.interactive_mode
                                     ? "Interactive navigation enabled (use arrow keys)"
                                     : "Interactive navigation disabled";
                last_ui_update = now - std::chrono::milliseconds(200);
            } else if (ch == 't' || ch == 'T') {
                if (loop_mode) {
                    send_loop_ping("manual");
                    next_loop_transmit = std::chrono::steady_clock::now() + loop_interval;
                } else {
                    transmit_random_message("manual");
                    if (transmit_mode) {
                        next_transmit = std::chrono::steady_clock::now() +
                                        std::chrono::milliseconds(transmit_delay_dist(rng));
                    }
                }
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
            } else if (ch == KEY_UP) {
                if (nav_state.interactive_mode) {
                    auto &entries = nav_state.focus_filtered ? filtered_entries : all_entries;
                    auto &index = nav_state.focus_filtered ? nav_state.selected_filtered_index
                                                           : nav_state.selected_all_index;
                    if (!entries.empty() && index > 0) {
                        --index;
                        last_ui_update = now - std::chrono::milliseconds(200);
                    }
                }
            } else if (ch == KEY_DOWN) {
                if (nav_state.interactive_mode) {
                    auto &entries = nav_state.focus_filtered ? filtered_entries : all_entries;
                    auto &index = nav_state.focus_filtered ? nav_state.selected_filtered_index
                                                           : nav_state.selected_all_index;
                    if (!entries.empty() && index + 1 < entries.size()) {
                        ++index;
                        last_ui_update = now - std::chrono::milliseconds(200);
                    }
                }
            } else if (ch == KEY_LEFT || ch == KEY_BTAB) {
                if (nav_state.interactive_mode && nav_state.focus_filtered) {
                    nav_state.focus_filtered = false;
                    status_message = "Interactive focus: All Messages";
                    last_ui_update = now - std::chrono::milliseconds(200);
                }
            } else if (ch == KEY_RIGHT || ch == '\t') {
                if (nav_state.interactive_mode) {
                    if (filtered_entries.empty()) {
                        status_message = "No filtered messages available";
                    } else if (!nav_state.focus_filtered) {
                        nav_state.focus_filtered = true;
                        if (nav_state.selected_filtered_index >= filtered_entries.size()) {
                            nav_state.selected_filtered_index = filtered_entries.size() - 1;
                        }
                        status_message = "Interactive focus: Filtered Messages";
                    }
                    last_ui_update = now - std::chrono::milliseconds(200);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto final_all_entries = gather_sorted_entries(messages);
    auto final_filtered_entries = filter_entries(final_all_entries, filter);
    render_ui(connection_line, connection_details, messages, output_path, status_message, filter,
              transmit_mode, loop_mode, loop_mode ? &loop_stats : nullptr, transmit_log, raw_mode, raw_log,
              nav_state, final_all_entries, final_filtered_entries);
    endwin();

    if (output.is_open()) {
        output.flush();
    }

    if (transport.fd >= 0) {
        ::close(transport.fd);
    }
    return 0;
}
