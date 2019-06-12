#!/bin/bash

ChannelBandSwitcher=$1
ChannelIPCamera=$2
IsBandSwitcherEnabled=$3
IsIPCameraSwitcherEnabled=$4
IsEncrypt=$5

WlanName=$6

while true; do

    nice -n -5 /home/pi/wifibroadcast-rc-Ath9k/rctxUDP_IN $ChannelBandSwitcher $ChannelIPCamera $IsBandSwitcherEnabled $IsIPCameraSwitcherEnabled $IsEncrypt  $WlanName
    sleep 1

done
