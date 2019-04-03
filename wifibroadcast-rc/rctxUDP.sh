#!/bin/bash

while true; do

    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`

    nice -n -5 /home/pi/wifibroadcast-rc/rctxUDP_IN $NICS
    sleep 1

done
