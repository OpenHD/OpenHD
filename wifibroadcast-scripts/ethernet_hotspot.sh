#!/bin/bash

HOTSPOT_INTERFACE=$1

# todo: pass in the address and dhcp range so it can be set or changed automatically
sed -i -e "s/interface=eth0/interface=${HOTSPOT_INTERFACE}/g" /etc/dnsmasqEth0.conf
sed -i -e "s/dhcp-range=eth0/dhcp-range=${HOTSPOT_INTERFACE}/g" /etc/dnsmasqEth0.conf


ifconfig ${HOTSPOT_INTERFACE} 192.168.1.1 up
/usr/sbin/dnsmasq --conf-file=/etc/dnsmasqEth0.conf

