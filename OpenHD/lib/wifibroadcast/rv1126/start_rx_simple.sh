#!/bin/bash
# Given a PC with 2 wifi cards connected that support monitor mode,
# This starts the tx on one of them and the rx on the other one

TAOBAO="wlx00e0863200b9" #Taobao card
ASUS="wlx244bfeb71c05" #ASUS card


MY_RX=$TAOBAO

MY_WIFI_CHANNEL=149 #5ghz channel
#MY_WIFI_CHANNEL=13 #2.4ghz channel

WFB_FOLDER="/home/consti10/Desktop/wifibroadcast"
#WFB_FOLDER="/home/pi/Desktop/wifibroadcast"


sh ./enable_monitor_mode.sh $MY_RX $MY_WIFI_CHANNEL

xterm -hold -e $WFB_FOLDER/wfb_rx -u 5600 -r 60 $MY_RX &

$WFB_FOLDER/udp_generator_validator -u 5600 -v 1