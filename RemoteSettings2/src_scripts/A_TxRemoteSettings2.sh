#!/bin/bash

echo "starting Air TX script\n"

cd /home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

echo "starting ./wfb_tx -u 5702 -p 91 -B 20 -M 0 -n 2 -k 1 $NICS_LIST\n"

./wfb_tx -u 5702 -p 91 -B 20 -M 0 -n 2 -k 1 $NICS_LIST
