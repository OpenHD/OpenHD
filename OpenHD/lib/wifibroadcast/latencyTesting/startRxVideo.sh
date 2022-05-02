#!/bin/bash
# Given a PC with 2 wifi cards connected that support monitor mode,
# This starts the tx on one of them and the rx on the other one

TAOBAO="wlx00e0863200b9" #Taobao card
ASUS="wlx244bfeb71c05" #ASUS card


MY_RX=$TAOBAO
#MY_RX="wlan0" #rpi testing

WFB_FOLDER="/home/consti10/Desktop/wifibroadcast"
#WFB_FOLDER="/home/pi/Desktop/wifibroadcast"

MY_WIFI_CHANNEL=149 #5ghz channel
#MY_WIFI_CHANNEL=13 #2.4ghz channel

sudo rfkill unblock wifi
#sudo killall ifplugd #stop management of interface

sudo ifconfig $MY_RX down
sudo iw dev $MY_RX set monitor otherbss fcsfail
sudo ifconfig $MY_RX up
sudo iwconfig $MY_RX channel $MY_WIFI_CHANNEL
#sudo iw dev $MY_TX set channel "6" HT40+
#sudo iwconfig $MY_TX rts off

$WFB_FOLDER/wfb_rx -u 6100 -r 60 -K $WFB_FOLDER/gs.key $MY_RX


