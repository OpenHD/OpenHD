#!/usr/bin/env bash

if [  -n "$(uname -a | grep Ubuntu)" ]; then
    sudo apt remove libboost*

fi

echo "-------------trying to mount boot... might not exist if jetson --------------"

mount -o remount,rw /boot || true

echo "-------------trying to mount conf... might not exist if pi --------------"

mount -o remount,rw /conf || true
