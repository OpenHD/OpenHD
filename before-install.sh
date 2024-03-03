#!/usr/bin/env bash

# back it up in case the user has written valuable scripting that would be lost otherwise
mv /boot/openhd/scripts/custom_unmanaged_camera.sh /boot/openhd/scripts/custom_unmanaged_camera_old.sh
# NOTE: Updating overwrites the .config file and also the service file
rm -rf /boot/openhd/hardware.config

if [ "$(uname -m)" = "x86_64" ]; then
    whiptail --title "Confirmation" --yesno "Is it all right?" 10 50 && echo "Writing to /boot/yes.conf" > /boot/yes.conf || echo "Writing to /boot/no.conf" > /boot/no.conf
fi