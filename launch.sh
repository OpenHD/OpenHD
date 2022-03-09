#!/usr/bin/env bash

systemctl disable osd
systemctl disable openhd_system
systemctl disable openhd_security
systemctl disable openhd_interface
systemctl disable openhd_video
systemctl disable openhd_power
systemctl disable openhd_status
systemctl disable openhd_telemetry@microservice
systemctl disable openhd_telemetry@telemetry
systemctl disable openhd_settings_m

systemctl stop NetworkManager.service \
systemctl stop avahi-daemon.service \
systemctl stop dhcpcd.service \
systemctl stop dnsmasq.service \
systemctl stop ser2net.service \
systemctl stop hciuart.service \
# systemctl stop exim4.service
systemctl stop hostapd.service \
systemctl stop wpa_supplicant.service \
systemctl stop openhd_system \
systemctl stop openhd_security \
systemctl stop openhd_interface \
systemctl stop openhd_video \
systemctl stop openhd_power \
systemctl stop openhd_status \
systemctl stop openhd_telemetry@microservice \
systemctl stop openhd_telemetry@telemetry \
systemctl stop openhd_settings_m

#airmon-ng check kill
#airmon-ng start enxecf00e67784a

systemctl start openhd_system
systemctl start openhd_security
systemctl start openhd_interface
systemctl start openhd_video
systemctl start openhd_power
systemctl start openhd_status
systemctl start openhd_telemetry@microservice
systemctl start openhd_telemetry@telemetry
systemctl start openhd_settings_m

echo "nameserver 1.1.1.1" > /etc/resolv.conf

