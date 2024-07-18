#bin/bash

# Copies the hardware.config to the appropriate location (/boot/openhd/)

SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

sudo mkdir -p /boot/openhd
sudo mkdir -p /config/openhd
sudo cp $SCRIPTPATH/config/hardware.config /boot/openhd/hardware.config
sudo cp $SCRIPTPATH/config/hardware.config /config/openhd/hardware.config
