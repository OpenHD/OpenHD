#!/bin/bash

op="$1"
IP="$2"

if [ -e "/tmp/settings.sh" ]; then
    OK=`bash -n /tmp/settings.sh`
    if [ "$?" == "0" ]; then
        source /tmp/settings.sh
    fi
fi

echo $op
#echo $mac
#echo $ip
#echo $hostname

export PATH=/usr/local/bin:${PATH}

if [ ! -f /tmp/mavlink_router_pipe ]; then 
    sleep 1
fi

if [[ $op == "old" || $op == "add" ]]; then
        echo "add $IP"  > /dev/udp/127.0.0.1/9120
	echo "add $IP"  > /dev/udp/127.0.0.1/9121
	echo "add $IP"  > /dev/udp/127.0.0.1/9122
	echo "add $IP"  > /dev/udp/127.0.0.1/9123
	echo "add $IP"  > /dev/udp/127.0.0.1/9124
	#OpenHD RemoteSettings android app
	echo "add $IP"  > /dev/udp/127.0.0.1/9125
	echo "add udp $IP $IP 14550 0" > /tmp/mavlink_router_pipe
	if [ "$QUIET" == "N" ]; then
            if [ "$ENABLE_QOPENHD" == "Y" ]; then
                qstatus "External device connected: $IP" 5
            else
                wbc_status "External device connected: $IP" 5 55 0 &
            fi
        fi
        IPTHERE=1
        while [  $IPTHERE -eq 1 ]; do
                ping -c 3 -W 1 -n -q $IP > /dev/null 2>&1
                if [ $? -eq 0 ]; then
                        IPTHERE=1
                else
                        echo "IP $IP gone. Check 2..."
                        ping -c 3 -W 1 -n -q $IP > /dev/null 2>&1
                        if [ $? -ne 0 ]; then
                                echo "IP $IP gone. Check 2 - gone."
                                IPTHERE=0
                                echo "del $IP" > /dev/udp/127.0.0.1/9120
				echo "del $IP" > /dev/udp/127.0.0.1/9121
				echo "del $IP" > /dev/udp/127.0.0.1/9122
                                echo "del $IP" > /dev/udp/127.0.0.1/9123
				echo "del $IP" > /dev/udp/127.0.0.1/9124
                        fi
                fi

        sleep 1
        done

        if [ "$QUIET" == "N" ]; then
                if [ "$ENABLE_QOPENHD" == "Y" ]; then
                    qstatus "External device gone: $IP" 5
                else
                    wbc_status "External device gone: $IP" 5 55 0 &
                fi
        fi
fi
