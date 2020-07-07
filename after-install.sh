#!/usr/bin/env bash

systemctl disable osd
systemctl enable openhdconfig

mkdir -p /wbc_tmp
mkdir -p /media/usb

# crude hack to avoid making people put fonts somewhere else
cp -a /usr/local/share/openhd/osdfonts/*.ttf /boot/osdfonts/ > /dev/null 2>&1 || true

mount -oremount,ro /boot || true
