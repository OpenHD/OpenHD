#!/bin/bash

LAN_INTERFACE=$1

ip link set dev ${LAN_INTERFACE} up
                
while ! [ "$(cat /sys/class/net/${LAN_INTERFACE}/operstate)" = "up" ] do
    sleep 1
done
                
if cat /sys/class/net/${LAN_INTERFACE}/carrier | nice grep -q 1; then
    qstatus "Ethernet connection detected" 5

    CARRIER=1
                    
    if dhclient eth0; then
        ETHCLIENTIP=`ip addr show ${LAN_INTERFACE} | grep -Po 'inet \K[\d.]+'`
                    
        qstatus "Ethernet IP: $ETHCLIENTIP" 5

        ping -n -q -c 1 1.1.1.1
    else
        ps -ef | nice grep "dhclient ${LAN_INTERFACE}" | nice grep -v grep | awk '{print $2}' | xargs kill -9

        ip link set dev ${LAN_INTERFACE} down
                        
        qstatus "ERROR: Could not acquire IP via DHCP!" 5
    fi
else
    qstatus "No ethernet connection detected" 5
fi
