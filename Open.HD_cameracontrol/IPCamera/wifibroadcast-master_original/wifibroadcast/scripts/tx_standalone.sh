#!/bin/bash

WLAN=$1

BAND="5G"
#BAND="2G"

CHANNEL2G="6"
CHANNEL5G="149"

ifconfig $WLAN down
iw dev $WLAN set monitor otherbss
iw reg set BO
ifconfig $WLAN up

case $BAND in
  "5G")
      echo "Setting $WLAN to channel $CHANNEL5G"
      iw dev $WLAN set bitrates ht-mcs-5 1 sgi-5
      iw dev $WLAN set channel $CHANNEL5G HT40+
      ;;
  "2G")
      echo "Setting $WLAN to channel $CHANNEL2G"
      iw dev $WLAN set bitrates ht-mcs-2.4 1 sgi-2.4
      iw dev $WLAN set channel $CHANNEL2G HT40+
      ;;
   *)
      echo "Select 2G or 5G band"
      exit -1;
      ;;
esac

./tx $WLAN
