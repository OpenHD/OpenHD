#!/bin/bash

if [ -e "/tmp/settings.sh" ]; then
    OK=`bash -n /tmp/settings.sh`
    if [ "$?" == "0" ]; then
        source /tmp/settings.sh
    else
        echo "ERROR: wifobroadcast config file contains syntax error(s)!"
        collect_errorlog
        sleep 365d
    fi else
    echo "ERROR: wifobroadcast config file not found!"
    collect_errorlog
    sleep 365d
fi

#IP Camera don`t have DHCP, so, fixed IP. Must be moved to better place.
if [ $SecondaryCamera == "IP" ]; then
    ifconfig eth0 192.168.0.215 netmask 255.255.0.0 up
    service ssh start
fi

/usr/local/share/RemoteSettings/Air/TxBandSwitcher.sh &
sleep 0.2
/usr/local/share/RemoteSettings/Air/RxBandSwitcher.sh &
sleep 0.2
python3 /usr/local/share/RemoteSettings/BandSwitchAir.py -DefaultBandWidthAth9k $Bandwidth &
