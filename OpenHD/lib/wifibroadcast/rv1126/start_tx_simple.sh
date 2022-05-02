#!/bin/bash
# Given a PC with 2 wifi cards connected that support monitor mode,
# This starts the tx on one of them and the rx on the other one

TAOBAO="wlx00e0863200b9" #Taobao card
ASUS="wlx244bfeb71c05" #ASUS card
RV1126="wlan0"

#MY_TX=$TAOBAO
MY_TX=$RV1126

MY_WIFI_CHANNEL=149 #5ghz channel
#MY_WIFI_CHANNEL=13 #2.4ghz channel

FEC_K=1

sh ./enable_monitor_mode.sh $MY_TX $MY_WIFI_CHANNEL

/oem/usr/bin/wfb_tx -u 5600 -r 60 -M 5 -B 20 -k $FEC_K $MY_TX
