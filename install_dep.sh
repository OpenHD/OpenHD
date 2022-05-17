#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# TODO do we need libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev ?

apt -y install snapd build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libsodium-dev \
            libboost-dev libboost-system-dev libasio-dev libboost-filesystem-dev libboost-thread-dev libboost-regex-dev \
            libgstreamer-plugins-base1.0-dev \
            libusb-1.0-0-dev \
            libv4l-dev \
            tree \
            ruby-dev \
            python3-pip \
            libnl-3-dev libnl-genl-3-dev libnl-route-3-dev \
            libfmt-dev || exit 1
snap install cmake
cmake --version
gem install fpm