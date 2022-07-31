//
// Created by consti10 on 31.07.22.
//

#include "SerialEndpoint3.h"
#include "openhd-util-filesystem.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <utility>
#include <cassert>

static const char* GET_ERROR(){
  return strerror(errno);
}

SerialEndpoint3::SerialEndpoint3(std::string TAG1,SerialEndpoint3::HWOptions options1):
	MEndpoint(std::move(TAG1)),
	_options(std::move(options1)){
  std::cout<<"SerialEndpoint3: created with "<<_options.to_string()<<"\n";
}

void SerialEndpoint3::sendMessageImpl(const MavlinkMessage &message) {
  /*if(_fd==-1){
	// cannot send data at the time, UART not setup / doesn't exist.
	std::cout<<"Cannot send data, no fd\n";
	return;
  }*/
  const auto data = message.pack();
  const auto send_len = static_cast<int>(write(_fd,data.data(), data.size()));
  if (send_len != data.size()) {
	std::stringstream ss;
	ss<<" Failure to write uart data: "<<data.size()<<" actual: "<<send_len<<GET_ERROR()<<"\n";
	std::cerr << ss.str();
	return;
  }
}

int SerialEndpoint3::define_from_baudrate(int baudrate) {
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
	  assert(false);
	  return -1;
	}
  }
}

int SerialEndpoint3::setup_port(const SerialEndpoint3::HWOptions &options) {
  // open() hangs on macOS or Linux devices(e.g. pocket beagle) unless you give it O_NONBLOCK
  int fd = open(options.linux_filename.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd == -1) {
	std::cerr << "open failed: " << GET_ERROR();
	return -1;
  }
  // We need to clear the O_NONBLOCK again because we can block while reading
  // as we do it in a separate thread.
  if (fcntl(fd, F_SETFL, 0) == -1) {
	std::cerr << "fcntl failed: " << GET_ERROR();
	close(fd);
	return -1;
  }
  struct termios tc{};
  bzero(&tc, sizeof(tc));
  if (tcgetattr(fd, &tc) != 0) {
	std::cerr << "tcgetattr failed: " << GET_ERROR();
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
  if (baudrate_or_define == -1) {
	close(fd);
	return -1;
  }
  if (cfsetispeed(&tc, baudrate_or_define) != 0) {
	std::cerr << "cfsetispeed failed: " << GET_ERROR();
	close(fd);
	return -1;
  }
  if (cfsetospeed(&tc, baudrate_or_define) != 0) {
	std::cerr << "cfsetospeed failed: " << GET_ERROR();
	close(fd);
	return -1;
  }
  if (tcsetattr(fd, TCSANOW, &tc) != 0) {
	std::cerr << "tcsetattr failed: " << GET_ERROR();
	close(fd);
	return -1;
  }
  return fd;
}

void SerialEndpoint3::connect_and_read_loop() {
  while (!_stop_requested){
	if(!OHDFilesystemUtil::exists(_options.linux_filename.c_str())){
	  std::cout<<"UART file does not exist\n";
	  std::this_thread::sleep_for(std::chrono::seconds(1));
	  continue;
	}
	// The file exists, so creating the FD should be no problem
	_fd=setup_port(_options);
	if(_fd==-1){
	  // But if it fails, we start over again, checking if at least the linux fd exists
	  std::cerr<<"Cannot create uart fd "<<_options.to_string()<<"\n";
	  std::this_thread::sleep_for(std::chrono::seconds(1));
	  continue;
	}
	std::cout<<"Successfully created UART fd for:"<<_options.to_string()<<"\n";
	receive_data_until_error();
	// cleanup and start over again
	close(_fd);
	_fd=-1;
  }
}

void SerialEndpoint3::receive_data_until_error() {
  std::cout<<"SerialEndpoint3::receive_data_until_error() begin\n";
  // Enough for MTU 1500 bytes.
  uint8_t buffer[2048];

  struct pollfd fds[1];
  fds[0].fd = _fd;
  fds[0].events = POLLIN;

  while (!_stop_requested) {
	int recv_len;
	int pollrc = poll(fds, 1, 1000);
	if (pollrc == 0 || !(fds[0].revents & POLLIN)) {
	  continue;
	} else if (pollrc == -1) {
	  std::cerr<< "read poll failure: " << GET_ERROR();
	  // The UART most likely disconnected.
	  return;
	}
	// We enter here if (fds[0].revents & POLLIN) == true
	recv_len = static_cast<int>(read(_fd, buffer, sizeof(buffer)));
	if (recv_len < -1) {
	  std::cerr << "read failure: " << GET_ERROR();
	}
	if (recv_len > static_cast<int>(sizeof(buffer)) || recv_len == 0) {
	  // probably timeout
	  continue;
	}
	std::cout<<"UART got data\n";
	MEndpoint::parseNewData(buffer,recv_len);
  }
  std::cout<<"SerialEndpoint3::receive_data_until_error() end\n";
}

void SerialEndpoint3::start() {
  std::lock_guard<std::mutex> lock(_connectReceiveThreadMutex);
  if(_connectReceiveThread!= nullptr){
	std::cout<<"Already started\n";
	return;
  }
  _connectReceiveThread=std::make_unique<std::thread>(&SerialEndpoint3::connect_and_read_loop, this);
}

void SerialEndpoint3::stop() {
  std::lock_guard<std::mutex> lock(_connectReceiveThreadMutex);
  _stop_requested=true;
  if (_connectReceiveThread && _connectReceiveThread->joinable()) {
	_connectReceiveThread->join();
  }
  _connectReceiveThread = nullptr;
}
