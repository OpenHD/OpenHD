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


amixer -c 0 sset PCM $SpeakersLevel%


gst-launch-1.0 udpsrc port=5051 caps="application/x-rtp, media=(string)audio, clock-rate=(int)8000, encoding-name=(string)PCMA" ! rtppcmadepay ! audio/x-alaw, rate=8000, channels=1 ! alawdec ! alsasink device=hw:0
