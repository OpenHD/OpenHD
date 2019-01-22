#!/bin/bash

#!/bin/bash

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


cd /home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

if [ "$EncryptionOrRange" == "Encryption" ]; then
	while true
	do
		echo "start joystick forward wfb_tx -k 1 -n 1 -u 5565 -p 97 -B 20 -M 0 \n"
		./wfb_tx -k 1 -n 1 -u 5565 -p 97 -B 20 -M 0 $NICS_LIST


		NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
		echo "start wfb_tx -k 1 -n 1 -u 5565 -p 97 -B 20 -M 0 down. Restating with:  $NICS_LIST \n"
		sleep 2
	done
fi
