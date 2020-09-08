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
	echo "start /wfb_tx -u 8943 -t 2 -p 59 -B 20 -M 0 -K /tmp/tx.key $NICS_LIST (BandSwitcher\n"

	/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx  -k 1 -n 1 -u 8943 -p 44 -B 20 -M 0 -K /tmp/tx.key $NICS_LIST

	NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`
	echo "start /wfb_tx -u 8943  -k 1 -n 1 -t 2 -p 59 -B 20 -M 0 down. Restating with:  $NICS_LIST \n"
	sleep 2
done
