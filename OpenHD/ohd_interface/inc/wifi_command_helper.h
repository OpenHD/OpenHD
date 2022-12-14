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
bool rfkill_unblock_all();

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
bool iw_set_frequency_and_channel_width(const std::string &device, uint32_t freq_mhz,uint32_t channel_width);

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

// R.n I do not know of a better solution than this one
// runs iwlist <device> frequencies
// then checks all suplied frequencies (in mhz) and returns them (mhz) if they show up in the list
// if running the command fails, prints warning and assumes all supplied frequencies are supported.
std::vector<uint32_t> iw_get_supported_frequencies(const std::string& device,const std::vector<uint32_t>& frequencies_mhz_to_try);

// check if the device supports monitor mode
// Note that this does not necessarily mean the device properly does monitor mode with injection - quite a lot of devices
// report monitor mode capabilities, but actually either don't even do proper listen-only monitor mode and/or don't do injection
// phy <phyname> info
//		Show capabilities for the specified wireless device.
// NOTE: for phy info gives capabilities,for dev-name NOT !!
bool iw_supports_monitor_mode(int phy_index);

}

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_COMMAND_HELPER_H_
