//
// Created by consti10 on 31.07.22.
//

#include "SerialEndpoint.h"
#include "openhd-util-filesystem.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <utility>
#include <cassert>
#include <sys/stat.h>

static std::string GET_ERROR(){
  return {strerror(errno)};
}
static void debug_poll_fd(const struct pollfd& poll_fd){
  std::stringstream ss;
  ss<<"Poll:{";
  if(poll_fd.events & POLLERR){
    ss<<"POLLERR,";
  }
  if(poll_fd.events & POLLHUP){
    ss<<"POLLHUP,";
  }
  if(poll_fd.events & POLLNVAL){
    ss<<"POLLNVAL,";
  }
  ss<<"}\n";
  std::cout<<ss.str();
}

// https://stackoverflow.com/questions/12340695/how-to-check-if-a-given-file-descriptor-stored-in-a-variable-is-still-valid
static bool is_serial_fd_still_connected(const int fd){
  struct stat s{};
  fstat(fd, &s);
  // struct stat::nlink_t   st_nlink;   ... number of hard links
  if( s.st_nlink < 1 ){
    // treat device disconnected case
    return false;
  }
  return true;
}

SerialEndpoint::SerialEndpoint(std::string TAG1,SerialEndpoint::HWOptions options1):
                                                                                       MEndpoint(std::move(TAG1)),
                                                                                       _options(std::move(options1)){
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  m_console->info("SerialEndpoint: created with "+_options.to_string());
  start();
}

SerialEndpoint::~SerialEndpoint() {
  stop();
}

bool SerialEndpoint::sendMessagesImpl(const std::vector<MavlinkMessage>& messages) {
  auto message_buffers= pack_messages(messages);
  bool success= true;
  for(const auto& message_buffer:message_buffers){
    if(!write_data_serial(message_buffer)){
      success= false;
    }
  }
  return success;
}

bool SerialEndpoint::write_data_serial(const std::vector<uint8_t> &data){
  m_console->debug("Write data serial:{}",data.size());
  if(_fd==-1){
    // cannot send data at the time, UART not setup / doesn't exist.
    m_console->warn("Cannot send data, no fd");
    return false;
  }
  // If we have a fd, but the write fails, most likely the UART disconnected
  // but the linux driver hasn't noticed it yet.
  const auto send_len = static_cast<int>(write(_fd,data.data(), data.size()));
  if (send_len != data.size()) {
    n_failed_writes++;
    const auto elapsed_since_last_log=std::chrono::steady_clock::now()-m_last_log_serial_write_failed;
    if(elapsed_since_last_log>MIN_DELAY_BETWEEN_SERIAL_WRITE_FAILED_LOG_MESSAGES){
      std::stringstream ss;
      ss<<"F UART write:"<<data.size()<<" actual:"<<send_len<<","<<GET_ERROR()<<"tot:"<<n_failed_writes;
      m_console->warn(ss.str());
      m_last_log_serial_write_failed=std::chrono::steady_clock::now();
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
      std::cerr << "Unknown baudrate\n";
      return B1152000;
    }
  }
}

int SerialEndpoint::setup_port(const SerialEndpoint::HWOptions &options,std::shared_ptr<spdlog::logger> m_console) {
  if(!m_console){
    m_console=openhd::log::get_default();
  }
  // open() hangs on macOS or Linux devices(e.g. pocket beagle) unless you give it O_NONBLOCK
  int fd = open(options.linux_filename.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd == -1) {
    m_console->warn("open failed: {}",GET_ERROR());
    return -1;
  }
  // We need to clear the O_NONBLOCK again because we can block while reading
  // as we do it in a separate thread.
  if (fcntl(fd, F_SETFL, 0) == -1) {
    m_console->warn("fcntl failed: {}",GET_ERROR());
    close(fd);
    return -1;
  }
  struct termios tc{};
  bzero(&tc, sizeof(tc));
  if (tcgetattr(fd, &tc) != 0) {
    m_console->warn("tcgetattr failed: {}",GET_ERROR());
    close(fd);
    return -1;
  }
  tc.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
  tc.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OPOST);
  tc.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG | TOSTOP);
  tc.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
  tc.c_cflag |= CS8;
  tc.c_cc[VMIN] = 0; // We are ok with 0 bytes.
  tc.c_cc[VTIME] = 10; // Timeout after 1 second.
  if (options.flow_control) {
    tc.c_cflag |= CRTSCTS;
  }
  tc.c_cflag |= CLOCAL; // Without this a write() blocks indefinitely.
  //
  const int baudrate_or_define = define_from_baudrate(options.baud_rate);
  if (cfsetispeed(&tc, baudrate_or_define) != 0) {
    m_console->warn("cfsetispeed failed: {}",GET_ERROR());
    close(fd);
    return -1;
  }
  if (cfsetospeed(&tc, baudrate_or_define) != 0) {
    m_console->warn("cfsetospeed failed: {}",GET_ERROR());
    close(fd);
    return -1;
  }
  if (tcsetattr(fd, TCSANOW, &tc) != 0) {
    m_console->warn("tcsetattr failed: {}",GET_ERROR());
    close(fd);
    return -1;
  }
  return fd;
}

