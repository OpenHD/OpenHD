#!/bin/bash

op="${1:-op}"
mac="${2:-mac}"
IP="${3:-ip}"
hostname="${4}"

/usr/local/share/RemoteSettings/dhcpeventThread.sh $op $IP &
