#!/bin/bash

while :
do
	echo "RSSI" > /dev/udp/127.0.0.1/9090
	sleep 1
done
