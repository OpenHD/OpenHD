//
// Created by consti10 on 18.11.22.
//

#include "wifi_command_helper2.h"

#include <algorithm>
#include <thread>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <linux/wireless.h>
#include <ifaddrs.h>
#undef __USE_MISC
#include <net/if.h>
#include <linux/if.h>

bool wifi::commandhelper::set_wifi_frequency(const std::string& device,
                                             uint32_t freq_mhz) {
  return false;
}
