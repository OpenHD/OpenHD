#!/usr/bin/env bash

# this is the serial port on the jetson boards, we don't want a tty running on it
systemctl stop nvgetty || true
systemctl disable nvgetty || true


mkdir -p /media/usb

# these are intentionally written with spaces around them to avoid false negatives if people
# edit the fstab file and change the rest of the line, the way these are written it will still
# find them as long as a line for each of these mountpoints is present


# enable the loopback module if it isn't already, so that seek and flir cameras can be used
grep "v4l2loopback" /etc/modules
if [[ "$?" -ne 0 ]]; then
    echo "v4l2loopback" >> /etc/modules
fi

mount -o remount,ro /boot || true
