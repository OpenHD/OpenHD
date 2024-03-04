#!/usr/bin/env bash

# back it up in case the user has written valuable scripting that would be lost otherwise
mv /boot/openhd/scripts/custom_unmanaged_camera.sh /boot/openhd/scripts/custom_unmanaged_camera_old.sh
# NOTE: Updating overwrites the .config file and also the service file
rm -rf /boot/openhd/hardware.config

if [ "$(uname -m)" == "x86_64" ]; then
    if ! uname -a | grep -q "azure"; then
        whiptail --title "OpenHD" --yesno "You are about to install OpenHD to your Computer. Please be aware that we do not allow military usage! Do you want to continue?" 10 50
        if ! [ $? -eq 0 ]; then
            echo "Operation cancelled."
            exit 1
        else
            whiptail --title "OpenHD" --yesno "Do you want to remove the old drivers?" 10 50
            if [ $? -eq 0 ]; then
                if whiptail --title "Confirmation" --yesno "This action will remove old drivers. Do you want to proceed?" 10 50; then
                    echo "Removing old drivers"
                    sudo dkms uninstall -m rtl8812au -v 5.2.20.2 --all || true
                    sudo dkms remove -m rtl8812au -v 5.2.20.2 --all || true
                    sudo dkms uninstall -m rtl88x2bu -v 5.13.1 --all || true
                    sudo dkms remove -m rtl88x2bu -v 5.13.1 --all || true
                else
                    echo "No drivers were removed!"
                fi
            fi
        fi
    fi
fi
