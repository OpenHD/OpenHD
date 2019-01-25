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

while true
do
	python3.5 /home/pi/RemoteSettings/BandSwitch.py -DefaultBandWidthAth9k $Bandwidth -PrimaryCardMAC $PrimaryCardMAC -SlaveCardMAC $SlaveCardMAC -Band5Below $Band5Below -Band10ValueMin $Band10ValueMin -Band10ValueMax $Band10ValueMax -Band20After $Band20After
        sleep 2
done
