#!/bin/bash

source /usr/local/share/wifibroadcast-scripts/global_functions.sh

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

detect_os

migration_helper

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_rx -u 5612 -p 23 -c 127.0.0.1 -n $VIDEO_BLOCKS_SECONDARY -k $VIDEO_FECS_SECONDARY -K /tmp/rx.key $NICS_LIST >/dev/null 2>/dev/null &

gst-launch-1.0 udpsrc port=5612 ! tee name=t !  queue ! udpsink host=127.0.0.1 port=5621 t. ! udpsink host=127.0.0.1 port=${VIDEO_UDP_PORT2}
