#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# This is for the simple_build_test.yml github CI or when setting up a development environment
# PLEASE KEEP THIS FILE AS CLEAN AS POSSIBLE, Ubuntu20 is the baseline - for other platforms / OS versions, create their own files

sudo apt -y install cmake build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libsodium-dev \
            libboost-dev libboost-filesystem-dev \
            libusb-1.0-0-dev \
            libv4l-dev \
            libavcodec-dev \
            libnl-3-dev libnl-genl-3-dev libnl-route-3-dev \
            libfmt-dev \
            libspdlog-dev || exit 1

sudo apt -y install libgstreamer-plugins-base1.0-dev libunwind-14-dev

sudo apt -y install libsdl2-dev