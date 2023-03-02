#!/bin/bash
#./install_dep_ubuntu22.sh
#sudo ./package.sh arm64 debian bullseye
echo "here is a debug step"
ls -a
mkdir -p /opt/out/ || exit 1
cp -v *.dep /opt/out/
echo "copied deb file"
zip file.zip * 
cp -v *.zip /opt/out/
echo "copied zip file"
echo "end"
