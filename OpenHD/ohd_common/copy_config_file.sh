#bin/bash

SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

sudo mkdir -p /boot/openhd
sudo cp $SCRIPTPATH/config/hardware.config /boot/openhd/hardware.config
