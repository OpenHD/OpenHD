#!/usr/bin/env bash

# Install all the dependencies needed to build OpenHD from source.
# TODO do we need libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev ?

sudo apt -y install build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libsodium-dev \
            libasio-dev \
            libgstreamer-plugins-base1.0-dev \
            libusb-1.0-0-dev \
            libv4l-dev \
            ruby-dev \
            python3-pip \
            git \
            apt-transport-https \
            curl \
            libavcodec-dev \
            git \
            libnl-3-dev libnl-genl-3-dev libnl-route-3-dev || exit 1

        sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
        sudo add-apt-repository ppa:mhier/libboost-latest -y
        sudo add-apt-repository ppa:git-core/ppa -y
        apt update
        apt upgrade
        sudo apt install libspdlog-dev build-essential -y
        sudo apt install gcc-8 g++-8 gcc-9 g++-9 gcc-10 g++-10

        sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10
        sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 80 --slave /usr/bin/g++ g++ /usr/bin/g++-9 --slave /usr/bin/gcov gcov /usr/bin/gcov-9
        sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8 --slave /usr/bin/gcov gcov /usr/bin/gcov-8

    sudo apt -y -o Dpkg::Options::="--force-overwrite" install libboost1.74-dev
    
apt -y install fmt cmake
gem install public_suffix -v 4.0.7
gem install fpm 
