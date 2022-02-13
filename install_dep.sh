#!/usr/bin/env bash

apt-get -y install cmake build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libpng-dev libsdl2-dev libsdl1.2-dev libconfig++-dev \
            libreadline-dev libjpeg-dev libusb-1.0-0-dev libsodium-dev \
            libfontconfig1-dev libfreetype6-dev \
            libgstreamer-plugins-base1.0-dev \
            libboost-dev libboost-program-options-dev libboost-system-dev libasio-dev libboost-chrono-dev libsystemd-dev \
            libboost-regex-dev libboost-filesystem-dev libboost-thread-dev indent libv4l-dev libnl-3-dev libnl-genl-3-dev \
            git ruby-dev build-essential python3-pip || exit 1
            
gem i fpm -f
pip install --upgrade cloudsmith-cli
  
