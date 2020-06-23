#!/bin/bash

/usr/local/share/RemoteSettings/Ground/TxForwardFromAndroid.sh  &
sleep 1
/usr/local/share/RemoteSettings/Ground/RxFrowardToAndroid.sh &
sleep 0.5
/usr/local/share/RemoteSettings/Ground/TxForwardJoystick.sh &
