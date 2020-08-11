#!/usr/bin/env bash

apt -y install build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libpng-dev libsdl2-dev libsdl1.2-dev libconfig++-dev \
            libreadline-dev libjpeg-dev libusb-1.0-0-dev libsodium-dev \
            libfontconfig1-dev libfreetype6-dev ttf-dejavu-core \
            libboost-dev libboost-program-options-dev libboost-system-dev libasio-dev libboost-chrono-dev \
            libboost-regex-dev libboost-filesystem-dev libboost-thread-dev wiringpi indent || exit 1
