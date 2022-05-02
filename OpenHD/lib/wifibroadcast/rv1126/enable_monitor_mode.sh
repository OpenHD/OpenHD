#!/bin/bash
# same as enable_monitor_mode but without sudo (aka must be running already as root)
# reason: buildroot doesn't know sudo, it is root by default

# wifi card is first param
MY_WIFI_CARD=$1
# wifi channel is the second param
MY_WIFI_CHANNEL=$2

rfkill unblock wifi
#sudo killall ifplugd #stop management of interface

ifconfig $MY_WIFI_CARD down
iw dev $MY_WIFI_CARD set monitor otherbss fcsfail
ifconfig $MY_WIFI_CARD up
iwconfig $MY_WIFI_CARD channel $MY_WIFI_CHANNEL
#sudo iw dev $MY_TX set channel "6" HT40+
#sudo iwconfig $MY_TX rts off

echo "Monitor mode enabled on card $WIFI_CARD set to channel $MY_WIFI_CHANNEL"