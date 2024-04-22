//
// Created by consti10 on 12.12.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_COMMAND_HELPER_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_COMMAND_HELPER_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// NOTE:
// All those iw commands use netlink to talk to linux - we could theoretically
// switch to using netlink directly (e.g. see wifi_command_helper2) and get a
// bit more compile-time safety but I don't think that's worth it - it would be
// one more stack we need to test, and those commands used here are pretty much
// guaranteed to be available on every linux system.
namespace wifi::commandhelper {

// needed for enabling monitor mode
bool ip_link_set_card_state(const std::string& device, bool up);
// unblock all cards, also needed for enabling monitor mode
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
//		mumimo-follow-mac <MAC_ADDRESS>: use MUMIMO according to a MAC
// address
bool iw_enable_monitor_mode(const std::string& device);

// from iw documentation:
// dev <devname> set freq <freq> [NOHT|HT20|HT40+|HT40-|5MHz|10MHz|80MHz]
bool iw_set_frequency_and_channel_width(const std::string& device,
                                        uint32_t freq_mhz,
                                        uint32_t channel_width);
bool iw_set_frequency_and_channel_width2(const std::string& device,
                                         uint32_t freq_mhz,
                                         const std::string& ht_mode,
                                         bool dummy = false);

// See
// https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/nl80211.h#L1905
// NOTE: even linux seems to have no idea what mBm means - rtl8812au interprets
// that not as milli(1000)dBm, but dBm/100 from iw documentation: dev <devname>
// set txpower <auto|fixed|limit> [<tx power in mBm>]
//		Specify transmit power level and setting type.
bool iw_set_tx_power(const std::string& device, uint32_t tx_power_mBm);

// NOTE: so far, no card has been found that supports changing the mcs index in
// monitor mode via iw - rtl8812bu does it by changing the mcs index in the
// radiotap header
bool iw_set_rate_mcs(const std::string& device, uint32_t mcs_index, bool is_2g);

// https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/configuring_and_managing_networking/configuring-networkmanager-to-ignore-certain-devices_configuring-and-managing-networking
// example: nmcli device set wlx244bfeb71c05 managed no
// blacklist the card from network manager (managed=false), such that we can
// safely do our own thing, aka wifibroadcast with it or give it back to network
// manager (managed=true)
bool nmcli_set_device_managed_status(const std::string& device, bool managed);

// R.n I do not know of a better solution than this one
// runs iwlist <device> frequencies
// then checks all suplied frequencies (in mhz) and returns them (mhz) if they
// show up in the list if running the command fails, prints warning and assumes
// all supplied frequencies are supported. NOTE: "iwlist wlan1 frequency" seems
// to omit some channels - no idea why, but even though those channels are not
// shown in iwlist, they seem to work when set via "iw". We therefore have to
// use "iw phy phy0 info" -> and we use "iw" already to set the frequencies, so
// this is most likely better than "iwlist". NOTE: There seems to be no iw
// <device_name> alternative - we have to use the "phy" index
std::vector<uint32_t> iw_get_supported_frequencies(
    const int phy_index, const std::vector<uint32_t>& frequencies_mhz_to_try);

struct SupportedFrequencyBand {
  bool supports_any_2G = false;
  bool supports_any_5G = false;
};
SupportedFrequencyBand iw_get_supported_frequency_bands(
    const std::string& device);

// check if the device supports monitor mode
// Note that this does not necessarily mean the device properly does monitor
// mode with injection - quite a lot of devices report monitor mode
// capabilities, but actually either don't even do proper listen-only monitor
// mode and/or don't do injection phy <phyname> info
//		Show capabilities for the specified wireless device.
// NOTE: for phy info gives capabilities,for dev-name NOT !!
bool iw_supports_monitor_mode(int phy_index);

// Sets the channel and channel width
// REQUIRES openhd rtl8812au driver
// BUT works regardless of crda for all channels - YEAH !
// type 0= rtl8812au, 1=rtl88x2bu driver
bool openhd_driver_set_frequency_and_channel_width(int type,
                                                   const std::string& device,
                                                   uint32_t freq_mhz,
                                                   uint32_t channel_width);

// RTL8812bu driver only so far
bool openhd_driver_set_tx_power(const std::string& device,
                                uint32_t tx_power_mBm);
// RTL8812au driver only
void openhd_driver_set_tx_power_index_override(const std::string& device,
                                               uint32_t tpi);

void cleanup_openhd_driver_overrides();

}  // namespace wifi::commandhelper

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WIFI_COMMAND_HELPER_H_
