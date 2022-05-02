#!/bin/bash
# Given a PC with 2 wifi cards connected that support monitor mode,
# This starts the tx on one of them and the rx on the other one

TAOBAO="wlx00e0863200b9" #Taobao card
ASUS="wlx244bfeb71c05" #ASUS card

MY_TX=$ASUS
MY_RX=$TAOBAO

WFB_FOLDER="/home/consti10/Desktop/wifibroadcast"

FEC_K=0
FEC_PERCENTAGE=0

sudo rfkill unblock wifi
#sudo killall ifplugd #stop management of interface

sudo ifconfig $MY_TX down
sudo iw dev $MY_TX set monitor otherbss fcsfail
sudo ifconfig $MY_TX up
sudo iwconfig $MY_TX channel 6
#sudo iw dev $MY_TX set channel "6" HT40+

sudo ifconfig $MY_RX down
sudo iw dev $MY_RX set monitor otherbss fcsfail
sudo ifconfig $MY_RX up
sudo iwconfig $MY_RX channel 6
#sudo iwconfig $MY_RX channel "6" HT40+


xterm -hold -e $WFB_FOLDER/wfb_tx -k $FEC_K -p $FEC_PERCENTAGE -u 6000 -r 60 -M 7 -K $WFB_FOLDER/drone.key $MY_TX &

xterm -hold -e $WFB_FOLDER/wfb_rx  -u 6100 -r 60 -K $WFB_FOLDER/gs.key $MY_RX &

xterm -hold -e $WFB_FOLDER/wfb_tx -k $FEC_K -p $FEC_PERCENTAGE -u 6001 -r 70 -M 7 -K $WFB_FOLDER/drone.key $MY_RX &

xterm -hold -e $WFB_FOLDER/wfb_rx  -u 6101 -r 70 -K $WFB_FOLDER/gs.key $MY_TX

