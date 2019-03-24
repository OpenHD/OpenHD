#!/bin/bash

/home/pi/RemoteSettings2/Ground/TxForwardFromAndroid.sh  &
sleep 1
/home/pi/RemoteSettings2/Ground/RxForwardToAndroid.sh &
sleep 1
python3 /home/pi/RemoteSettings2/src_python/ServerGround.py &
