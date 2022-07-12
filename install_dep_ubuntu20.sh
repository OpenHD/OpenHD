#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# This is for the simple_build_test.yml github CI or when setting up a development environment

sudo apt -y install build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libsodium-dev \
            libboost-all-dev libasio-dev \
            libgstreamer-plugins-base1.0-dev \
            libusb-1.0-0-dev \
            libv4l-dev \
            apt-transport-https \
            curl \
            ruby-dev \
            python3-pip \
            libnl-3-dev cmake snapd libboost-all-dev libnl-genl-3-dev \
            libnl-route-3-dev \
            libfmt-dev || exit 1
