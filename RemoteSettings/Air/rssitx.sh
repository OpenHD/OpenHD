#!/bin/bash

cd /home/pi/wifibroadcast-base

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v $NIC_BLACKLIST | nice grep -v eth1`


while true
do
	./rssitx $NICS_LIST

	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v $NIC_BLACKLIST | nice grep -v eth1`
        sleep 2
done

