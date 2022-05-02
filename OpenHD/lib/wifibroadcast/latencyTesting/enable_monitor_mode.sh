#!/bin/bash
# enable monitor mode and sets the card
# to the right wifi channel (for listening OR injecting)

# wifi card is first param
MY_WIFI_CARD=$1
# wifi channel is the second param
MY_WIFI_CHANNEL=$2

sudo rfkill unblock wifi
#sudo killall ifplugd #stop management of interface

sudo ifconfig $MY_WIFI_CARD down
sudo iw dev $MY_WIFI_CARD set monitor otherbss fcsfail
sudo ifconfig $MY_WIFI_CARD up
sudo iwconfig $MY_WIFI_CARD channel $MY_WIFI_CHANNEL
#sudo iw dev $MY_TX set channel "6" HT40+
#sudo iwconfig $MY_TX rts off

echo "Monitor mode enabled on card $WIFI_CARD set to channel $MY_WIFI_CHANNEL"