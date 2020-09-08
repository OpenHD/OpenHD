#!/usr/bin/env bash

systemctl disable osd
systemctl enable openhdconfig
systemctl enable openhd_security

mkdir -p /wbc_tmp
mkdir -p /media/usb

# crude hack to avoid making people put fonts somewhere else
cp -a /usr/local/share/openhd/osdfonts/*.ttf /boot/osdfonts/ > /dev/null 2>&1 || true

# these are intentionally written with spaces around them to avoid false negatives if people
# edit the fstab file and change the rest of the line, the way these are written it will still
# find them as long as a line for each of these mountpoints is present
grep " /tmp " /etc/fstab
if [[ "$?" -ne 0 ]]; then
    echo "tmpfs /tmp tmpfs nosuid,nodev,noatime,size=10M 0 0" >> /etc/fstab
fi

grep " /var/log " /etc/fstab
if [[ "$?" -ne 0 ]]; then
    echo "tmpfs /var/log tmpfs nosuid,nodev,noatime,size=10M 0 0" >> /etc/fstab
fi

grep " /var/tmp " /etc/fstab
if [[ "$?" -ne 0 ]]; then
    echo "tmpfs /var/tmp tmpfs nosuid,nodev,noatime,size=10M 0 0" >> /etc/fstab
fi


mount -oremount,ro /boot || true
