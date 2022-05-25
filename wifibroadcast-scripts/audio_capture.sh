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

amixer -c 1 sset Mic $MicBoost%

while true
do

        isRestart=0
        stdbuf -oL gst-launch-1.0 alsasrc device=plughw:1,0 name=mic provide-clock=true  do-timestamp=true buffer-time=20000 ! alawenc ! rtppcmapay max-ptime=20000000 ! udpsink host=127.0.0.1 port=5051 |
        while IFS= read -r line
        do

                if [ $(echo $line  | grep -c "WARNING: from element") -eq 1 ]; then
                        echo "Restarting gst-launch";
                        ps -ef | nice grep "gst-launch-1.0 alsasrc" | nice grep -v grep | awk '{print $2}' | xargs kill -9
                        break
                fi
                echo "echo: $line"
        done

        echo "Sleeping 5 seconds before restart..."
        sleep 5
done
