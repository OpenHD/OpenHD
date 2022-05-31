#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# TODO do we need libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev ?

apt -y install build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libsodium-dev \
            libboost1.74-dev libasio-dev \
            libgstreamer-plugins-base1.0-dev \
            libusb-1.0-0-dev \
            libv4l-dev \
            apt-transport-https \
            curl \
            ruby-dev \
            python3-pip \
            libnl-3-dev cmake snapd libboost-all-dev libnl-genl-3-dev \
            libnl-route-3-dev || exit 1


gem install fpm
pip install --upgrade cloudsmith-cli

 sudo apt purge cmake
    sudo snap install cmake --classic
    wget https://codeload.github.com/fmtlib/fmt/zip/refs/tags/7.1.3
    unzip 7.1.3
    cd fmt*
    mkdir build 
    cd build
    cmake ..
    sudo make -j4 install
    cd ../../

