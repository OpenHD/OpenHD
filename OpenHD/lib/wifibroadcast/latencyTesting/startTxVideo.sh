#!/bin/bash
# Given a PC with 2 wifi cards connected that support monitor mode,
# This starts the tx on one of them and the rx on the other one

TAOBAO="wlx00e0863200b9" #Taobao card
ASUS="wlx244bfeb71c05" #ASUS card


MY_TX=$TAOBAO
#MY_TX="wlan0" #rpi testing

WFB_FOLDER="/home/consti10/Desktop/wifibroadcast"
#WFB_FOLDER="/home/pi/Desktop/wifibroadcast"

FEC_K="h264"
FEC_PERCENTAGE=50

MY_WIFI_CHANNEL=149 #5ghz channel
#MY_WIFI_CHANNEL=13 #2.4ghz channel

sudo rfkill unblock wifi
#sudo killall ifplugd #stop management of interface

sudo ifconfig $MY_TX down
sudo iw dev $MY_TX set monitor otherbss fcsfail
sudo ifconfig $MY_TX up
sudo iwconfig $MY_TX channel $MY_WIFI_CHANNEL
#sudo iw dev $MY_TX set channel "6" HT40+
#sudo iwconfig $MY_TX rts off

$WFB_FOLDER/wfb_tx -k $FEC_K -p $FEC_PERCENTAGE -u 6000 -r 60 -M 5 -B 20 -K $WFB_FOLDER/drone.key  $MY_TX


