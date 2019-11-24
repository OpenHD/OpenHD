#!/bin/bash

modprobe v4l2loopback devices=5

cd /home/pi/Open.HD_FlirOneDrv/flir8p1-gpl/
while true
do

	/home/pi/Open.HD_FlirOneDrv/flir8p1-gpl/flir8p1 Rainbow.raw 
	echo "Restart FLIR reader..."
	sleep 2
done
