#!/bin/bash
./install_dep_ubuntu22.sh
sudo ./package.sh arm64 debian bullseye
echo"here is a debug step"
ls -a
sudo apt install tree
tree
cp *.dep /opt/out/
echo "end"
