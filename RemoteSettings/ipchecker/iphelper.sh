#!/bin/bash

while :
do
	#echo "Send ACK packet to phone (IP: $PHONEIP)"
	echo "GroundIP" > /dev/udp/127.0.0.1/5116
	
	sleep 1
done
