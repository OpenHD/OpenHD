/******************************************************************************
 * OpenHD
 *
 * Licensed under the GNU General Public License (GPL) Version 3.
 *
 * This software is provided "as-is," without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose, and non-infringement. For details, see the
 * full license in the LICENSE file provided with this source code.
 *
 * Non-Military Use Only:
 * This software and its associated components are explicitly intended for
 * civilian and non-military purposes. Use in any military or defense
 * applications is strictly prohibited unless explicitly and individually
 * licensed otherwise by the OpenHD Team.
 *
 * Contributors:
 * A full list of contributors can be found at the OpenHD GitHub repository:
 * https://github.com/OpenHD
 *
 * © OpenHD, All Rights Reserved.
 ******************************************************************************/

#include "SerialEndpoint.h"

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include <cassert>
#include <map>
#include <utility>

#include "openhd_platform.h"
#include "openhd_sock.h"
#include "openhd_spdlog_include.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "openhd_link_usage.hpp"

static std::string GET_ERROR() { return {strerror(errno)}; }
static void debug_poll_fd(const struct pollfd& poll_fd) {
  std::stringstream ss;
  ss << "Poll:{";
  if (poll_fd.events & POLLERR) {
    ss << "POLLERR,";
  }
  if (poll_fd.events & POLLHUP) {
    ss << "POLLHUP,";
  }
  if (poll_fd.events & POLLNVAL) {
    ss << "POLLNVAL,";
  }
  ss << "}\n";
  std::cout << ss.str();
}

// https://stackoverflow.com/questions/12340695/how-to-check-if-a-given-file-descriptor-stored-in-a-variable-is-still-valid
static bool is_serial_fd_still_connected(const int fd) {
  struct stat s {};
  fstat(fd, &s);
  // struct stat::nlink_t   st_nlink;   ... number of hard links
  if (s.st_nlink < 1) {
    // treat device disconnected case
    return false;
  }
  return true;
}

SerialEndpoint::SerialEndpoint(std::string TAG1,
                               SerialEndpoint::HWOptions options1)
    : MEndpoint(std::move(TAG1)), m_options(std::move(options1)) {
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  // m_limited_rate_logger=std::make_unique<openhd::log::LimitedRateLogger>(m_console,std::chrono::milliseconds(1000));
  m_console->info("created with {}", m_options.to_string());
  if (TAG == "openhd_uart") {
    // UART 8N1 uses ten line bits per payload byte.
    openhd::link_usage::Registry::instance().add_link(
        "UART", static_cast<uint32_t>(m_options.baud_rate * 8 / 10));
  }
  start();
}

SerialEndpoint::~SerialEndpoint() {
  stop();
  if (TAG == "openhd_uart")
    openhd::link_usage::Registry::instance().remove_link("UART");
}

bool SerialEndpoint::sendMessagesImpl(
    const std::vector<MavlinkMessage>& messages) {
  bool accepted = true;
  {
    std::lock_guard<std::mutex> lock(m_tx_mutex);
    if (_stop_requested || m_fd < 0 || m_tx_link_failed) {
      m_tx_disconnected += messages.size();
      return false;
    }
    for (const auto& message : messages) {
      if (!m_tx_queue.push(message, SerialTxQueue::Clock::now(), m_connection_generation.load())) accepted = false;
    }
  }
  m_tx_cv.notify_one();
  return accepted;  // accepted by the queue, not proof of physical transmission
}

void SerialEndpoint::write_loop() {
  auto last_log = std::chrono::steady_clock::now();
  while (!_stop_requested) {
    std::optional<SerialTxQueue::Item> item;
    {
      std::unique_lock<std::mutex> lock(m_tx_mutex);
      m_tx_cv.wait_for(lock, std::chrono::milliseconds(100), [this] {
        return _stop_requested || m_tx_queue.size() != 0;
      });
      if (_stop_requested) break;
      item = m_tx_queue.pop();
    }
    if (item) {
      if (write_data_serial(item->message.pack(), item->generation)) ++m_tx_written;
      else ++m_tx_write_failed;
    }
    const auto now = std::chrono::steady_clock::now();
    if (now - last_log >= std::chrono::seconds(5)) {
      size_t queued, high_water;
      uint64_t dropped, expired, replaced, disconnected;
      {
        std::lock_guard<std::mutex> lock(m_tx_mutex);
        queued = m_tx_queue.size(); high_water = m_tx_queue.high_water;
        dropped = m_tx_queue.dropped; expired = m_tx_queue.expired;
        replaced = m_tx_queue.replaced; disconnected = m_tx_disconnected;
      }
      m_console->info("serial-tx-v1 fd={} queued={} high_water={} written={} "
                      "write_failed={} dropped={} expired={} rc_replaced={} disconnected={}",
                      m_fd.load(), queued, high_water, m_tx_written,
                      m_tx_write_failed, dropped, expired, replaced, disconnected);
      last_log = now;
    }
  }
}

