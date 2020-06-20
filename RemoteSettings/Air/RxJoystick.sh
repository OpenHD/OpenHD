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

if [ "$EncryptionOrRange" == "Encryption" ]; then
	while true
	do
		echo "strt Joystick rc rx -c 127.0.0.1 -u 5565 -p 97 $NICS_LIST \n"
		/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_rx_rc -c 127.0.0.1 -k 1 -n 1 -u 5565 -p 97 $NICS_LIST

		NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
	        echo "Joystick rc rx -c 127.0.0.1 -u 5565 -p 97 Restating with:  $NICS_LIST \n"
        	sleep 4
	done
fi
