//
// Created by rsaxvc 31.07.22.
//

#include "FifoEndpoint.h"

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include <cassert>
#include <map>
#include <utility>

#include "openhd_util_filesystem.h"

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

FifoEndpoint::FifoEndpoint(std::string TAG1,FifoEndpoint::HWOptions options1):
                                                                                       MEndpoint(std::move(TAG1)), m_options(std::move(options1)){
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  //m_limited_rate_logger=std::make_unique<openhd::log::LimitedRateLogger>(m_console,std::chrono::milliseconds(1000));
  m_console->info("created with {}", m_options.to_string());
  start();
}

FifoEndpoint::~FifoEndpoint() {
  stop();
}

bool FifoEndpoint::sendMessagesImpl(const std::vector<MavlinkMessage>& messages) {
  auto message_buffers= pack_messages(messages);
  bool success= true;
  for(const auto& message_buffer:message_buffers){
    if(!write_data_fifo(message_buffer)){
      success= false;
    }
  }
  return success;
}

bool FifoEndpoint::write_data_fifo(const std::vector<uint8_t> &data){
  if(m_fd_tx ==-1){
    // cannot send data at the time, FIFO not setup / doesn't exist. Limit message to once per second
    const auto elapsed=std::chrono::steady_clock::now()-m_last_log_cannot_send_no_fd;
    if(elapsed>std::chrono::seconds(1)){
      m_console->warn("Cannot send data, no fd");
      m_last_log_cannot_send_no_fd=std::chrono::steady_clock::now();
    }
    return false;
  }
  const auto before=std::chrono::steady_clock::now();
  // If we have a fd, but the write fails, most likely the FIFO disconnected
  // but the linux driver hasn't noticed it yet.
  const auto send_len = static_cast<int>(write(m_fd_tx,data.data(), data.size()));
  const auto send_delta=std::chrono::steady_clock::now()-before;
  if(send_delta>std::chrono::milliseconds(1000)){
    const auto send_delta_ms=static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(send_delta).count())/1000.0f;
    m_console->warn("FIFO sending {} bytes took {}ms", send_len, send_delta_ms);
  }
  //m_console->debug("Written {} bytes",send_len);
  //m_console->debug("{}",MEndpoint::get_tx_rx_stats());
  if (send_len != data.size()) {
    m_n_failed_writes++;
    const auto elapsed_since_last_log=std::chrono::steady_clock::now()-m_last_log_serial_write_failed;
    if(elapsed_since_last_log>MIN_DELAY_BETWEEN_SERIAL_WRITE_FAILED_LOG_MESSAGES){
      m_console->warn("wrote {} instead of {} bytes,n failed:{}",send_len,data.size(),m_n_failed_writes);
      m_last_log_serial_write_failed=std::chrono::steady_clock::now();
    }
    return false;
  }
  return true;
}

bool FifoEndpoint::isafifo(int fd)
{
  struct stat sb;

  if (fstat(fd, &sb) == -1) {
    return false;
  }

  return !!((sb.st_mode & S_IFMT) == S_IFIFO);
}

int FifoEndpoint::setup_fifo_rx(const FifoEndpoint::HWOptions &options,std::shared_ptr<spdlog::logger> m_console) {
  if(!m_console){
    m_console=openhd::log::get_default();
  }
  // open() hangs on FIFO devices until the other side is ready unless you give it O_NONBLOCK
  int fd = open(options.linux_filename_rx.c_str(), O_RDONLY | O_NONBLOCK);
  if (fd == -1) {
    m_console->warn("open {} failed: {}", options.linux_filename_rx, GET_ERROR());
    return -1;
  }
  // Clear O_NONBLOCK to allow blocking in our reader thread
  if (fcntl(fd, F_SETFL, 0) == -1) {
    m_console->warn("fcntl failed: {}",GET_ERROR());
    close(fd);
    return -1;
  }
  if(!isafifo(fd)){
    m_console->warn("file descriptor {} is NOT a FIFO",options.linux_filename_rx);
    close(fd);
    return -1;
  }
  return fd;
}

int FifoEndpoint::setup_fifo_tx(const FifoEndpoint::HWOptions &options,std::shared_ptr<spdlog::logger> m_console) {
  if(!m_console){
    m_console=openhd::log::get_default();
  }
  // open() hangs on FIFO devices until the other side is ready unless you give it O_NONBLOCK
  // open() fails on FIFO devices until the other side is ready unless we open with O_RDWR
  int fd = open(options.linux_filename_tx.c_str(), O_RDWR | O_NONBLOCK);
  if (fd == -1) {
    m_console->warn("open {} failed: {}", options.linux_filename_tx, GET_ERROR());
    return -1;
  }
  // Clear O_NONBLOCK to match rx.
  if (fcntl(fd, F_SETFL, 0) == -1) {
    m_console->warn("fcntl failed: {}",GET_ERROR());
    close(fd);
    return -1;
  }
  if(!isafifo(fd)){
    m_console->warn("file descriptor {} is NOT a FIFO",options.linux_filename_tx);
    close(fd);
    return -1;
  }
  return fd;
}

