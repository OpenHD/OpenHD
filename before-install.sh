#!/usr/bin/env bash

if [  -n "$(uname -a | grep Ubuntu)" ]; then
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    sudo add-apt-repository ppa:mhier/libboost-latest -y
    sudo add-apt-repository ppa:git-core/ppa -y
    apt update
fi

echo "-------------trying to mount boot... might not exist if jetson --------------"

mount -o remount,rw /boot || true

echo "-------------trying to mount conf... might not exist if pi --------------"

mount -o remount,rw /conf || true
