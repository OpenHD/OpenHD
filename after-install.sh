#!/usr/bin/env bash

systemctl disable osd
systemctl enable openhdconfig

mkdir -p /wbc_tmp

# crude hack to avoid making people put fonts somewhere else
cp -a /usr/local/share/openhd/osdfonts/*.ttf /boot/osdfonts/
