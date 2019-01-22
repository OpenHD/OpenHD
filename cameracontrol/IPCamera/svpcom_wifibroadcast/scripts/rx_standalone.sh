#!/bin/bash

WLANS=$@
BAND="5G"
#BAND="2G"

CHANNEL2G="6"
CHANNEL5G="149"

for WLAN in $WLANS
do
ifconfig $WLAN down
iw dev $WLAN set monitor otherbss
iw reg set BO
ifconfig $WLAN up

case $BAND in
  "5G")
      echo "Setting $WLAN to channel $CHANNEL5G"
      iw dev $WLAN set channel $CHANNEL5G HT40+
      ;;
  "2G")
      echo "Setting $WLAN to channel $CHANNEL2G"
      iw dev $WLAN set channel $CHANNEL2G HT40+
      ;;
   *)
      echo "Select 2G or 5G band"
      exit -1;
      ;;
esac
done

# No UI, video only
./wfb_rx -p 1 -u 5600 -K gs.key $WLANS
