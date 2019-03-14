#!/bin/bash

echo "starting remote settings scripts on air pi\n"
/home/pi/RemoteSettings2/Air/TxRemoteSettings.sh &
#sleep 0.5
/home/pi/RemoteSettings2/Air/RxRemoteSettings.sh 
