//
// Created by consti10 on 17.05.22.
//

#include "WifiHotspot.h"

#include <utility>

/**
 * Create the content of a hostapd configuration file for wifi access point.
 * @return the content of the file, should be written somewhere for hostapd.
 */
static std::string createHostapdConfigFile(const std::string& interface_name){
  std::stringstream ss;
  ss<<"#OpenHD wifi hotspot hostapd config file\n";
  // the interface used by the AP
  ss<<"interface="<<interface_name<<"\n";
  // set this to "a" for 5Ghz, to "g" for 2.4Ghz
  ss<<"hw_mode=a\n";
  // Channels 1-13 for 2.4Ghz, Channels 36,40,44,48,52,56,60,64 for 5Ghz
  ss<<"channel=40\n";
  //# 802.11n support
  ss<<"ieee80211n=1\n";
  // # QoS support, also required for full speed on 802.11n/ac/ax
  ss<<"wmm_enabled=1\n";
  //# the name of the AP
  ss<<"ssid=OpenHD\n";
  // # 1=wpa, 2=wep, 3=both
  ss<<"auth_algs=1\n";
  ss<<"\n";
  // # WPA2 only
  ss<<"wpa=2\n";
  ss<<"wpa_key_mgmt=WPA-PSK\n"
	  "rsn_pairwise=CCMP\n";
  // the wifi password
  ss<<"wpa_passphrase=wifiopenhd\n";
  ss<<"\n";

  // Now this stuff comes from someone (from the apconfig.txt in the old releases),
  // I think they are optimizations for our specific use case. They are completely optional though.
  /*ss<<"# Optimizations begin -----------------------\n";
  ss<<"supported_rates=240 360\n"
	  "basic_rates=240 360\n"
	  "\n"
	  "disassoc_low_ack=0\n"
	  "ap_max_inactivity=3600\n"
	  "\n"
	  "# Low priority / AC_BK = background\n"
	  "tx_queue_data3_aifs=1\n"
	  "tx_queue_data3_cwmin=3\n"
	  "tx_queue_data3_cwmax=7\n"
	  "tx_queue_data3_burst=0\n"
	  "# Note: for IEEE 802.11b mode: cWmin=31 cWmax=1023 burst=0\n"
	  "#\n"
	  "# Normal priority / AC_BE = best effort\n"
	  "tx_queue_data2_aifs=1\n"
	  "tx_queue_data2_cwmin=3\n"
	  "tx_queue_data2_cwmax=7\n"
	  "tx_queue_data2_burst=0\n"
	  "# Note: for IEEE 802.11b mode: cWmin=31 cWmax=127 burst=0\n"
	  "#\n"
	  "# High priority / AC_VI = video\n"
	  "tx_queue_data1_aifs=1\n"
	  "tx_queue_data1_cwmin=3\n"
	  "tx_queue_data1_cwmax=7\n"
	  "tx_queue_data1_burst=0\n"
	  "# Note: for IEEE 802.11b mode: cWmin=15 cWmax=31 burst=6.0\n"
	  "#\n"
	  "# Highest priority / AC_VO = voice\n"
	  "tx_queue_data0_aifs=1\n"
	  "tx_queue_data0_cwmin=3\n"
	  "tx_queue_data0_cwmax=7\n"
	  "tx_queue_data0_burst=0\n"
	  "# Note: for IEEE 802.11b mode: cWmin=7 cWmax=15 burst=3.3\n"
	  "\n"
	  "\n"
	  "#\n"
	  "# WMM-PS Unscheduled Automatic Power Save Delivery [U-APSD]\n"
	  "# Enable this flag if U-APSD supported outside hostapd (eg., Firmware/driver)\n"
	  "#uapsd_advertisement_enabled=1\n"
	  "#\n"
	  "# Low priority / AC_BK = background\n"
	  "wmm_ac_bk_cwmin=2\n"
	  "wmm_ac_bk_cwmax=2\n"
	  "wmm_ac_bk_aifs=3\n"
	  "wmm_ac_bk_txop_limit=47\n"
	  "wmm_ac_bk_acm=0\n"
	  "# Note: for IEEE 802.11b mode: cWmin=5 cWmax=10\n"
	  "#\n"
	  "# Normal priority / AC_BE = best effort\n"
	  "wmm_ac_be_aifs=2\n"
	  "wmm_ac_be_cwmin=2\n"
	  "wmm_ac_be_cwmax=3\n"
	  "wmm_ac_be_txop_limit=47\n"
	  "wmm_ac_be_acm=0\n"
	  "# Note: for IEEE 802.11b mode: cWmin=5 cWmax=7\n"
	  "#\n"
	  "# High priority / AC_VI = video\n"
	  "wmm_ac_vi_aifs=2\n"
	  "wmm_ac_vi_cwmin=2\n"
	  "wmm_ac_vi_cwmax=3\n"
	  "wmm_ac_vi_txop_limit=47\n"
	  "wmm_ac_vi_acm=0\n"
	  "# Note: for IEEE 802.11b mode: cWmin=4 cWmax=5 txop_limit=188\n"
	  "#\n"
	  "# Highest priority / AC_VO = voice\n"
	  "wmm_ac_vo_aifs=2\n"
	  "wmm_ac_vo_cwmin=2\n"
	  "wmm_ac_vo_cwmax=3\n"
	  "wmm_ac_vo_txop_limit=47\n"
	  "wmm_ac_vo_acm=0\n"
	  "# Note: for IEEE 802.11b mode: cWmin=3 cWmax=4 burst=102\n";
  ss<<"# Optimizations end -----------------------\n";*/
  return ss.str();
}


WifiHotspot::WifiHotspot(WiFiCard wifiCard):
wifiCard(std::move(wifiCard)) {
}

void WifiHotspot::start() {
  std::cout<<"Starting WIFI hotspot on card:"<<wifiCard.interface_name<<"\n";
  // first, we create the content for the config file
  m_hostapd_config_file_content=createHostapdConfigFile(wifiCard.interface_name);
  // then we write it out to /tmp
  std::ofstream _config("/tmp/hostapd.conf");
  _config << m_hostapd_config_file_content;
  _config.close();
  // disable hostapd if it is running
  OHDUtil::run_command("systemctl",{"disable hostapd"});
  // then start hostapd with the created config file. Now the wifi AP is running.
  // -B means run in the background.
  OHDUtil::run_command("hostapd",{"-B -d /tmp/hostapd.conf"});
  // and we re-start the hostapd service
  OHDUtil::run_command("systemctl",{"enable hostapd"});
  // Configure the detected USB tether device (not sure if needed)
  OHDUtil::run_command("dhclient",{wifiCard.interface_name});
}

void WifiHotspot::stop() {
  OHDUtil::run_command("systemctl",{"disable hostapd"});
}
