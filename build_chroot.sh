#!/bin/bash
sudo apt install -y python3-pip
sudo pip3 install --upgrade cloudsmith-cli
#./install_dep_ubuntu22.sh
#sudo ./package.sh arm64 debian bullseye

echo "here is a debug step"
ls -a
FILENAME="example.deb" && SIZE="1m" && touch $FILENAME && touch -m $FILENAME && dd if=/dev/urandom of=$FILENAME bs=$SIZE count=1
mkdir -p /opt/out/ || exit 1
cp -v *.dep /opt/out/
echo "copied deb file"
echo "push to cloudsmith"
git describe --exact-match HEAD >/dev/null 2>&1
echo "Pushing package to OpenHD 2.3 repository"
cloudsmith push deb openhd/openhd-2-3-evo/debian/bullseye openhd_2.3-Evo-Rock_arm64.deb


