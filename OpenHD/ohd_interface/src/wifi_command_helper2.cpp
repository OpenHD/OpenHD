//
// Created by consti10 on 18.11.22.
//

#include "wifi_command_helper2.h"

#include "openhd-spdlog.hpp"
#include "openhd-util.hpp"

#include <algorithm>
#include <cstdint>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#undef __USE_MISC
#include <linux/if.h>
#include <net/if.h>
#include <sys/ioctl.h>

static std::shared_ptr<spdlog::logger> get_logger(){
  return openhd::log::create_or_get("w_helper2");
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
  int *ret = reinterpret_cast<int*>(arg);
  *ret = err->error;
  get_logger()->warn("error_handler {}",err->error);
  return NL_STOP;
}

bool wifi::commandhelper2::set_wifi_up_down(const std::string &device, bool up) {
  get_logger()->debug("set_wifi_up_down {} up:{}",device,OHDUtil::yes_or_no(up));
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  assert(sockfd>=0);
  struct ifreq ifr{};
  memset(&ifr, 0, sizeof ifr);
  strncpy(ifr.ifr_name, device.c_str(),
          std::min(static_cast<size_t>(IFNAMSIZ), device.length()));
  if (up) {
    ifr.ifr_flags = ifr.ifr_flags | IFF_UP;
  } else {
    ifr.ifr_flags = ifr.ifr_flags & ~IFF_UP;
  }
  const bool ret=(ioctl(sockfd, SIOCSIFFLAGS, &ifr) >= 0);
  if(!ret){
    get_logger()->warn("set_wifi_up_down failed");
  }
  return ret;
}


bool wifi::commandhelper2::set_wifi_monitor_mode(const std::string &device) {
  get_logger()->debug("set_wifi_monitor_mode {} ",device);
  //The device must be down to change the mode
  if (!set_wifi_up_down(device, false)) {
    return false;
  }

  // The v5.6.4.2 rtl8812au driver throttles monitor mode trafic by default
  // removed, we are not on v5.6.4.2
  /*FILE *fp = fopen("/sys/module/88XXau/parameters/rtw_monitor_disable_1m", "w");
  if (fp) {
    fprintf(fp, "1");
    fclose(fp);
  }*/

  // Create the socket and connect to it.
  struct nl_sock *sckt = nl_socket_alloc();
  genl_connect(sckt);

  // Allocate a new message.
  struct nl_msg *mesg = nlmsg_alloc();

  // Check /usr/include/linux/nl80211.h for a list of commands and attributes.
  enum nl80211_commands command = NL80211_CMD_SET_INTERFACE;

  // Create the message so it will send a command to the nl80211 interface.
  genlmsg_put(mesg, 0, 0, genl_ctrl_resolve(sckt, "nl80211"), 0, 0, command, 0);

  // Add specific attributes to change the frequency of the device.
  NLA_PUT_U32(mesg, NL80211_ATTR_IFINDEX, if_nametoindex(device.c_str()));
  NLA_PUT_U32(mesg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_MONITOR);

  // Finally send it and receive the amount of bytes sent.
  nl_send_auto_complete(sckt, mesg);

  // Bring the device back up
  if (!set_wifi_up_down(device, true)) {
    goto nla_put_failure;
  }

  nlmsg_free(mesg);
  nl_socket_free(sckt);
  return true;

nla_put_failure:
  nlmsg_free(mesg);
  nl_socket_free(sckt);
  return false;
}

bool wifi::commandhelper2::set_wifi_frequency(const std::string& device,
                                             uint32_t freq_mhz,std::optional<uint32_t> channel_width) {
  get_logger()->debug("set_wifi_frequency {} {}Mhz",device,freq_mhz);
  int err = 1;
  struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
  int rc;

  // Create the socket and connect to it.
  struct nl_sock *sckt = nl_socket_alloc();
  genl_connect(sckt);

  // Allocate a new message.
  struct nl_msg *mesg = nlmsg_alloc();

  // Check /usr/include/linux/nl80211.h for a list of commands and attributes.
  enum nl80211_commands command = NL80211_CMD_SET_WIPHY;

  // Create the message so it will send a command to the nl80211 interface.
  genlmsg_put(mesg, 0, 0, genl_ctrl_resolve(sckt, "nl80211"), 0, 0, command, 0);

  // Add specific attributes to change the frequency of the device.
  NLA_PUT_U32(mesg, NL80211_ATTR_IFINDEX, if_nametoindex(device.c_str()));
  NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_FREQ, freq_mhz);
  if(channel_width!=std::nullopt){
    const uint32_t channel_width_given=channel_width.value();
    NLA_PUT_U32(mesg,NL80211_ATTR_CHANNEL_WIDTH,channel_width_given);
  }

  // Finally send it and receive the amount of bytes sent.
  nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
  rc = nl_send_auto(sckt, mesg);
  nl_recvmsgs(sckt, cb);
  nl_cb_put(cb);
  nlmsg_free(mesg);
  nl_socket_free(sckt);
  return (err >= 0);

nla_put_failure:
  nlmsg_free(mesg);
  nl_socket_free(sckt);
  return false;
}

bool wifi::commandhelper2::set_wifi_txpower(const std::string &device,const uint32_t tx_power_mBm){
  get_logger()->debug("set_wifi_txpower {} {} mBm",device,tx_power_mBm);
  // Create the socket and connect to it.
  struct nl_sock *sckt = nl_socket_alloc();
  genl_connect(sckt);

  // Allocate a new message.
  struct nl_msg *mesg = nlmsg_alloc();

  // Check /usr/include/linux/nl80211.h for a list of commands and attributes.
  enum nl80211_commands command = NL80211_CMD_SET_WIPHY;

  // Create the message so it will send a command to the nl80211 interface.
  genlmsg_put(mesg, 0, 0, genl_ctrl_resolve(sckt, "nl80211"), 0, 0, command, 0);

  // Add specific attributes to change the frequency of the device.
  NLA_PUT_U32(mesg, NL80211_ATTR_IFINDEX, if_nametoindex(device.c_str()));
  NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_TX_POWER_SETTING, NL80211_TX_POWER_FIXED);
  NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_TX_POWER_LEVEL, tx_power_mBm);

  // Finally send it and receive the amount of bytes sent.
  nl_send_auto_complete(sckt, mesg);

  nlmsg_free(mesg);
  nl_socket_free(sckt);
  return true;

nla_put_failure:
  nlmsg_free(mesg);
  nl_socket_free(sckt);
  return false;
}