bool SerialEndpoint::write_data_serial(const std::vector<uint8_t>& data, uint64_t generation) {
  std::lock_guard<std::mutex> fd_lock(m_fd_mutex);
  const int fd = m_fd.load();
  if (fd < 0 || m_tx_link_failed || _stop_requested ||
      generation != m_connection_generation.load()) return false;
  size_t offset = 0;
  // One nonblocking writer completes each frame before taking another. A stalled
  // port must not prevent shutdown, nor splice the next frame onto a partial one.
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
  while (offset < data.size() && !_stop_requested) {
    const auto count = write(fd, data.data() + offset, data.size() - offset);
    if (count > 0) {
      offset += static_cast<size_t>(count);
      if (TAG == "openhd_uart")
        openhd::link_usage::Registry::instance().record(
            "UART", openhd::link_usage::Category::Telemetry, count);
      continue;
    }
    if (count < 0 && errno == EINTR) continue;
    if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) break;
    if (std::chrono::steady_clock::now() >= deadline) break;
    struct pollfd pending{fd, POLLOUT, 0};
    const int result = poll(&pending, 1, 20);
    if (result < 0 && errno == EINTR) continue;
    if (result < 0 || (pending.revents & (POLLERR | POLLHUP | POLLNVAL))) break;
  }
  if (offset == data.size()) return true;
  m_tx_link_failed = true;
  // Never replay queued commands on a reconnected/replaced device.
  {
    std::lock_guard<std::mutex> lock(m_tx_mutex);
    m_tx_queue.clear();
  }
  const auto now = std::chrono::steady_clock::now();
  if (!_stop_requested && now - m_last_log_serial_write_failed >=
                          MIN_DELAY_BETWEEN_SERIAL_WRITE_FAILED_LOG_MESSAGES) {
    m_console->warn("serial-tx-v1 incomplete frame bytes={}/{}; reconnecting", offset, data.size());
    m_last_log_serial_write_failed = now;
  }
  return false;
}

int SerialEndpoint::define_from_baudrate(int baudrate) {
  switch (baudrate) {
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    case 230400:
      return B230400;
    case 460800:
      return B460800;
    case 500000:
      return B500000;
    case 576000:
      return B576000;
    case 921600:
      return B921600;
    case 1000000:
      return B1000000;
    case 1152000:
      return B1152000;
    case 1500000:
      return B1500000;
    case 2000000:
      return B2000000;
    case 2500000:
      return B2500000;
    case 3000000:
      return B3000000;
    case 3500000:
      return B3500000;
    case 4000000:
      return B4000000;
    default: {
      openhd::log::get_default()->warn("Unknown baudrate");
      return B115200;
    }
  }
}

int SerialEndpoint::setup_port(const SerialEndpoint::HWOptions& options,
                               std::shared_ptr<spdlog::logger> m_console) {
  if (!m_console) {
    m_console = openhd::log::get_default();
  }
  // Also see
  // https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

  // open() hangs on macOS or Linux devices(e.g. pocket beagle) unless you give
  // it O_NONBLOCK
  int fd = open(options.linux_filename.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd == -1) {
    m_console->warn("open failed: {}", GET_ERROR());
    return -1;
  }
  // Keep O_NONBLOCK: read readiness is polled and the dedicated writer
  // handles backpressure and short writes without blocking telemetry callers.
  // From
  // https://github.com/mavlink/c_uart_interface_example/blob/master/serial_port.cpp
  if (!isatty(fd)) {
    m_console->warn("file descriptor {} is NOT a serial port",
                    options.linux_filename);
    close(fd);
    return -1;
  }
  struct termios tc {};
  bzero(&tc, sizeof(tc));
  if (tcgetattr(fd, &tc) != 0) {
    m_console->warn("tcgetattr failed: {}", GET_ERROR());
    close(fd);
    return -1;
  }
  tc.c_iflag &=
      ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
  tc.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OPOST);
  tc.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG | TOSTOP);
  tc.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
  tc.c_cflag |= CS8;
  tc.c_cc[VMIN] = 0;    // We are ok with 0 bytes.
  tc.c_cc[VTIME] = 10;  // Timeout after  1s (10 deciseconds)
  if (options.flow_control) {
    tc.c_cflag |= CRTSCTS;
  }
  tc.c_cflag |= CLOCAL;  // Without this a write() blocks indefinitely.
  //
  const int baudrate_or_define = define_from_baudrate(options.baud_rate);
  if (cfsetispeed(&tc, baudrate_or_define) != 0) {
    m_console->warn("cfsetispeed failed: {}", GET_ERROR());
    close(fd);
    return -1;
  }
  if (cfsetospeed(&tc, baudrate_or_define) != 0) {
    m_console->warn("cfsetospeed failed: {}", GET_ERROR());
    close(fd);
    return -1;
  }
  if (tcsetattr(fd, TCSANOW, &tc) != 0) {
    m_console->warn("tcsetattr failed: {}", GET_ERROR());
    close(fd);
    return -1;
  }
  return fd;
}

