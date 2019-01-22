#!/bin/bash

NICS=`cat /tmp/NICS_LIST`
echo $NICS
PHONEIP="192.168.2.2"
SOCAT_PID=-1

function valid_ip()
{
    local  ip=$1
    local  stat=1

    if [[ $ip =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
        OIFS=$IFS
        IFS='.'
        ip=($ip)
        IFS=$OIFS
        [[ ${ip[0]} -le 255 && ${ip[1]} -le 255 \
            && ${ip[2]} -le 255 && ${ip[3]} -le 255 ]]
        stat=$?
    fi
    return $stat
}

function start_socat()
{
	echo  "(re)start socat UDP4-RECVFROM:5115,fork UDP4-SENDTO:$PHONEIP:5115 &"
	socat UDP4-RECVFROM:5115,fork UDP4-SENDTO:$PHONEIP:5115 &
	SOCAT_PID=$!
	echo "New socat pid: $SOCAT_PID"
}

function kill_socat()
{
	kill $SOCAT_PID
	echo "socat ($SOCAT_PID) killed"
}

start_socat

while :
do
	USBPHONE=`ip route show 0.0.0.0/0 dev usb0 | cut -d\  -f3`

	if valid_ip $USBPHONE; then 
		echo "$USBPHONE good"
		if [ "$PHONEIP" != "$USBPHONE" ]; then
			echo "New PhoneIP is USB IP: $USBPHONE"
			PHONEIP=$USBPHONE
			echo "PhoneIP check: $PHONEIP"
			kill_socat
			start_socat
			fi
	else
		echo  "$USBPHONE bad"
		if [  "$PHONEIP" != "192.168.2.2" ]; then
			echo "New PhoneIP is static 192.168.2.2"
			PHONEIP="192.168.2.2"
			echo "PhoneIP check: $PHONEIP"
			kill_socat
			start_socat
		fi
	fi
	echo "Send ACK packet to phone (IP: $PHONEIP)"
	echo "GroundIP" > /dev/udp/$PHONEIP/5115
	sleep 1
done


echo  "(re)start socat UDP4-RECVFROM:5115,fork UDP4-SENDTO:$PHONEIP:5115 &"
socat UDP4-RECVFROM:5115,fork UDP4-SENDTO:$PHONEIP:5115 &
SOCAT_PID=$!
echo "PID: $SOCAT_PID"
kill $SOCAT_PID
