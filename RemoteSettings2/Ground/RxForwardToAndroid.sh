#!/bin/bash

cd /home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

while true
do
	echo "/wfb_rx -c 127.0.0.1 -u 9292 -p 91 $NICS_LIST  (forward msg from android phone. UDP port 9292.\n"
	./wfb_rx -c 127.0.0.1 -u 9292 -p 91 $NICS_LIST


	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
	echo "/wfb_rx -c 127.0.0.1 -u 9292 -p 91 down. Restating with:  $NICS_LIST \n"
	sleep 2
done
