#!/usr/bin/env bash

if [[ "${PACKAGE_ARCH}" != "x86" ]]; then 
#for x86 these services should be stopped at runtime

systemctl disable osd
systemctl enable openhdconfig
systemctl enable openhd_system
systemctl enable openhd_security
systemctl enable openhd_interface
systemctl enable openhd_video
systemctl enable openhd_power
systemctl enable openhd_status
systemctl enable openhd_telemetry@microservice
systemctl enable openhd_telemetry@telemetry

# this is the serial port on the jetson boards, we don't want a tty running on it
systemctl stop nvgetty || true
systemctl disable nvgetty || true

fi

mkdir -p /wbc_tmp
mkdir -p /media/usb

# these are intentionally written with spaces around them to avoid false negatives if people
# edit the fstab file and change the rest of the line, the way these are written it will still
# find them as long as a line for each of these mountpoints is present
grep " /tmp " /etc/fstab
if [[ "$?" -ne 0 ]]; then
    echo "tmpfs /tmp tmpfs nosuid,nodev,noatime,size=50M 0 0" >> /etc/fstab
fi

grep " /var/log " /etc/fstab
if [[ "$?" -ne 0 ]]; then
    echo "tmpfs /var/log tmpfs nosuid,nodev,noatime,size=50M 0 0" >> /etc/fstab
fi

grep " /var/tmp " /etc/fstab
if [[ "$?" -ne 0 ]]; then
    echo "tmpfs /var/tmp tmpfs nosuid,nodev,noatime,size=50M 0 0" >> /etc/fstab
fi


# enable the loopback module if it isn't already, so that seek and flir cameras can be used
grep "v4l2loopback" /etc/modules
if [[ "$?" -ne 0 ]]; then
    echo "v4l2loopback" >> /etc/modules
fi


mount -o remount,ro /boot || true
