#!/bin/bash

cd /usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/

if [ -e "/tmp/settings.sh" ]; then
    OK=`bash -n /tmp/settings.sh`
    if [ "$?" == "0" ]; then
        source /tmp/settings.sh
    else
        echo "ERROR: wifobroadcast config file contains syntax error(s)!"
        collect_errorlog
        sleep 365d
    fi else
    echo "ERROR: wifobroadcast config file not found!"
    collect_errorlog
    sleep 365d
fi



NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`


while true
do
	echo "start wfb_tx -u 9090 -p 90 $NICS_LIST  (forward msg from android phone. UDP port 9090.\n"

	if [ "$PrimaryCardMAC" == "0" ]; then
		if [ "$EncryptionOrRange" == "Range" ]; then
			/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -u 9090 -t 2 -p 90 -B 20 -M 0 $NICS_LIST
    		fi
		
		if [ "$EncryptionOrRange" == "Encryption" ]; then
            		/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -u 9090 -t 0 -p 90 -B 20 -M 0 $NICS_LIST
    		fi
	else
		if [ "$EncryptionOrRange" == "Range" ]; then
			/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -u 9090 -t 2 -p 90 -B 20 -M 0 $PrimaryCardMAC
    		fi
		
		if [ "$EncryptionOrRange" == "Encryption" ]; then
            		/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -u 9090 -t 0 -p 90 -B 20 -M 0 $PrimaryCardMAC
    		fi	
	fi

	echo "start wfb_tx -u 9090 -p 90 down. Restating...  $PrimaryCardMAC \n"
	sleep 2
	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

done
