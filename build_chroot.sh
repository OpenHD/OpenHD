#!/bin/bash
#./install_dep_ubuntu22.sh
#sudo ./package.sh arm64 debian bullseye
echo "here is a debug step"
ls -a
sudo apt install tree
tree
mkdir -p /opt/out
cp *.dep /opt/out/
zip file.zip * 
cp *.zip /opt/out
echo "end"
