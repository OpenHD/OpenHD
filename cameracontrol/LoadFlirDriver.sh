#!/bin/bash

modprobe v4l2loopback devices=5

while true
do
    /usr/local/bin/flir8p1 /usr/local/share/flirone-driver/Rainbow.raw 
    echo "Restart FLIR reader..."
    sleep 2
done
