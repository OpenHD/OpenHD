#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# TODO do we need libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev ?
   

apt -y install build-essential autotools-dev automake libtool autoconf libpcap-dev libsodium-dev libboost-filesystem1.74-dev
apt -y install libasio-dev libgstreamer-plugins-base1.0-dev libusb-1.0-0-dev libv4l-dev git apt-transport-https curl
apt -y install ruby-dev python3-pip libnl-3-dev cmake snapd libnl-genl-3-dev libnl-route-3-dev git
apt -y remove  python3-libcamera libcamera0
apt -y -o Dpkg::Options::="--force-overwrite" --no-install-recommends install libcamera-openhd

# Needed for RC, optional
apt -y install libsdl2-dev

gem install fpm

