#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# This is for the xxx_build_test.yml github CI or when setting up a development environment
# PLEASE KEEP THIS FILE AS CLEAN AS POSSIBLE, Ubuntu22 is the baseline - for other platforms / OS versions, create their own files

# weird ubuntu 22 libundwind fix
sudo apt install -y libunwind-dev

sudo apt -y install cmake build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libsodium-dev ruby-dev \
            libboost-dev libboost-filesystem-dev \
            libusb-1.0-0-dev \
            libv4l-dev git \
            libnl-3-dev libnl-genl-3-dev libnl-route-3-dev || exit 1

# Gstreamer is separate from the "rest".
# Note that there is no compile time validation for all the elements required by gstreamer
sudo apt -y install libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly || exit 1

# Needed for RC, optional
sudo apt -y install libsdl2-dev

sudo gem install fpm
