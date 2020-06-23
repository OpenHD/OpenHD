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
		if [ "$Lora" == "1" ]; then
			/usr/local/bin/lora /dev/ttyUSB0
		else

			if [ "$PrimaryCardMAC" == "0" ]; then
				echo "start joystick forward wfb_tx -k 1 -n 1 -u 5566 -p 97 -B 20 -M 0 $NICS_LIST \n"
				/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -k 1 -n 1 -u 5566 -p 97 -B 20 -M 0 $NICS_LIST
			else
				echo "start joystick forward wfb_tx -k 1 -n 1 -u 5565 -p 97 -B 20 -M 0 \n"
				/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -k 1 -n 1 -u 5566 -p 97 -B 20 -M 0 $PrimaryCardMAC
			fi

			echo "start wfb_tx -k 1 -n 1 -u 5566 -p 97 -B 20 -M 0 down. Restating... \n"
			NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
		fi
		sleep 2
	done
fi
