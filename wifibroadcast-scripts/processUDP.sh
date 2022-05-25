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
	echo "start processUDP -b $FC_TELEMETRY_BAUDRATE -s $FC_TELEMETRY_SERIALPORT -r 1 \n"

	/usr/local/bin/processUDP -b $FC_TELEMETRY_BAUDRATE -s $FC_TELEMETRY_SERIALPORT -r 1

        echo "Restating  processUDP -b $FC_TELEMETRY_BAUDRATE -s $FC_TELEMETRY_SERIALPORT -r 1 \n"
        sleep 2
done