void SerialEndpoint::connect_and_read_loop() {
  while (!_stop_requested){
    if(!OHDFilesystemUtil::exists(_options.linux_filename.c_str())){
      m_console->warn("UART file does not exist");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    // The file exists, so creating the FD should be no problem
    _fd=setup_port(_options,m_console);
    if(_fd==-1){
      // But if it fails, we start over again, checking if at least the linux fd exists
      m_console->warn("Cannot create uart fd "+_options.to_string());
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    m_console->debug("Successfully created UART fd for: {}",_options.to_string());
    receive_data_until_error();
    // cleanup and start over again
    close(_fd);
    _fd=-1;
  }
}

void SerialEndpoint::receive_data_until_error() {
  m_console->debug("SerialEndpoint3::receive_data_until_error() begin");
  // Enough for MTU 1500 bytes.
  uint8_t buffer[2048];

  struct pollfd fds[1];
  fds[0].fd = _fd;
  fds[0].events = POLLIN;

  int n_failed_polls=0;

  while (!_stop_requested) {
    int recv_len;
    const auto before=std::chrono::steady_clock::now();
    const int pollrc = poll(fds, 1, 1000);
    // on my ubuntu laptop, with usb serial, if the device disconnects I don't get any error results,
    // but poll suddenly never blocks anymore. Therefore, every time we time out we check if the fd is still valid
    // and exit if not (which will lead to a re-start)
    const auto valid= is_serial_fd_still_connected(_fd);
    if(!valid){
      m_console->debug("Exiting serial, not connected");
      return;
    }
    //const auto delta=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-before).count();
    //std::cout<<"Poll res:"<<pollrc<<" took:"<<delta<<" ms\n";
    //debug_poll_fd(fds[0]);
    if (pollrc == 0 || !(fds[0].revents & POLLIN)) {
      n_failed_polls++;
      const auto elapsed_since_last_log=std::chrono::steady_clock::now()-m_last_log_serial_read_failed;
      if(elapsed_since_last_log>MIN_DELAY_BETWEEN_SERIAL_READ_FAILED_LOG_MESSAGES){
        m_last_log_serial_read_failed=std::chrono::steady_clock::now();
        m_console->warn("{} failed polls(reads)",n_failed_polls);
      }else{
        m_console->debug("poll probably timeout {}",n_failed_polls);
      }
      continue;
    } else if (pollrc == -1) {
      m_console->warn("read poll failure: {}",GET_ERROR());
      // The UART most likely disconnected.
      return;
    }
    // We enter here if (fds[0].revents & POLLIN) == true
    recv_len = static_cast<int>(read(_fd, buffer, sizeof(buffer)));
    if (recv_len < -1) {
      m_console->warn("read failure: {}",GET_ERROR());
    }
    if (recv_len > static_cast<int>(sizeof(buffer)) || recv_len == 0) {
      // probably timeout
      continue;
    }
    //std::cout<<"UART got data\n";
    MEndpoint::parseNewData(buffer,recv_len);
  }
  m_console->debug("SerialEndpoint3::receive_data_until_error() end");
}

void SerialEndpoint::start() {
  std::lock_guard<std::mutex> lock(_connectReceiveThreadMutex);
  m_console->debug("SerialEndpoint::start()-begin");
  if(_connectReceiveThread!= nullptr){
    m_console->debug("Already started");
    return;
  }
  _stop_requested= false;
  _connectReceiveThread=std::make_unique<std::thread>(&SerialEndpoint::connect_and_read_loop, this);
  m_console->debug("SerialEndpoint::start()-end");
}

void SerialEndpoint::stop() {
  std::lock_guard<std::mutex> lock(_connectReceiveThreadMutex);
  m_console->debug("SerialEndpoint3::stop()-begin");
  _stop_requested=true;
  if (_connectReceiveThread && _connectReceiveThread->joinable()) {
    _connectReceiveThread->join();
  }
  _connectReceiveThread = nullptr;
  m_console->debug("SerialEndpoint3::stop()-end");
}

