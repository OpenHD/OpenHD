#!/bin/bash

echo "starting RX script\n"

cd /home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

echo "starting ./wfb_rx -c 127.0.0.1 -u 5701 -p 90 -n 2 -k 1 $NICS_LIST"

./wfb_rx -c 127.0.0.1 -u 5701 -p 90 -n 2 -k 1 $NICS_LIST