void SerialEndpoint::connect_and_read_loop() {
  while (!_stop_requested) {
    if (!OHDFilesystemUtil::exists(m_options.linux_filename)) {
      if (!uart_log_warning_once) {
        m_console->debug("UART not found!");
        uart_log_warning_once = true;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    // The file exists, so creating the FD should be no problem
    const int fd = setup_port(m_options, m_console);
    {
      std::lock_guard<std::mutex> lock(m_fd_mutex);
      ++m_connection_generation;
      m_fd = fd;
      m_tx_link_failed = false;
    }
    if (m_fd == -1) {
      // But if it fails, we start over again, checking if at least the linux fd
      // exists
      m_console->warn("Cannot create uart fd " + m_options.to_string());
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    m_console->debug("Successfully created UART fd for: {}",
                     m_options.to_string());
    reset_parser();
    receive_data_until_error();
    // cleanup and start over again
    {
      std::lock_guard<std::mutex> lock(m_fd_mutex);
      m_tx_link_failed = true;
      ++m_connection_generation;
      m_fd = -1;
      close(fd);
      std::lock_guard<std::mutex> queue_lock(m_tx_mutex);
      m_tx_queue.clear();
    }
  }
}

void SerialEndpoint::receive_data_until_error() {
  m_console->debug("receive_data_until_error() begin");
  // Enough for MTU 1500 bytes.
  uint8_t buffer[2048];

  struct pollfd fds[1];
  fds[0].fd = m_fd;
  fds[0].events = POLLIN;
  m_n_failed_reads = 0;
  bool warned_once = false;
  auto last_received = std::chrono::steady_clock::now();

  while (!_stop_requested) {
    if (m_tx_link_failed) return;
    const int pollrc = poll(fds, 1, 100);
    const auto valid = is_serial_fd_still_connected(m_fd);
    if (!valid) {
      m_console->debug("Exiting serial, not connected");
      return;
    }
    if (pollrc < 0) {
      if (errno == EINTR) continue;
      m_console->warn("read poll failure: {}", GET_ERROR());
      return;
    }
    if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) return;
    if (pollrc == 0 || !(fds[0].revents & POLLIN)) {
      m_n_failed_reads++;
      if (!warned_once && m_options.enable_reading &&
          std::chrono::steady_clock::now() - last_received >= std::chrono::seconds(1)) {
        m_console->warn("{} failed reads - FC connected ?", m_n_failed_reads);
        if (TAG == "fc_ser") {
          openhd::Reporter::instance().report_status(
              "no_air_telemetry",
              "No telemetry received from flight controller over UART", 10000);
        }
        warned_once = true;
      }
      continue;
    }
    const int recv_len = static_cast<int>(read(m_fd, buffer, sizeof(buffer)));
    if (recv_len > 0) {
      last_received = std::chrono::steady_clock::now();
      MEndpoint::parseNewData(buffer, recv_len);
      warned_once = false;
    } else if (recv_len < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
      m_console->warn("read failure: {} {}", recv_len, GET_ERROR());
    }
  }
  m_console->debug("receive_data_until_error() end");
}

void SerialEndpoint::start() {
  std::lock_guard<std::mutex> lock(m_connect_receive_thread_mutex);
  m_console->debug("start()-begin");
  if (m_connect_receive_thread != nullptr) {
    m_console->debug("Already started");
    return;
  }
  _stop_requested = false;
  m_write_thread = std::make_unique<std::thread>(&SerialEndpoint::write_loop, this);
  m_connect_receive_thread = std::make_unique<std::thread>(
      &SerialEndpoint::connect_and_read_loop, this);
  m_console->debug("start()-end");
}

void SerialEndpoint::stop() {
  std::lock_guard<std::mutex> lock(m_connect_receive_thread_mutex);
  m_console->debug("stop()-begin");
  {
    std::lock_guard<std::mutex> queue_lock(m_tx_mutex);
    _stop_requested = true;
    m_tx_queue.clear();
  }
  m_tx_cv.notify_all();
  if (m_write_thread && m_write_thread->joinable()) m_write_thread->join();
  m_write_thread.reset();
  if (m_connect_receive_thread && m_connect_receive_thread->joinable()) {
    m_connect_receive_thread->join();
  }
  m_connect_receive_thread = nullptr;
  m_console->debug("stop()-end");
}

// based on mavsdk and what linux allows setting
// if a value is in the map, we allow the user to set it
static std::map<int, void*> valid_uart_baudrates() {
  std::map<int, void*> ret;
  ret[9600] = nullptr;
  ret[19200] = nullptr;
  ret[38400] = nullptr;
  ret[57600] = nullptr;
  ret[115200] = nullptr;
  ret[230400] = nullptr;
  ret[460800] = nullptr;
  ret[500000] = nullptr;
  ret[576000] = nullptr;
  ret[921600] = nullptr;
  ret[1000000] = nullptr;
  // I think it is sane to stop here, I doubt anything higher makes sense
  return ret;
}

bool SerialEndpoint::is_valid_linux_baudrate(int baudrate) {
  const auto supported = valid_uart_baudrates();
  if (supported.find(baudrate) != supported.end()) {
    return true;
  }
  return false;
}

void SerialEndpointManager::send_messages_if_enabled(
    const std::vector<MavlinkMessage>& messages) {
  std::lock_guard<std::mutex> guard(m_serial_endpoint_mutex);
  if (m_serial_endpoint) {
    m_serial_endpoint->sendMessages(messages);
  }
}

void SerialEndpointManager::disable() {
  std::lock_guard<std::mutex> guard(m_serial_endpoint_mutex);
  if (m_serial_endpoint != nullptr) {
    m_console->info("Stopping already existing FC UART");
    m_serial_endpoint->stop();
    m_serial_endpoint.reset();
    m_serial_endpoint = nullptr;
  }
}

void SerialEndpointManager::configure(const SerialEndpoint::HWOptions& options,
                                      const std::string& tag,
                                      MAV_MSG_CALLBACK cb) {
  std::lock_guard<std::mutex> guard(m_serial_endpoint_mutex);
  if (m_serial_endpoint != nullptr) {
    // Disable the currently running uart configuration, if there is any
    m_console->info("Stopping already existing FC UART");
    m_serial_endpoint->stop();
    m_serial_endpoint.reset();
    m_serial_endpoint = nullptr;
  }
  m_serial_endpoint = std::make_unique<SerialEndpoint>(tag, options);
  m_serial_endpoint->registerCallback(std::move(cb));
}

namespace {

bool device_path_exists(const char* path) { return access(path, F_OK) == 0; }

std::string role_alias_for(SerialPortRole role) {
  switch (role) {
    case SerialPortRole::Flight:
      return "/dev/Flight";
    case SerialPortRole::OpenHD:
      return "/dev/OpenHD";
    case SerialPortRole::Tracker:
      return "/dev/Tracker";
  }
  return "/dev/Flight";
}

std::string platform_default_for(SerialPortRole role) {
  if (role == SerialPortRole::Tracker) {
    return role_alias_for(role);
  }
  const auto platform = OHDPlatform::instance();
  if (platform.is_orqa()) {
    if (role == SerialPortRole::OpenHD) {
      return "/dev/ttymxc1";
    }
    return "/dev/ttymxc0";
  }
  if (platform.is_rpi()) {
    if (role == SerialPortRole::OpenHD) {
      return "/dev/serial1";
    }
    return "/dev/serial0";
  }
  if (platform.is_x20() || platform.is_rock()) {
    return "/dev/ttyS2";
  }
  if (platform.is_rv1126()) {
    return "/dev/ttyS4";
  }
  if (platform.is_luckfox_pico()) {
    return "/dev/ttyS3";
  }
  if (platform.is_luckfox_lyra()) {
    return "/dev/ttyS1";
  }
  openhd::log::get_default()->debug(
      "No default serial mapping for this platform");
  return "/dev/ttyS2";
}

}  // namespace

std::optional<std::string> serial_openhd_param_to_linux_fd(
    const std::string& param_name, SerialPortRole role) {
  if (param_name.empty()) {
    // "" means disabled
    return std::nullopt;
  }
  // Default mapping
  if (OHDUtil::str_equal(param_name, "DEFAULT")) {
    const auto alias = role_alias_for(role);
    if (device_path_exists(alias.c_str())) {
      return alias;
    }
    return platform_default_for(role);
  }
  // Otherwise, the user can enter any serial FD name
  return param_name;
}
