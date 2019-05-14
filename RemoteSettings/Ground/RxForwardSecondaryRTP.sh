#!/bin/bash

ps -ef | nice grep "cat /root/videofifo2" | nice grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | nice grep "gst-launch-1.0 fdsrc" | nice grep -v grep | awk '{print $2}' | xargs kill -9

ps -ef | nice grep "wfb_rx -u 5600 -p 23 -c" | nice grep -v grep | awk '{print $2}' | xargs kill -9

cd /home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

./wfb_rx -u 5600 -p 23 -c 192.168.2.2 $NICS_LIST


