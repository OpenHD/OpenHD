#!/bin/bash

cd /usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

while true
do
	echo "Audio RX  /wfb_rx -c 127.0.0.1 -u 5051 -p 67 $NICS_LIST \n"
	/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_rx -c 127.0.0.1 -u 5051 -p 67 $NICS_LIST >/dev/null 2>/dev/null


	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
	echo "/wfb_rx -c 127.0.0.1 -u 5051 -p 67 Audio rx down. Restating with:  $NICS_LIST \n"
	sleep 2
done
