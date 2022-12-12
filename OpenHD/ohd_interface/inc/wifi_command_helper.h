//
// Created by consti10 on 12.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_COMMAND_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_COMMAND_HELPER_H_

#include <cstdint>
#include <string>
#include <optional>
#include <vector>

// NOTE:
// All those iw commands use netlink to talk to linux
// TODO should we switch to using netlink directly instead of using the run command workaround ?
namespace wifi::commandhelper{

// needed for enabling monitor mode
bool ip_link_set_card_state(const std::string &device, bool up);

// from iw documentation:
// dev <devname> set monitor <flag>*
//		Set monitor flags. Valid flags are:
//		none:     no special flags
//		fcsfail:  show frames with FCS errors
//		control:  show control frames
//		otherbss: show frames from other BSSes
//		cook:     use cooked mode
//		active:   use active mode (ACK incoming unicast packets)
//		mumimo-groupid <GROUP_ID>: use MUMIMO according to a group id
//		mumimo-follow-mac <MAC_ADDRESS>: use MUMIMO according to a MAC address
bool iw_enable_monitor_mode(const std::string& device);

// dev <devname> set freq <freq> [NOHT|HT20|HT40+|HT40-|5MHz|10MHz|80MHz]
bool iw_set_frequency_and_channel_width(const std::string &device, uint32_t freq_mhz,bool width_40);

// See https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/nl80211.h#L1905
// NOTE: even linux seems to have no idea what mBm means - rtl8812au interprets that not as milli(1000)dBm, but dBm/100
// from iw documentation:
// dev <devname> set txpower <auto|fixed|limit> [<tx power in mBm>]
//		Specify transmit power level and setting type.
bool iw_set_tx_power(const std::string& device,uint32_t tx_power_mBm);

// https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/configuring_and_managing_networking/configuring-networkmanager-to-ignore-certain-devices_configuring-and-managing-networking
// example: nmcli device set wlx244bfeb71c05 managed no
// blacklist the card from network manager (so we can safely do our own thing, aka wifibroadcast) with it
// NOTE: this is not permament between restarts - but that is exactly what we want,
// since on each restart we might do different things with the wifi card(s)
bool nmcli_set_device_unmanaged(const std::string& device);
}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_COMMAND_HELPER_H_
