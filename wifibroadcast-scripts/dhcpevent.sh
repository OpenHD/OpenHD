#!/bin/bash

op="${1:-op}"
mac="${2:-mac}"
IP="${3:-ip}"
hostname="${4}"

/usr/local/share/wifibroadcast-scripts/dhcpevent_thread.sh $op $IP &
