#!/bin/bash
# This script enables monitor mode on 2 wifi cards (both connected to the same pc) and
# starts injecting generated packets on the tx card. At the same time, the packets received
# on the rx are validated. This is a simple test to make sure that injecting and receiving packets works,
# and that the received packets have the right content.

TAOBAO="wlx00e0863200b9" #Taobao card
ASUS="wlx244bfeb71c05" #ASUS card

MY_RX=$TAOBAO
MY_TX=$ASUS

MY_WIFI_CHANNEL=149 #5ghz channel
#MY_WIFI_CHANNEL=13 #2.4ghz channel

WFB_FOLDER="/home/consti10/Desktop/wifibroadcast"
#WFB_FOLDER="/home/pi/Desktop/wifibroadcast"

# enable monitor mode on rx card, start wfb_rx
sh ./enable_monitor_mode.sh $MY_RX $MY_WIFI_CHANNEL

xterm -hold -e $WFB_FOLDER/wfb_rx -u 6200 -r 60 $MY_RX &


# enable monitor mode on tx card, start wfb_tx
sh ./enable_monitor_mode.sh $MY_TX $MY_WIFI_CHANNEL

xterm -hold -e $WFB_FOLDER/wfb_tx -u 6000 -r 60 -M 5 -B 20 $MY_TX &



# start the generator
xterm -hold -e $WFB_FOLDER/udp_generator_validator -u 6000 &

# validate incoming packets
$WFB_FOLDER/udp_generator_validator -u 6200 -v 1