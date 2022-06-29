#ifndef OPENHD_LOG_MESSAGES_H
#define OPENHD_LOG_MESSAGES_H

#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>
#include <iostream>
#include <sstream>
#include <cassert>

#include "openhd-global-constants.h"

/**
 * This provides convenient methods to send log messages from any service running on the air or ground unit to a final output deice,
 * for example QOpenHD.
 * As an example, this makes it possible to view the logs from the air unit on QOpenHD without connecting a display to the air unit.
 * The general way this works is simple:
 * The log messages is sent to a specific udp port on localhost and then picked up by the telemetry service,
 * which converts it to mavlink and forwards it accordingly.
 */

struct OHDLocalLogMessage{
  uint8_t level;
  uint8_t message[50];
  // returns true if the message has a proper null terminator.
  // Only in this case we can treat the raw string array as a string.
  [[nodiscard]] bool hasNullTerminator() const {
	// check if the string has a null-terminator
	bool nullTerminatorFound = false;
	for (const auto &i: message) {
	  if (i == '\0') {
		nullTerminatorFound = true;
		break;
	  }
	}
	return nullTerminatorFound;
  }
} __attribute__((packed));


// these match the mavlink SEVERITY_LEVEL enum, but this code should not depend on
// the mavlink headers
// See https://mavlink.io/en/messages/common.html#MAV_SEVERITY
typedef enum STATUS_LEVEL {
  STATUS_LEVEL_EMERGENCY = 0,
  STATUS_LEVEL_ALERT,
  STATUS_LEVEL_CRITICAL,
  STATUS_LEVEL_ERROR,
  STATUS_LEVEL_WARNING,
  STATUS_LEVEL_INFO,
  STATUS_LEVEL_NOTICE,
  STATUS_LEVEL_DEBUG
} STATUS_LEVEL;

static void print_log_by_level(const STATUS_LEVEL level, std::string message) {
  // Each message is logged with a newline at the end, add a new line at the end if non-existing.
  const auto messageN = message.back() == '\n' ? message : (message + "\n");
  if (level == STATUS_LEVEL_INFO || level == STATUS_LEVEL_NOTICE || level == STATUS_LEVEL_DEBUG) {
	std::cout << messageN;
  } else {
	std::cerr << messageN;
  }
}

/**
 * Send a log message out via udp to localhost, it wll be picked up by the telemetry service.
 * @param message the log message to send, has to have a valid null terminator.
 */
static void sendLocalLogMessageUDP(const OHDLocalLogMessage& message){
  assert(message.hasNullTerminator());
  int sockfd;
  struct sockaddr_in servaddr{};
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("Socket create failed");
	exit(EXIT_FAILURE);
  }
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(OHD_LOCAL_LOG_MESSAGES_UDP_PORT);
  inet_aton("127.0.0.1", (in_addr *)&servaddr.sin_addr.s_addr);
  sendto(sockfd, &message, sizeof(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
  close(sockfd);
}

/*
 * Messages sent here will end up in the telemetry microservice, where they will be packed up and sent through
 * mavlink for storage and review by qopenhd, the boot screen system, and other software.
 */
static void ohd_log(STATUS_LEVEL level, const std::string &message) {
  print_log_by_level(level, message);
  OHDLocalLogMessage lmessage{};
  lmessage.level = static_cast<uint8_t>(level);
  strncpy((char *)lmessage.message, message.c_str(), 50);
  if (lmessage.message[49] != '\0') {
	lmessage.message[49] = '\0';
  }
  sendLocalLogMessageUDP(lmessage);
}

class OpenHDLogger{
 public:
  //explicit OpenHDLogger(const STATUS_LEVEL level=STATUS_LEVEL_DEBUG,const std::string& tag=""):
  //  _status_level(level),_tag(tag) {}
  explicit OpenHDLogger(const STATUS_LEVEL level=STATUS_LEVEL_DEBUG,std::string_view tag=""):
    _status_level(level),_tag(tag){}
  ~OpenHDLogger() {
    const auto tmp=stream.str();
    log_message(tmp);
  }
  OpenHDLogger(const OpenHDLogger& other)=delete;
  // the non-member function operator<< will now have access to private members
  template <typename T>
  friend OpenHDLogger& operator<<(OpenHDLogger& record, T&& t);
 private:
  const STATUS_LEVEL _status_level;
  const std::string_view _tag;
  std::stringstream stream;
  // Checks for a newline, and if detected logs the message immediately and then clears it.
  void log_immediately_on_newline(){
    const auto tmp=stream.str();
    if(!tmp.empty() && tmp.back()=='\n') {
      log_message(tmp);
      stream.str("");
    }
  }
  void log_message(const std::string& message){
    if(message.empty())return;
    ohd_log(_status_level,message);
  }
};

template <typename T>
OpenHDLogger& operator<<(OpenHDLogger& record, T&& t) {
  record.stream << std::forward<T>(t);
  // If we detect a newline, log immediately, not delayed when going out of scope to mimic std::cout behaviour
  record.log_immediately_on_newline();
  return record;
}
template <typename T>
OpenHDLogger& operator<<(OpenHDLogger&& record, T&& t) {
  // This just uses the operator declared above.
  record << std::forward<T>(t);
  return record;
}

// macro for logging like std::cout in OpenHD
#define LOGD OpenHDLogger(STATUS_LEVEL_DEBUG,"")

// macro for logging like std::cerr in OpenHD
#define LOGE OpenHDLogger(STATUS_LEVEL_ERROR,"")

#define LOGI OpenHDLogger(STATUS_LEVEL_INFO,"")

#endif //OPENHD_LOG_MESSAGES_H
