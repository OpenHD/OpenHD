#!/bin/bash

cd /usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`


while ! [ -e /tmp/IsTerminateRemoteSettingsPath.txt ]
do
	echo "start wfb_tx -u 9292 -p 91 -B 20 -M 0 $NICS_LIST \n"
	/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -u 9292 -p 91 -B 20 -M 0 -K /tmp/tx.key $NICS_LIST

	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
        echo "wfb_tx -u 9292 -p 91. Restating with:  $NICS_LIST \n"
        sleep 2
done

