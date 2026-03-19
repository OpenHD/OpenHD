#include "SbusOutput.h"

#include <algorithm>
#include <cerrno>
#include <cstring>

#include <asm/termbits.h>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
constexpr uint8_t SBUS_START_BYTE = 0x0F;
constexpr uint8_t SBUS_END_BYTE = 0x00;
constexpr int SBUS_FRAME_SIZE = 25;
constexpr int SBUS_BAUDRATE = 100000;
constexpr int SBUS_DEFAULT_VALUE = 1500;
constexpr int SBUS_STALE_TIMEOUT_MS = 500;

static std::string last_errno() { return std::string(strerror(errno)); }

bool configure_sbus_port(int fd) {
  struct termios2 tio {};
  if (ioctl(fd, TCGETS2, &tio) != 0) {
    return false;
  }

  tio.c_cflag &= ~CBAUD;
  tio.c_cflag |= BOTHER;
  tio.c_ispeed = SBUS_BAUDRATE;
  tio.c_ospeed = SBUS_BAUDRATE;

  tio.c_cflag &= ~CSIZE;
  tio.c_cflag |= CS8;
  tio.c_cflag |= PARENB;
  tio.c_cflag &= ~PARODD;
  tio.c_cflag |= CSTOPB;
  tio.c_cflag |= CREAD | CLOCAL;

  tio.c_iflag = IGNPAR;
  tio.c_oflag = 0;
  tio.c_lflag = 0;

  if (ioctl(fd, TCSETS2, &tio) != 0) {
    return false;
  }

  ioctl(fd, TCFLSH, TCIOFLUSH);
  return true;
}
}  // namespace

SbusOutput::SbusOutput() {
  m_console = openhd::log::create_or_get("sbus_out");
  m_channels.fill(SBUS_DEFAULT_VALUE);
}

SbusOutput::~SbusOutput() { stop(); }

void SbusOutput::configure(const Options& options) {
  const bool changed = (options.enabled != m_options.enabled) ||
                       (options.device != m_options.device) ||
                       (options.update_rate_hz != m_options.update_rate_hz);
  if (changed) {
    stop();
    m_options = options;
    if (m_options.enabled && !m_options.device.empty()) {
      start();
    }
  }
}

void SbusOutput::update_channels(const std::array<uint16_t, 18>& channels) {
  std::lock_guard<std::mutex> guard(m_mutex);
  for (size_t i = 0; i < m_channels.size(); ++i) {
    const uint16_t val = channels[i];
    if (val == 0 || val == UINT16_MAX) {
      continue;
    }
    m_channels[i] = val;
  }
  const auto now = std::chrono::steady_clock::now();
  const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch())
                          .count();
  m_last_update_ms.store(now_ms, std::memory_order_relaxed);
}

void SbusOutput::start() {
  if (m_running.exchange(true)) {
    return;
  }
  if (!open_port()) {
    m_running = false;
    return;
  }
  m_thread = std::thread([this]() { loop(); });
}

void SbusOutput::stop() {
  m_running = false;
  if (m_thread.joinable()) {
    m_thread.join();
  }
  close_port();
}

