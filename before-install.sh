#!/usr/bin/env bash

# back it up in case the user has written valuable scripting that would be lost otherwise
mv /boot/openhd/scripts/custom_unmanaged_camera.sh /boot/openhd/scripts/custom_unmanaged_camera_old.sh
# NOTE: Updating overwrites the .config file and also the service file
rm -rf /boot/openhd/hardware.config

if [ "$(uname -m)" != "x86_64" ]; then

    whiptail --title "OpenHD" --yesno "You are about to install OpenHD to your Computer. Please be aware that we do not allow military usage! Do you want to continue ?" 10 50

    if [ $? -eq 0 ]; then
        echo "Installing OpenHD..."
    else
        exit 0
    fi
fi