#!/bin/bash

while [ ! -f /tmp/ReadyToGo ]
do
	echo "test" > /dev/udp/127.0.0.1/1376
        sleep 1
done
exit
