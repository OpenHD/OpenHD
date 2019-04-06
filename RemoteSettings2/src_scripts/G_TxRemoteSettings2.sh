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


if [ "$EncryptionOrRange" == "Range" ]; then
	MY_T_PARAM=2
else
	MY_T_PARAM=0
fi

if [ "$PrimaryCardMAC" == "0" ]; then
	MY_USED_WIFI_CARD=$NICS_LIST
else
	MY_USED_WIFI_CARD=$PrimaryCardMAC
fi

echo "./wfb_tx -u 9090 -t $MY_T_PARAM -p 90 -B 20 -M 0 -n 2 -k 1 $MY_USED_WIFI_CARD"

./wfb_tx -u 9090 -t $MY_T_PARAM -p 90 -B 20 -M 0 -n 2 -k 1 $MY_USED_WIFI_CARD