bool SbusOutput::open_port() {
  m_fd = open(m_options.device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (m_fd == -1) {
    m_console->warn("SBUS open failed for {}: {}", m_options.device,
                    last_errno());
    return false;
  }
  if (fcntl(m_fd, F_SETFL, 0) == -1) {
    m_console->warn("SBUS fcntl failed: {}", last_errno());
    close(m_fd);
    m_fd = -1;
    return false;
  }
  if (!configure_sbus_port(m_fd)) {
    m_console->warn("SBUS configure failed: {}", last_errno());
    close(m_fd);
    m_fd = -1;
    return false;
  }
  m_console->info("SBUS output enabled on {}", m_options.device);
  return true;
}

void SbusOutput::close_port() {
  if (m_fd >= 0) {
    close(m_fd);
    m_fd = -1;
  }
}

std::array<uint16_t, 18> SbusOutput::get_channels_copy() {
  std::lock_guard<std::mutex> guard(m_mutex);
  return m_channels;
}

uint16_t SbusOutput::map_pwm_to_sbus(uint16_t pwm) {
  if (pwm < 1000) pwm = 1000;
  if (pwm > 2000) pwm = 2000;
  const int sbus_min = 172;
  const int sbus_max = 1811;
  const int range = sbus_max - sbus_min;
  const int scaled = sbus_min + (static_cast<int>(pwm) - 1000) * range / 1000;
  return static_cast<uint16_t>(scaled);
}

std::array<uint8_t, SBUS_FRAME_SIZE> SbusOutput::build_frame(
    const std::array<uint16_t, 18>& channels, bool stale) {
  std::array<uint8_t, SBUS_FRAME_SIZE> frame{};
  frame[0] = SBUS_START_BYTE;

  uint16_t ch[16] = {};
  for (int i = 0; i < 16; ++i) {
    ch[i] = map_pwm_to_sbus(channels[i]);
  }

  frame[1] = static_cast<uint8_t>(ch[0] & 0xFF);
  frame[2] = static_cast<uint8_t>((ch[0] >> 8) | (ch[1] << 3));
  frame[3] = static_cast<uint8_t>((ch[1] >> 5) | (ch[2] << 6));
  frame[4] = static_cast<uint8_t>(ch[2] >> 2);
  frame[5] = static_cast<uint8_t>((ch[2] >> 10) | (ch[3] << 1));
  frame[6] = static_cast<uint8_t>((ch[3] >> 7) | (ch[4] << 4));
  frame[7] = static_cast<uint8_t>((ch[4] >> 4) | (ch[5] << 7));
  frame[8] = static_cast<uint8_t>(ch[5] >> 1);
  frame[9] = static_cast<uint8_t>((ch[5] >> 9) | (ch[6] << 2));
  frame[10] = static_cast<uint8_t>((ch[6] >> 6) | (ch[7] << 5));
  frame[11] = static_cast<uint8_t>(ch[7] >> 3);

  frame[12] = static_cast<uint8_t>(ch[8] & 0xFF);
  frame[13] = static_cast<uint8_t>((ch[8] >> 8) | (ch[9] << 3));
  frame[14] = static_cast<uint8_t>((ch[9] >> 5) | (ch[10] << 6));
  frame[15] = static_cast<uint8_t>(ch[10] >> 2);
  frame[16] = static_cast<uint8_t>((ch[10] >> 10) | (ch[11] << 1));
  frame[17] = static_cast<uint8_t>((ch[11] >> 7) | (ch[12] << 4));
  frame[18] = static_cast<uint8_t>((ch[12] >> 4) | (ch[13] << 7));
  frame[19] = static_cast<uint8_t>(ch[13] >> 1);
  frame[20] = static_cast<uint8_t>((ch[13] >> 9) | (ch[14] << 2));
  frame[21] = static_cast<uint8_t>((ch[14] >> 6) | (ch[15] << 5));
  frame[22] = static_cast<uint8_t>(ch[15] >> 3);

  uint8_t flags = 0;
  if (channels[16] > 1024) flags |= 1 << 0;
  if (channels[17] > 1024) flags |= 1 << 1;
  if (stale) {
    flags |= 1 << 2;
    flags |= 1 << 3;
  }
  frame[23] = flags;
  frame[24] = SBUS_END_BYTE;
  return frame;
}

void SbusOutput::loop() {
  const int rate_hz =
      std::max(1, std::min(200, m_options.update_rate_hz));
  const auto period = std::chrono::microseconds(1000000 / rate_hz);
  while (m_running) {
    const auto now = std::chrono::steady_clock::now();
    const auto channels = get_channels_copy();
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch())
                            .count();
    const int64_t last_ms =
        m_last_update_ms.load(std::memory_order_relaxed);
    const bool stale =
        last_ms == 0 ||
        (now_ms - last_ms) > SBUS_STALE_TIMEOUT_MS;
    const auto frame = build_frame(channels, stale);
    if (m_fd >= 0) {
      const ssize_t written =
          write(m_fd, frame.data(), static_cast<size_t>(frame.size()));
      if (written != static_cast<ssize_t>(frame.size())) {
        m_console->warn("SBUS write failed ({} of {}): {}", written,
                        frame.size(), last_errno());
      }
    }
    std::this_thread::sleep_for(period);
  }
}
