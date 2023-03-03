#!/bin/bash
sudo apt install -y python3-pip
sudo pip3 install --upgrade cloudsmith-cli
#./install_dep_ubuntu22.sh
#sudo ./package.sh arm64 debian bullseye

echo "here is a debug step"
ls -a
FILENAME="example.deb" && SIZE="1M" && touch $FILENAME && touch -m $FILENAME && dd if=/dev/urandom of=$FILENAME bs=$SIZE count=1
mkdir -p /opt/out/ || exit 1
cp -v *.dep /opt/out/
echo "copied deb file"
echo "push to cloudsmith"
git describe --exact-match HEAD >/dev/null 2>&1
echo "Pushing the package to OpenHD 2.3 repository"
ls -a
API_KEY=$(cat cloudsmith_api_key.txt)
cloudsmith push deb --api-key "$API_KEY" openhd/openhd-2-3-evo/debian/bullseye example.deb

