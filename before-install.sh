#!/usr/bin/env bash

# back it up in case the user has written valuable scripting that would be lost otherwise
mv /boot/openhd/scripts/custom_unmanaged_camera.sh /boot/openhd/scripts/custom_unmanaged_camera_old.sh
# NOTE: Updating overwrites the .config file and also the service file
rm -rf /boot/openhd/hardware.config

if [ "$(uname -m)" == "x86_64" ]; then
    if ! uname -a | grep -q "azure"; then
        whiptail --title "OpenHD" --yesno "You are about to install OpenHD to your Computer. Please be aware that we do not allow military usage! Do you want to continue ?" 10 50
        
        if [ $? -eq 0 ]; then
                whiptail --title "OpenHD" --yesno "Do you want to remove the old drivers ?" 10 50
            if [ $? -eq 0 ]; then
                echo "Removing old drivers"
                sudo dkms uninstall -m rtl8812au -v 5.2.20.2 --all
                sudo dkms uninstall -m rtl88x2bu -v 5.13.1 --all
            fi
        else
            exit 0
        fi
    fi
fi