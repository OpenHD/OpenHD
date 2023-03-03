#!/bin/bash
#This file is the install instruction for the CHROOT build
#We're using cloudsmith-cli to upload the file in CHROOT
sudo apt install -y python3-pip
sudo pip3 install --upgrade cloudsmith-cli
./install_dep_ubuntu22.sh
sudo ./package.sh arm64 debian bullseye || exit 1
mkdir -p /opt/out/
cp -v *.dep /opt/out/
echo "copied deb file"
echo "push to cloudsmith"
git describe --exact-match HEAD >/dev/null 2>&1
echo "Pushing the package to OpenHD 2.3 repository"
ls -a
API_KEY=$(cat cloudsmith_api_key.txt)
cloudsmith push deb --api-key "$API_KEY" openhd/openhd-2-3-evo/debian/bullseye example.deb || exit 1

