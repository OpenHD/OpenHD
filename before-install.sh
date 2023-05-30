#!/usr/bin/env bash

mkdir -p /boot/openhd/rpi_camera_configs
rm -Rf /boot/openhd/rpi_camera_configs/*
# back it up in case the user has written valuable scripting that would be lost otherwise
mv /boot/openhd/scripts/custom_unmanaged_camera.sh /boot/openhd/scripts/custom_unmanaged_camera_old.sh
# NOTE: Updating overwrites the .config file and also the service file
rm -rf /boot/openhd/hardware.config
