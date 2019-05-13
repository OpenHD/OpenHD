#!/bin/bash

cd /home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`


while  [ 1 ]
do
	echo "start wfb_tx -u 5600 -t 2 -p 23 -B 20 -M 0 $NICS_LIST \n"
	./wfb_tx -u 5600 -t 2 -p 23 -B 20 -M 0 $NICS_LIST

	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
        echo "./wfb_tx -u 5600 -t 2 -p 23 -B 20 -M 0 Restating with:  $NICS_LIST \n"
        sleep 2
done

