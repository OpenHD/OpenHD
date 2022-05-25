#!/bin/bash

HOTSPOT_INTERFACE=$1
HOTSPOT_IP=$2

# todo: pass in the address and dhcp range so it can be set or changed automatically
sed -i -e "s/interface=eth0/interface=${HOTSPOT_INTERFACE}/g" /etc/dnsmasqEth0.conf
sed -i -e "s/dhcp-range=eth0/dhcp-range=${HOTSPOT_INTERFACE}/g" /etc/dnsmasqEth0.conf
sed -i -e "s/listen-address=192.168.1.1/listen-address=${HOTSPOT_IP}/g" /etc/dnsmasqEth0.conf

HOTSPOT_BASE=$(echo "${HOTSPOT_IP}" | awk -F'.' '{print $1,$2,$3}' OFS='.')

sed -i -e "s/dhcp-range=eth0,192.168.1.2,192.168.1.50,2m/dhcp-range=${HOTSPOT_INTERFACE},${HOTSPOT_IP},${HOTSPOT_BASE}.50/g" /etc/dnsmasqEth0.conf


ifconfig ${HOTSPOT_INTERFACE} ${HOTSPOT_IP} up
/usr/sbin/dnsmasq --conf-file=/etc/dnsmasqEth0.conf

