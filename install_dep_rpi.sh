#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# TODO do we need libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev ?
   

apt -y install build-essential autotools-dev automake libtool autoconf libpcap-dev libsodium-dev libboost-all-dev 
apt -y install libasio-dev libgstreamer-plugins-base1.0-dev libusb-1.0-0-dev libv4l-dev git apt-transport-https curl libavcodec-dev 
apt -y install ruby-dev python3-pip libnl-3-dev cmake snapd libnl-genl-3-dev libnl-route-3-dev libcamera-dev
apt -y install libspdlog-dev

# Needed for RC, optional
apt -y install libsdl2-dev

gem install fpm

