//
// Created by consti10 on 28.03.23.
//

#include "openhd_util.h"
#include "wifi_command_helper.h"
//#include "wifi_command_helper2.h"
#include "wifi_card_discovery.h"

#include <vector>
#include <utility>

//
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <unistd.h>


static bool test_set_wifi_channel(const std::string& device_name,int channel_number){
  auto console=openhd::log::get_default();
  console->debug("test_set_wifi_channel {} {}",device_name,channel_number);
  int sockfd;
  struct iwreq wrq{};

  // Open socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    openhd::log::get_default()->warn("test_set_wifi_channel - cannot create socket");
    return false;
  }

  // Set interface name
  strncpy(wrq.ifr_name, device_name.c_str(), IFNAMSIZ);

  // Set channel
  wrq.u.freq.m = channel_number;
  wrq.u.freq.e=0;
  wrq.u.freq.flags = IW_FREQ_FIXED;

  // Send IOCTL to set channel
  if (ioctl(sockfd, SIOCSIWFREQ, &wrq) < 0) {
    openhd::log::get_default()->warn("test_set_wifi_channel - ioctl");
    perror("ioctl");
    close(sockfd);
    return false;
  }
  // Close socket
  close(sockfd);

  openhd::log::get_default()->debug("Channel set to {}",channel_number);
  return true;
}

int main(int argc, char *argv[]) {
  OHDUtil::terminate_if_not_root();

  std::string card_name="wlx244bfeb71c05";
  int channel_num=177;

  if(argc<3){
    openhd::log::get_default()->warn("Usage [wifi card] [channel number], using defaults");
  }else{
    card_name = argv[1];
    channel_num = atoi(argv[2]);
  }
  test_set_wifi_channel(card_name,channel_num);
  return 0;
}