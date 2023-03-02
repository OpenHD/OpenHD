#!/bin/bash
./install_dep_ubuntu22.sh
sudo ./package.sh arm64 debian bullseye
cp *.dep /opt/out/
