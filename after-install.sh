#!/usr/bin/env bash

systemctl enable openhd.service

# this is the serial port on the jetson boards, we don't want a tty running on it
systemctl stop nvgetty || true
systemctl disable nvgetty || true


# these are intentionally written with spaces around them to avoid false negatives if people
# edit the fstab file and change the rest of the line, the way these are written it will still
# find them as long as a line for each of these mountpoints is present


# enable the loopback module if it isn't already, so that seek and flir cameras can be used
grep "v4l2loopback" /etc/modules
if [[ "$?" -ne 0 ]]; then
    echo "v4l2loopback" >> /etc/modules
fi

if [ "$(uname -m)" == "x86_64" ]; then
    if ! uname -a | grep -q "azure"; then

        whiptail --title "OpenHD" --yesno "You are about to install OpenHD to your Computer. Do you want to install the required drivers ?" 10 50

        if [ $? -eq 0 ]; then
            echo "Installing drivers..."
            whiptail --title "OpenHD" --yesno "You are about to install OpenHD Drivers. Continue ?" 10 50
            if [ $? -eq 0 ]; then
            whiptail --title "Installing drivers" --msgbox "Installing drivers..." 10 50
            git clone https://github.com/OpenHD/rtl88x2bu /usr/src/rtl88x2bu-5.13.1
            git clone https://github.com/OpenHD/rtl8812au /usr/src/rtl8812au-git
            echo "Installing RTL8812AU..."
            cd /usr/src/rtl8812au-git
            ./dkms-install.sh || { echo "Failed to install RTL8812AU"; exit 1; }
            echo "RTL8812AU installed successfully."
            echo "Installing RTL8812BU..."
            cd /usr/src/rtl88x2bu-5.13.1
            sed -i 's/PACKAGE_VERSION="@PKGVER@"/PACKAGE_VERSION="5.13.1"/g' /usr/src/rtl88x2bu-5.13.1/dkms.conf
            dkms add -m rtl88x2bu -v 5.13.1 || { echo "Failed to install RTL8812BU"; exit 1; }
            dkms autoinstall
            echo "RTL8812BU installed successfully."  
            else
                whiptail --title "OpenHD Installation" --msgbox "Installation aborted. Stopped Installation." 10 50
                exit 0
            fi
        else
            whiptail --title "OpenHD Installation" --msgbox "No Drivers installed, please make sure to manually do that before running OpenHD!" 10 50
        fi
        echo "copying shortcuts"
        sudo chmod a+x /usr/share/applications/OpenHD*.desktop
        sudo chmod a+x /usr/share/applications/QOpenHD*.desktop
        for homedir in /home/*; do sudo cp /usr/share/applications/OpenHD*.desktop "$homedir"/Desktop/; done
        for homedir in /home/*; do sudo cp /usr/share/applications/QOpenHD*.desktop "$homedir"/Desktop/; done
        chmod +777 /etc/profile.d/desktop-truster.sh
        chmod a+x /etc/profile.d/desktop-truster.sh
    fi
fi