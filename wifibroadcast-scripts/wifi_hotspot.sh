#!/bin/bash

HOTSPOT_BAND=$1
HOTSPOT_CHANNEL=$2
HOTSPOT_INTERFACE=$3
HOTSPOT_TXPOWER=$4

#
# Convert hostap config from DOS format to UNIX format
#
dos2unix -n /boot/apconfig.txt /tmp/apconfig.txt

#
# Read the hotspot configuration file and replace the band/channel
#
source /tmp/apconfig.txt

sed -i -e "s/hw_mode=$hw_mode/hw_mode=$HOTSPOT_BAND/g" /tmp/apconfig.txt
sed -i -e "s/channel=$channel/channel=$HOTSPOT_CHANNEL/g" /tmp/apconfig.txt
sed -i -e "s/interface=wifihotspot0/interface=${HOTSPOT_INTERFACE}/g" /tmp/apconfig.txt


sed -i -e "s/interface=wifihotspot0/interface=${HOTSPOT_INTERFACE}/g" /etc/dnsmasqWifi.conf
sed -i -e "s/dhcp-range=wifihotspot0/dhcp-range==${HOTSPOT_INTERFACE}/g" /etc/dnsmasqWifi.conf


#
# Start a DHCP server and then configure access point management
#
/usr/sbin/dnsmasq --conf-file=/etc/dnsmasqWifi.conf
hostapd -B -d /tmp/apconfig.txt

iw dev wifihotspot0 set txpower fixed ${HOTSPOT_TXPOWER}
