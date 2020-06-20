#!/bin/bash

if [ -e "/tmp/settings.sh" ]; then
    OK=`bash -n /tmp/settings.sh`
    if [ "$?" == "0" ]; then
        source /tmp/settings.sh
    else
        echo "ERROR: openhd settings file contains syntax error(s)!"
        collect_errorlog
        sleep 365d
    fi else
    echo "ERROR: openhd settings file not found!"
    collect_errorlog
    sleep 365d
fi


/usr/local/share/RemoteSettings/Air/TxRemoteSettings.sh &
#sleep 0.5
/usr/local/share/RemoteSettings/Air/RxRemoteSettings.sh &

