#!/usr/bin/env bash

mkdir -p /boot/openhd/rpi_camera_configs
rm -Rf /boot/openhd/rpi_camera_configs/*
# NOTE: Updating overwrites the .config file and also the service file
rm -rf /boot/openhd/hardware.config
rm -rf /boot/openhd/enable_ip_camera.sh