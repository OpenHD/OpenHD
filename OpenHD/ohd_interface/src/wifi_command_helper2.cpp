//
// Created by consti10 on 18.11.22.
//

#include "wifi_command_helper2.h"

#include <algorithm>
#include <cstdint>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#undef __USE_MISC
#include <net/if.h>

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
  int *ret = reinterpret_cast<int*>(arg);
  *ret = err->error;
  return NL_STOP;
}

bool wifi::commandhelper2::set_wifi_frequency(const std::string& device,
                                             uint32_t freq_mhz) {
  int err = 1;
  struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
  int rc;

  /* Create the socket and connect to it. */
  struct nl_sock *sckt = nl_socket_alloc();
  genl_connect(sckt);

  /* Allocate a new message. */
  struct nl_msg *mesg = nlmsg_alloc();

  /* Check /usr/include/linux/nl80211.h for a list of commands and attributes. */
  enum nl80211_commands command = NL80211_CMD_SET_WIPHY;

  /* Create the message so it will send a command to the nl80211 interface. */
  genlmsg_put(mesg, 0, 0, genl_ctrl_resolve(sckt, "nl80211"), 0, 0, command, 0);

  /* Add specific attributes to change the frequency of the device. */
  NLA_PUT_U32(mesg, NL80211_ATTR_IFINDEX, if_nametoindex(device.c_str()));
  NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_FREQ, freq_mhz);

  /* Finally send it and receive the amount of bytes sent. */
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

bool wifi::commandhelper2::set_wifi_txpower(const std::string &device, uint32_t power_mbm){
  /* Create the socket and connect to it. */
  struct nl_sock *sckt = nl_socket_alloc();
  genl_connect(sckt);

  /* Allocate a new message. */
  struct nl_msg *mesg = nlmsg_alloc();

  /* Check /usr/include/linux/nl80211.h for a list of commands and attributes. */
  enum nl80211_commands command = NL80211_CMD_SET_WIPHY;

  /* Create the message so it will send a command to the nl80211 interface. */
  genlmsg_put(mesg, 0, 0, genl_ctrl_resolve(sckt, "nl80211"), 0, 0, command, 0);

  /* Add specific attributes to change the frequency of the device. */
  NLA_PUT_U32(mesg, NL80211_ATTR_IFINDEX, if_nametoindex(device.c_str()));
  NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_TX_POWER_SETTING, NL80211_TX_POWER_FIXED);
  NLA_PUT_U32(mesg, NL80211_ATTR_WIPHY_TX_POWER_LEVEL, power_mbm);

  /* Finally send it and receive the amount of bytes sent. */
  nl_send_auto_complete(sckt, mesg);

  nlmsg_free(mesg);
  nl_socket_free(sckt);
  return true;

nla_put_failure:
  nlmsg_free(mesg);
  nl_socket_free(sckt);
  return false;
}