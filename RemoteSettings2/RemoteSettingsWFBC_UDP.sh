#!/bin/bash

/home/pi/RemoteSettings2/Ground/TxForwardFromAndroid.sh  &
sleep 1
/home/pi/RemoteSettings2/Ground/RxForwardToAndroid.sh &
