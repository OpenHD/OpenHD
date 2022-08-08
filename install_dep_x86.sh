#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# TODO do we need libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev ?

apt -y install cmake build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libsodium-dev \
            libboost-filesystem1.74-dev libboost1.74-dev libboost1.74-all-dev \
            libasio-dev \
            libusb-1.0-0-dev \
            libv4l-dev \
            apt-transport-https \
            curl libgstreamer-plugins-base1.0-dev\
            libnl-3-dev libnl-genl-3-dev libnl-route-3-dev \
            libfmt-dev || exit 1
            
gem install fpm
pip install --upgrade cloudsmith-cli