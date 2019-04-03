#!/bin/bash

channel=$1
WlanName=$2

while true; do

    nice -n -5 /home/pi/wifibroadcast-rc-Ath9k/rctxUDP_IN $channel $WlanName
    sleep 1

done
