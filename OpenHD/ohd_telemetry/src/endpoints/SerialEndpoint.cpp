//
// Created by consti10 on 31.07.22.
//

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
#include "openhd_spdlog_include.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"

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
  start();
}

SerialEndpoint::~SerialEndpoint() { stop(); }

bool SerialEndpoint::sendMessagesImpl(
    const std::vector<MavlinkMessage>& messages) {
  auto message_buffers = aggregate_pack_messages(messages);
  bool success = true;
  for (const auto& message_buffer : message_buffers) {
    if (!write_data_serial(*message_buffer.aggregated_data)) {
      success = false;
    }
  }
  return success;
}

bool SerialEndpoint::write_data_serial(const std::vector<uint8_t>& data) {
  // m_console->debug("Write data serial:{} bytes",data.size());
  if (m_fd == -1) {
    // cannot send data at the time, UART not setup / doesn't exist. Limit
    // message to once per second
    const auto elapsed =
        std::chrono::steady_clock::now() - m_last_log_cannot_send_no_fd;
    if (elapsed > std::chrono::seconds(1)) {
      m_console->warn("Cannot send data, no fd");
      m_last_log_cannot_send_no_fd = std::chrono::steady_clock::now();
    }
    return false;
  }
  const auto before = std::chrono::steady_clock::now();
  // If we have a fd, but the write fails, most likely the UART disconnected
  // but the linux driver hasn't noticed it yet.
  const auto send_len = static_cast<int>(write(m_fd, data.data(), data.size()));
  const auto send_delta = std::chrono::steady_clock::now() - before;
  if (send_delta > std::chrono::milliseconds(100)) {
    const auto send_delta_ms =
        static_cast<float>(
            std::chrono::duration_cast<std::chrono::microseconds>(send_delta)
                .count()) /
        1000.0f;
    m_console->warn("UART sending data took {}ms", send_delta_ms);
  }
  // m_console->debug("Written {} bytes",send_len);
  // m_console->debug("{}",MEndpoint::get_tx_rx_stats());
  if (send_len != data.size()) {
    m_n_failed_writes++;
    const auto elapsed_since_last_log =
        std::chrono::steady_clock::now() - m_last_log_serial_write_failed;
    if (elapsed_since_last_log >
        MIN_DELAY_BETWEEN_SERIAL_WRITE_FAILED_LOG_MESSAGES) {
      m_console->warn("wrote {} instead of {} bytes,n failed:{}", send_len,
                      data.size(), m_n_failed_writes);
      m_last_log_serial_write_failed = std::chrono::steady_clock::now();
    }
    return false;
  }
  return true;
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
  // We need to clear the O_NONBLOCK again because we can block while reading
  // as we do it in a separate thread.
  if (fcntl(fd, F_SETFL, 0) == -1) {
    m_console->warn("fcntl failed: {}", GET_ERROR());
    close(fd);
    return -1;
  }
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
      m_console->warn("UART file does not exist");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    // The file exists, so creating the FD should be no problem
    m_fd = setup_port(m_options, m_console);
    if (m_fd == -1) {
      // But if it fails, we start over again, checking if at least the linux fd
      // exists
      m_console->warn("Cannot create uart fd " + m_options.to_string());
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    m_console->debug("Successfully created UART fd for: {}",
                     m_options.to_string());
    receive_data_until_error();
    // cleanup and start over again
    close(m_fd);
    m_fd = -1;
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

  while (!_stop_requested) {
    const auto before = std::chrono::steady_clock::now();
    const int pollrc = poll(fds, 1, 1000);
    // on my ubuntu laptop, with usb serial, if the device disconnects I don't
    // get any error results, but poll suddenly never blocks anymore. Therefore,
    // every time we time out we check if the fd is still valid and exit if not
    // (which will lead to a re-start)
    const auto valid = is_serial_fd_still_connected(m_fd);
    if (!valid) {
      m_console->debug("Exiting serial, not connected");
      return;
    }
    // const auto
    // delta=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-before).count();
    // std::cout<<"Poll res:"<<pollrc<<" took:"<<delta<<" ms\n";
    // debug_poll_fd(fds[0]);
    if (pollrc == 0 || !(fds[0].revents & POLLIN)) {
      // if we land here, no data has become available after X ms. Not strictly
      // an error, but on a FC which constantly provides a data stream it most
      // likely is an error.
      m_n_failed_reads++;
      const auto elapsed_since_last_log =
          std::chrono::steady_clock::now() - m_last_log_serial_read_failed;
      if (elapsed_since_last_log >=
              MIN_DELAY_BETWEEN_SERIAL_READ_FAILED_LOG_MESSAGES &&
          m_options.enable_reading) {
        m_last_log_serial_read_failed = std::chrono::steady_clock::now();
        m_console->warn("{} failed reads - FC connected ?", m_n_failed_reads);
      } else {
        // m_console->debug("poll probably timeout {}",m_n_failed_reads);
      }
      continue;
    } else if (pollrc == -1) {
      m_console->warn("read poll failure: {}", GET_ERROR());
      // The UART most likely disconnected.
      return;
    }
    // We enter here if (fds[0].revents & POLLIN) == true
    const int recv_len = static_cast<int>(read(m_fd, buffer, sizeof(buffer)));
    if (recv_len > 0) {
      MEndpoint::parseNewData(buffer, recv_len);
    } else if (recv_len == 0) {
      // timeout
    } else {
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
  m_connect_receive_thread = std::make_unique<std::thread>(
      &SerialEndpoint::connect_and_read_loop, this);
  m_console->debug("start()-end");
}

void SerialEndpoint::stop() {
  std::lock_guard<std::mutex> lock(m_connect_receive_thread_mutex);
  m_console->debug("stop()-begin");
  _stop_requested = true;
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

std::optional<std::string> serial_openhd_param_to_linux_fd(
    const std::string& param_name) {
  if (param_name.empty()) {
    // "" means disabled
    return std::nullopt;
  }
  // Default mapping
  if (OHDUtil::str_equal(param_name, "DEFAULT")) {
    const auto platform = OHDPlatform::instance();
    if (platform.is_rpi()) {
      return "/dev/serial0";
    } else if (platform.is_x20() || platform.is_rock()) {
      return "dev/ttyS2";
    } else {
      openhd::log::get_default()->warn(
          "No default serial mapping for this platform");
      // fallback
      return "dev/ttyS2";
    }
  }
  // Otherwise, the user can enter any serial FD name
  return param_name;
}
