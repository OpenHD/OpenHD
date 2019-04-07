#!/bin/bash

/sbin/iw dev $1 set freq $2
echo "Wlan $1 set to $2 MHz"
exit
