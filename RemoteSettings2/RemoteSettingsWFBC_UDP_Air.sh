#!/bin/bash

echo "starting remote settings scripts on air pi\n"
/home/pi/RemoteSettings2/Air/TxRemoteSettings.sh &
#sleep 0.5
/home/pi/RemoteSettings2/Air/RxRemoteSettings.sh &
#sleep 1
python3 /home/pi/RemoteSettings2/src_python/ServerAir.py
