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

while true
do
	python3 /usr/local/share/RemoteSettings/BandSwitch.py -PrimaryCardMAC $PrimaryCardMAC -Band5Below $Band5Below -Band10ValueMin $Band10ValueMin -Band10ValueMax $Band10ValueMax -Band20After $Band20After -Camera1ValueMin $Camera1ValueMin -Camera1ValueMax $Camera1ValueMax -Camera2ValueMin $Camera2ValueMin -Camera2ValueMax $Camera2ValueMax -Camera3ValueMin $Camera3ValueMin -Camera3ValueMax $Camera3ValueMax -Camera4ValueMin $Camera4ValueMin -Camera4ValueMax $Camera4ValueMax
        sleep 2
done
