//
// Created by consti10 on 17.05.22.
//

#include "WifiHotspot.h"

#include <utility>


// From https://www.raspberryconnect.com/projects/65-raspberrypi-hotspot-accesspoints/157-raspberry-pi-auto-wifi-hotspot-switch-internet,
// Works on raspberry pi 4
static std::string createHostapdConfigFile(const std::string& interface_name){
  std::stringstream ss;
  ss<<"#2.4GHz setup wifi 80211 b,g,n\n";
  ss<<"interface="<<interface_name<<"\n";
  ss<<"driver=nl80211\n"
        "ssid=OpenHD\n"
        "hw_mode=g\n"
        "channel=8\n"
        "wmm_enabled=0\n"
        "macaddr_acl=0\n"
        "auth_algs=1\n"
        "ignore_broadcast_ssid=0\n"
        "wpa=2\n"
        "wpa_passphrase=openhdopenhd\n"
        "wpa_key_mgmt=WPA-PSK\n"
        "wpa_pairwise=CCMP TKIP\n"
        "rsn_pairwise=CCMP\n"
        "\n"
        "#80211n - Change GB to your WiFi country code\n"
        "country_code=GB\n"
        "ieee80211n=1\n"
        "ieee80211d=1\n";
  return ss.str();
}

// From https://www.raspberryconnect.com/projects/65-raspberrypi-hotspot-accesspoints/157-raspberry-pi-auto-wifi-hotspot-switch-internet,
// Works on raspberry pi 4
static std::string createDnsmasqConfFile(const std::string& interface_name){
  std::stringstream ss;
  ss<<"#AutoHotspot config\n";
  ss<<"interface="<<interface_name<<"\n";
  ss<<"bind-dynamic \n"
        "server=8.8.8.8\n"
        "domain-needed\n"
        "bogus-priv\n"
        "dhcp-range=192.168.50.150,192.168.50.200,12h\n";
  return ss.str();
}

WifiHotspot::WifiHotspot(WiFiCard wifiCard):
wifiCard(std::move(wifiCard)) {
}

void WifiHotspot::start() {
  std::cerr<<"Starting WIFI hotspot on card:"<<wifiCard.interface_name<<"\n";

  // First, we make sure the pi gives out IP addresses on the hotspot interface
  // First, cleanup already existing
  OHDUtil::run_command("systemctl",{"unmask dnsmasq"});
  OHDUtil::run_command("systemctl",{"disable dnsmasq"});
  OHDUtil::run_command("systemctl",{"stop dnsmasq"});
  // write file
  {
    const auto dnsmasq_conf_content=createDnsmasqConfFile(wifiCard.interface_name);
    std::ofstream  dnsmasq_conf_file("/etc/dnsmasq.conf");
    dnsmasq_conf_file << dnsmasq_conf_content;
    dnsmasq_conf_file.close();
  }
  // start
  OHDUtil::run_command("systemctl",{"start dnsmasq"});


  // disable hostapd if it is running
  OHDUtil::run_command("systemctl",{"unmask hostapd"});
  OHDUtil::run_command("systemctl",{"disable hostapd"});
  OHDUtil::run_command("systemctl",{"stop hostapd"});
  {
	// first, we create the content for the config file
	const auto hostapd_conf_content=createHostapdConfigFile(wifiCard.interface_name);
	// then we write it out to a file
	//std::ofstream _config("/tmp/hostapd.conf");
	std::ofstream hostapd_conf("/etc/hostapd/hostapd.conf");
	hostapd_conf << hostapd_conf_content;
	hostapd_conf.close();
  }
  // then start hostapd again
  //OHDUtil::run_command("systemctl",{"enable hostapd"});
  OHDUtil::run_command("systemctl",{"start hostapd"});

  // disable hostapd if it is running
  //OHDUtil::run_command("systemctl",{"disable hostapd"});
  // then start hostapd with the created config file. Now the wifi AP is running.
  // -B means run in the background.
  //OHDUtil::run_command("hostapd",{"-B -d /tmp/hostapd.conf"});
  // and we re-start the hostapd service
  //OHDUtil::run_command("systemctl",{"enable hostapd"});
  // Configure the detected USB tether device (not sure if needed)
  //OHDUtil::run_command("dhclient",{wifiCard.interface_name});

  // now we need to start a dhcp server -similar to before
  /*const auto dhcpd_conf_content=createDHClientConfigFile(wifiCard.interface_name);
  std::ofstream  dhcpd_conf("/etc/dhcp/dhcpd.conf");
  dhcpd_conf << dhcpd_conf_content;
  dhcpd_conf.close();*/

  std::cerr<<"Wifi hotspot started\n";
}

void WifiHotspot::stop() {
  OHDUtil::run_command("systemctl",{"disable hostapd"});
  OHDUtil::run_command("systemctl",{"stop hostapd"});

  OHDUtil::run_command("systemctl",{"disable dnsmasq"});
  OHDUtil::run_command("systemctl",{"stop dnsmasq"});
  std::cout<<"Wifi hotspot stopped\n";
}


// OLD crap
/**
 * Create the content of a hostapd configuration file for wifi access point.
 * @return the content of the file, should be written somewhere for hostapd.
 */
/*static std::string createHostapdConfigFile(const std::string& interface_name){
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
  ss<<"\n";*/

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
/*return ss.str();
}*/
/*static std::string createDHClientConfigFile(const std::string& interface_name){
  std::stringstream ss;
  // from https://gist.github.com/Semant1ka/ee087c2bd1fbf6b0287c3307b8d4f291
  ss<<"ddns-update-style none;\n";
  ss<<"authoritative;\n";
  ss<<"subnet 192.168.0.0 netmask 255.255.255.0 {\n";
  ss<<"	interface "<<interface_name<<";\n";
  //ss<<"	interface wlan0; # your interface name here\n";
  ss<<"	range 192.168.0.5 192.168.0.8; # desired ip range\n";
  ss<<"	option routers 192.168.0.1;\n";
  ss<<"	option subnet-mask 255.255.255.0;\n";
  ss<<"	option broadcast-address 192.168.0.255;\n";
  ss<<"}\n";
  return ss.str();
}*/