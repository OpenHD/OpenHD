#!/bin/bash
# Given a PC with 2 wifi cards connected that support monitor mode,
# This starts the tx on one of them and the rx on the other one
# Assuming tx and rx are already set into monitor mode !

TAOBAO="wlx00e0863200b9" #Taobao card
ASUS="wlx244bfeb71c05" #ASUS card

#MY_TX="wlp3s0"
#MY_TX="wlxc4e984126183"
#MY_TX="wlx000f00460445" # ALFA card
#MY_TX="wlx6cfdb9b2a156" #PW-DN421
#MY_TX="wlx0018e7bd24db"
#MY_RX="wlx0018e7bd24db" #Ralink card aliexpress
#MY_RX="wlxc4e9840e3cbe" #tp-link rx
#MY_RX_SECONDARY="wlxc4e984126183"

MY_TX=$ASUS
MY_RX=$TAOBAO
#MY_TX="wlan0" #rpi testing
#MY_RX="wlan1" #rpi testing

WFB_FOLDER="/home/consti10/Desktop/wifibroadcast"
#WFB_FOLDER="/home/pi/Desktop/wifibroadcast"

FEC_K=8
FEC_PERCENTAGE=50

RADIO_PORT=61

MY_WIFI_CHANNEL=149 #5ghz channel

xterm -hold -e $WFB_FOLDER/wfb_tx -k $FEC_K -p $FEC_PERCENTAGE -u 6001 -r $RADIO_PORT -M 5 -B 20 -K $WFB_FOLDER/drone.key  $MY_TX &

$WFB_FOLDER/wfb_rx -c 127.0.0.1 -u 6101 -r $RADIO_PORT -K $WFB_FOLDER/gs.key $MY_RX

