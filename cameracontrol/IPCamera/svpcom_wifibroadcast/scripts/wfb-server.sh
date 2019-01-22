#!/bin/bash
set -e

if [ $# -lt 2 ]
then echo "Usage: $0 <profile> <wlan1> [wlan2] ..."
     exit 1
fi

PROFILE=$1
shift 1
WLANS=$@
CHANNEL5G="149" # Freq: 5805 (5795–5815)  BW 40 MHz

for WLAN in $WLANS
do
echo "Setting $WLAN to channel $CHANNEL5G"
ifconfig $WLAN down
iw dev $WLAN set monitor otherbss
iw reg set BO
ifconfig $WLAN up
iw dev $WLAN set channel $CHANNEL5G HT40+
done

exec python -m telemetry.server $PROFILE $WLANS
