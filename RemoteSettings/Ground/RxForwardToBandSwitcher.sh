#!/bin/bash

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

while true
do
	echo "/usr/local/bin/wfb_rx -k 1 -n 1 -c 127.0.0.1 -u 8943 -p 59 $NICS_LIST  BandSwitcher in\n"
	/usr/local/bin/wfb_rx -k 1 -n 1 -c 127.0.0.1 -u 8943 -p 44 -K /tmp/rx.key $NICS_LIST >/dev/null 2>/dev/null


	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
	echo "/usr/local/bin/wfb_rx -k 1 -n 1 -c 127.0.0.1 -u 8943 -p 59 down. Restating with:  $NICS_LIST \n"
	sleep 2
done
