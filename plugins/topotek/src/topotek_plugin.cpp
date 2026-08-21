#include "openhd_plugin.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>

#include "topotek_protocol.h"

namespace {

constexpr const char* kPluginName = "topotek";
constexpr uint16_t kDefaultCameraPort = 9003;
constexpr uint16_t kDefaultClientPort = 9004;
constexpr int32_t kExternalIpCameraType = 3;

openhd_plugin_host g_host{};
bool g_has_host = false;
int g_socket = -1;
std::string g_camera_ip;
std::string g_last_camera_ip;
uint16_t g_camera_port = kDefaultCameraPort;
uint16_t g_client_port = kDefaultClientPort;
uint8_t g_angle_speed = 50;
int g_last_bitrate_code = -1;
std::mutex g_send_mutex;

void log_message(int32_t level, const std::string& message) {
  if (g_has_host && g_host.log) {
    g_host.log(level, kPluginName, message.c_str());
  }
}

uint16_t configured_port(const char* variable, uint16_t fallback) {
  const char* value = std::getenv(variable);
  if (!value || value[0] == '\0') return fallback;
  char* end = nullptr;
  errno = 0;
  const long parsed = std::strtol(value, &end, 10);
  if (errno != 0 || end == value || *end != '\0' || parsed < 1 ||
      parsed > std::numeric_limits<uint16_t>::max()) {
    return 0;
  }
  return static_cast<uint16_t>(parsed);
}

uint8_t configured_angle_speed() {
  const char* value = std::getenv("OPENHD_TOPOTEK_ANGLE_SPEED");
  if (!value || value[0] == '\0') return 50;
  char* end = nullptr;
  errno = 0;
  const long parsed = std::strtol(value, &end, 10);
  if (errno != 0 || end == value || *end != '\0' || parsed < 1 ||
      parsed > 99) {
    return 0;
  }
  return static_cast<uint8_t>(parsed);
}

bool send_command_locked(const std::string& command) {
  const std::string& destination_ip =
      g_camera_ip.empty() ? g_last_camera_ip : g_camera_ip;
  if (destination_ip.empty()) {
    log_message(OPENHD_PLUGIN_LOG_WARN,
                "Cannot control Topotek camera before IP_CAM_ADDRESS is known");
    return false;
  }

  sockaddr_in destination{};
  destination.sin_family = AF_INET;
  destination.sin_port = htons(g_camera_port);
  if (inet_pton(AF_INET, destination_ip.c_str(), &destination.sin_addr) != 1) {
    log_message(OPENHD_PLUGIN_LOG_ERROR,
                "Topotek destination is not a valid IPv4 address");
    return false;
  }
  const auto sent = sendto(g_socket, command.data(), command.size(), MSG_DONTWAIT,
                           reinterpret_cast<const sockaddr*>(&destination),
                           sizeof(destination));
  if (sent != static_cast<ssize_t>(command.size())) {
    log_message(OPENHD_PLUGIN_LOG_ERROR,
                std::string("Failed to send Topotek command: ") +
                    std::strerror(errno));
    return false;
  }
  return true;
}

bool send_command(const std::string& command) {
  std::lock_guard<std::mutex> lock(g_send_mutex);
  return g_socket >= 0 && send_command_locked(command);
}

int32_t plugin_init(const openhd_plugin_host* host,
                    const openhd_plugin_context* context) {
  if (!host || host->struct_size < sizeof(*host) ||
      host->abi_version != OPENHD_PLUGIN_ABI_VERSION || !context ||
      context->struct_size < sizeof(*context) ||
      context->role != OPENHD_PLUGIN_ROLE_AIR) {
    return -1;
  }
  g_host = *host;
  g_has_host = true;
  g_last_bitrate_code = -1;
  const char* configured_ip = std::getenv("OPENHD_TOPOTEK_IP");
  g_camera_ip = configured_ip && configured_ip[0] != '\0' ? configured_ip : "";
  if (!g_camera_ip.empty()) {
    in_addr address{};
    if (inet_pton(AF_INET, g_camera_ip.c_str(), &address) != 1) {
      log_message(OPENHD_PLUGIN_LOG_ERROR, "Invalid OPENHD_TOPOTEK_IP");
      return -2;
    }
  }

  g_camera_port = configured_port("OPENHD_TOPOTEK_PORT", kDefaultCameraPort);
  g_client_port =
      configured_port("OPENHD_TOPOTEK_CLIENT_PORT", kDefaultClientPort);
  g_angle_speed = configured_angle_speed();
  if (g_camera_port == 0 || g_client_port == 0 || g_angle_speed == 0) {
    log_message(OPENHD_PLUGIN_LOG_ERROR,
                "Invalid Topotek port or angle-speed environment setting");
    return -3;
  }

  g_socket = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if (g_socket < 0) {
    log_message(OPENHD_PLUGIN_LOG_ERROR,
                std::string("Cannot create UDP socket: ") +
                    std::strerror(errno));
    return -4;
  }
  const int reuse = 1;
  setsockopt(g_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  sockaddr_in local{};
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = htonl(INADDR_ANY);
  local.sin_port = htons(g_client_port);
  if (bind(g_socket, reinterpret_cast<const sockaddr*>(&local), sizeof(local)) <
      0) {
    log_message(OPENHD_PLUGIN_LOG_ERROR,
                std::string("Cannot bind Topotek reply port: ") +
                    std::strerror(errno));
    close(g_socket);
    g_socket = -1;
    return -5;
  }

  std::ostringstream message;
  message << "Ready for Topotek camera at "
          << (g_camera_ip.empty() ? "the MAVLink IP_CAM_ADDRESS" : g_camera_ip)
          << ':' << g_camera_port << " (reply port " << g_client_port << ')';
  log_message(OPENHD_PLUGIN_LOG_INFO, message.str());
  return 0;
}

void plugin_shutdown() {
  std::lock_guard<std::mutex> lock(g_send_mutex);
  if (g_socket >= 0) close(g_socket);
  g_socket = -1;
  g_camera_ip.clear();
  g_last_camera_ip.clear();
  g_last_bitrate_code = -1;
  g_host = {};
  g_has_host = false;
}

int resolution_code(uint16_t width, uint16_t height) {
  if (width == 3840 && height == 2160) return 0;
  if (width == 1920 && height == 1080) return 1;
  if (width == 1280 && height == 720) return 2;
  if (width == 640 && height == 480) return 3;
  return -1;
}

int bitrate_code(int32_t bitrate_kbits) {
  if (bitrate_kbits < 512) return -1;
  const auto rounded =
      (static_cast<int64_t>(bitrate_kbits) + 512) / 1024 - 1;
  return static_cast<int>(std::clamp<int64_t>(rounded, 0, 7));
}

bool update_bitrate(int32_t bitrate_kbits, const char* ip_address) {
  const int bitrate = bitrate_code(bitrate_kbits);
  if (bitrate < 0) {
    log_message(OPENHD_PLUGIN_LOG_WARN,
                "Topotek bitrate must be at least 512 kbit/s");
    return false;
  }
  std::lock_guard<std::mutex> lock(g_send_mutex);
  if (ip_address && ip_address[0] != '\0') g_last_camera_ip = ip_address;
  if (bitrate == g_last_bitrate_code) return true;
  const char data[3] = {'0', static_cast<char>('0' + bitrate), '\0'};
  if (g_socket < 0 ||
      !send_command_locked(topotek::make_command('D', "BIT", data))) {
    return false;
  }
  g_last_bitrate_code = bitrate;
  return true;
}

void on_video_settings_changed(
    const openhd_plugin_video_settings_event* event) {
  if (!event || event->struct_size < sizeof(*event) || g_socket < 0 ||
      event->camera_type != kExternalIpCameraType || !event->ip_address ||
      event->ip_address[0] == '\0') {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(g_send_mutex);
    g_last_camera_ip = event->ip_address;
  }

  const int resolution = resolution_code(event->width, event->height);
  const int bitrate = bitrate_code(event->bitrate_kbits);
  if (resolution < 0) {
    log_message(OPENHD_PLUGIN_LOG_WARN,
                "Topotek supports RTSP resolutions 3840x2160, 1920x1080, "
                "1280x720, and 640x480");
  } else {
    send_command(topotek::make_command(
        'D', "VID", "2" + std::to_string(resolution)));
  }
  update_bitrate(event->bitrate_kbits, event->ip_address);

  std::ostringstream message;
  message << "Configured Topotek RTSP stream for camera" << event->camera_index;
  if (bitrate >= 0) {
    message << " at " << (bitrate + 1) * 1024 << " kbit/s";
  }
  if (event->codec != OPENHD_PLUGIN_VIDEO_H264) {
    message << "; codec selection is not available in the Topotek SIP protocol";
  }
  if (event->framerate > 0) {
    message << "; frame-rate selection is not available in the protocol";
  }
  log_message(OPENHD_PLUGIN_LOG_INFO, message.str());
}

void on_video_bitrate_changed(
    const openhd_plugin_video_bitrate_event* event) {
  if (!event || event->struct_size < sizeof(*event) ||
      event->camera_type != kExternalIpCameraType || !event->ip_address ||
      event->ip_address[0] == '\0') {
    return;
  }
  if (update_bitrate(event->bitrate_kbits, event->ip_address)) {
    std::ostringstream message;
    message << "Set Topotek runtime bitrate for camera" << event->camera_index
            << " to " << (bitrate_code(event->bitrate_kbits) + 1) * 1024
            << " kbit/s";
    log_message(OPENHD_PLUGIN_LOG_DEBUG, message.str());
  }
}

int32_t on_camera_control(const openhd_plugin_camera_control_event* event) {
  if (!event || event->struct_size < sizeof(*event) || g_socket < 0) return -1;

  std::string command;
  std::string description;
  switch (event->action) {
    case OPENHD_PLUGIN_GIMBAL_RATE: {
      const auto pitch = static_cast<int8_t>(std::lround(
          std::clamp(event->value1, -1.0F, 1.0F) * 99.0F));
      const auto yaw = static_cast<int8_t>(std::lround(
          std::clamp(event->value2, -1.0F, 1.0F) * 99.0F));
      command = topotek::make_gimbal_rate_command(pitch, yaw);
      description = "gimbal rate";
      break;
    }
    case OPENHD_PLUGIN_GIMBAL_ANGLE:
      command = topotek::make_gimbal_angle_command(
          event->value1, event->value2, g_angle_speed);
      description = "gimbal angle";
      break;
    case OPENHD_PLUGIN_GIMBAL_CENTER:
      command = topotek::make_command('G', "PTZ", "05");
      description = "gimbal center";
      break;
    case OPENHD_PLUGIN_GIMBAL_CALIBRATE:
      command = topotek::make_command('G', "PTZ", "09");
      description = "gimbal calibration";
      break;
    case OPENHD_PLUGIN_GIMBAL_ROLL_RATE: {
      const auto roll = static_cast<int8_t>(std::lround(
          std::clamp(event->value1, -1.0F, 1.0F) * 99.0F));
      char data[3]{};
      std::snprintf(data, sizeof(data), "%02X",
                    static_cast<unsigned int>(static_cast<uint8_t>(roll)));
      command = topotek::make_command('G', "GSR", data);
      description = "gimbal roll rate";
      break;
    }
    case OPENHD_PLUGIN_GIMBAL_MODE: {
      const int mode = static_cast<int>(std::lround(event->value1));
      if (mode == OPENHD_PLUGIN_GIMBAL_MODE_LOCK) {
        command = topotek::make_command('G', "PTZ", "07");
      } else if (mode == OPENHD_PLUGIN_GIMBAL_MODE_FOLLOW) {
        command = topotek::make_command('G', "PTZ", "06");
      } else {
        return 1;
      }
      description = "gimbal mode";
      break;
    }
    case OPENHD_PLUGIN_CAMERA_ZOOM_RATE: {
      const std::string value =
          event->value1 > 0 ? "02" : event->value1 < 0 ? "01" : "00";
      command = topotek::make_command('M', "ZMC", value);
      description = "zoom rate";
      break;
    }
    case OPENHD_PLUGIN_CAMERA_FOCUS_RATE: {
      const std::string value =
          event->value1 > 0 ? "01" : event->value1 < 0 ? "02" : "00";
      command = topotek::make_command('M', "FCC", value);
      description = "focus rate";
      break;
    }
    case OPENHD_PLUGIN_CAMERA_AUTO_FOCUS:
      command = topotek::make_command('M', "FCC", "10");
      description = "autofocus";
      break;
    case OPENHD_PLUGIN_CAMERA_TAKE_PHOTO:
      command = topotek::make_command('D', "CAP", "01");
      description = "take photo";
      break;
    case OPENHD_PLUGIN_CAMERA_RECORD_START:
      command = topotek::make_command('D', "REC", "11");
      description = "start recording";
      break;
    case OPENHD_PLUGIN_CAMERA_RECORD_STOP:
      command = topotek::make_command('D', "REC", "00");
      description = "stop recording";
      break;
    case OPENHD_PLUGIN_CAMERA_IMAGE_TYPE: {
      const int value = static_cast<int>(std::lround(event->value1));
      if (value < 0 || value > 3) return 1;
      // OpenHD uses four consecutive semantic modes. Topotek encodes them as
      // the non-consecutive protocol bytes 00, 01, 10 and 11.
      constexpr int kTopotekImageTypes[] = {0x00, 0x01, 0x10, 0x11};
      char data[3]{};
      std::snprintf(data, sizeof(data), "%02X", kTopotekImageTypes[value]);
      command = topotek::make_command('D', "PIP", data);
      description = "image type";
      break;
    }
    case OPENHD_PLUGIN_CAMERA_THERMAL_PALETTE: {
      const int value = static_cast<int>(std::lround(event->value1));
      if (value < 0 || value > 11) return 1;
      char data[3]{};
      std::snprintf(data, sizeof(data), "%02X", value);
      command = topotek::make_command('E', "IMG", data);
      description = "thermal palette";
      break;
    }
    case OPENHD_PLUGIN_CAMERA_ZOOM_ABSOLUTE:
    case OPENHD_PLUGIN_CAMERA_ZOOM_PERCENT:
    default:
      return 1;
  }

  if (!send_command(command)) return -1;
  log_message(OPENHD_PLUGIN_LOG_DEBUG,
              "Sent Topotek " + description + " command");
  return 0;
}

const openhd_plugin_descriptor kDescriptor{
    sizeof(openhd_plugin_descriptor),
    OPENHD_PLUGIN_ABI_VERSION,
    OPENHD_PLUGIN_ROLE_AIR,
    kPluginName,
    "0.1.0",
    plugin_init,
    plugin_shutdown,
    on_video_bitrate_changed,
    on_video_settings_changed,
    on_camera_control};

}  // namespace

extern "C" __attribute__((visibility("default")))
const openhd_plugin_descriptor* openhd_plugin_get_descriptor() {
  return &kDescriptor;
}