void FifoEndpoint::connect_and_read_loop() {
  while (!_stop_requested && m_fd_tx == -1){
    if(!OHDFilesystemUtil::exists(m_options.linux_filename_tx)){
      m_console->warn("{} does not exist", m_options.linux_filename_tx);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    m_fd_tx =setup_fifo_tx(m_options,m_console);
    if(m_fd_tx == -1){
      m_console->warn("Cannot create FIFO RX fd for : {}",m_options.to_string());
    }
  }

  while (!_stop_requested){
    if(!OHDFilesystemUtil::exists(m_options.linux_filename_rx)){
      m_console->warn("{} does not exist", m_options.linux_filename_rx);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    // The files exist, so creating the FD should be no problem
    m_fd_rx =setup_fifo_rx(m_options,m_console);

    if(m_fd_rx ==-1){
      // But if it fails, we start over again, checking if at least the linux fd exists
      m_console->warn("Cannot create FIFO RX fd for : {}",m_options.to_string());
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    m_console->debug("Successfully created FIFO fd for: {}",m_options.to_string());
    receive_data_until_error();
    // cleanup and start over again
    close(m_fd_rx);
    m_fd_rx =-1;
  }
  if (m_fd_tx != -1) {
      close(m_fd_tx);
    m_fd_tx =-1;
  }
}

void FifoEndpoint::receive_data_until_error() {
  m_console->debug("receive_data_until_error() begin");
  // Enough for MTU 1500 bytes.
  uint8_t buffer[2048];

  struct pollfd fds[1];
  fds[0].fd = m_fd_rx;
  fds[0].events = POLLIN;
  m_n_failed_reads=0;

  while (!_stop_requested) {
    const auto before=std::chrono::steady_clock::now();
    const int pollrc = poll(fds, 1, 1000);
    //const auto delta=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-before).count();
    //std::cout<<"Poll res:"<<pollrc<<" took:"<<delta<<" ms\n";
    //debug_poll_fd(fds[0]);
    if (fds[0].revents & POLLHUP) {
      // The other end of the RX FIFO hung up, need to reopen both FIFOs
      return;
    } else if (pollrc == 0 || !(fds[0].revents & POLLIN)) {
      // if we land here, no data has become available after X ms. Not strictly an error,
      // but on a FC which constantly provides a data stream it most likely is an error.
      m_n_failed_reads++;
      const auto elapsed_since_last_log=std::chrono::steady_clock::now()-m_last_log_serial_read_failed;
      if(elapsed_since_last_log>=MIN_DELAY_BETWEEN_SERIAL_READ_FAILED_LOG_MESSAGES && m_options.enable_reading){
        m_last_log_serial_read_failed=std::chrono::steady_clock::now();
        m_console->warn("{} failed polls(reads)",m_n_failed_reads);
      }else{
        //m_console->debug("poll probably timeout {}",m_n_failed_reads);
      }
      continue;
    } else if (pollrc == -1) {
      m_console->warn("read poll failure: {}",GET_ERROR());
      // The FIFO most likely disconnected.
      return;
    }
    // We enter here if (fds[0].revents & POLLIN) == true
    const int recv_len = static_cast<int>(read(m_fd_rx, buffer, sizeof(buffer)));
    if(recv_len>0){
      m_n_failed_reads = 0;
      MEndpoint::parseNewData(buffer,recv_len);
    }else if(recv_len==0) {
      // timeout
    }else{
      m_console->warn("read failure: {} {}",recv_len,GET_ERROR());
    }
  }
  m_console->debug("receive_data_until_error() end");
}

void FifoEndpoint::start() {
  std::lock_guard<std::mutex> lock(m_connect_receive_thread_mutex);
  m_console->debug("start()-begin");
  if(m_connect_receive_thread != nullptr){
    m_console->debug("Already started");
    return;
  }
  _stop_requested= false;
  m_connect_receive_thread =std::make_unique<std::thread>(&FifoEndpoint::connect_and_read_loop, this);
  m_console->debug("start()-end");
}

void FifoEndpoint::stop() {
  std::lock_guard<std::mutex> lock(m_connect_receive_thread_mutex);
  m_console->debug("stop()-begin");
  _stop_requested=true;
  if (m_connect_receive_thread && m_connect_receive_thread->joinable()) {
    m_connect_receive_thread->join();
  }
  m_connect_receive_thread = nullptr;
  m_console->debug("stop()-end");
}

void FifoEndpointManager::send_messages_if_enabled(
    const std::vector<MavlinkMessage>& messages) {
  std::lock_guard<std::mutex> guard(m_serial_endpoint_mutex);
  if(m_serial_endpoint){
    m_serial_endpoint->sendMessages(messages);
  }
}

void FifoEndpointManager::disable() {
  std::lock_guard<std::mutex> guard(m_serial_endpoint_mutex);
  if(m_serial_endpoint !=nullptr) {
    m_console->info("Stopping already existing FC FIFO");
    m_serial_endpoint->stop();
    m_serial_endpoint.reset();
    m_serial_endpoint =nullptr;
  }
}

void FifoEndpointManager::configure(const FifoEndpoint::HWOptions& options,const std::string& tag,MAV_MSG_CALLBACK cb) {
  std::lock_guard<std::mutex> guard(m_serial_endpoint_mutex);
  if(m_serial_endpoint !=nullptr) {
    // Disable the currently running fifo configuration, if there is any
    m_console->info("Stopping already existing FC FIFO");
    m_serial_endpoint->stop();
    m_serial_endpoint.reset();
    m_serial_endpoint =nullptr;
  }
  m_serial_endpoint =std::make_unique<FifoEndpoint>(tag,options);
  m_serial_endpoint->registerCallback(std::move(cb));
}
