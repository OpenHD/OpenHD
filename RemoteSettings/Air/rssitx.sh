#!/bin/bash

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v zt* | nice grep -v eth1`


while true
do
	/usr/local/bin/rssitx -f ${FORCE_REALTEK_TELEMETRY_DATA_FRAME} $NICS_LIST

	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v zt* | nice grep -v eth1`
        sleep 2
done

