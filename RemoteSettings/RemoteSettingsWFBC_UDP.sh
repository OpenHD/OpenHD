#!/bin/bash

/home/pi/RemoteSettings/Ground/TxForwardFromAndroid.sh  &
sleep 1
/home/pi/RemoteSettings/Ground/RxFrowardToAndroid.sh &
sleep 0.5
/home/pi/RemoteSettings/Ground/TxForwardJoystick.sh &
