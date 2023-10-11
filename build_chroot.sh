#!/bin/bash
#This file is the install instruction for the CHROOT build
#We're using cloudsmith-cli to upload the file in CHROOT

echo "--------------------------------------------------------------"
echo "-----------------------HELLO-WORLD----------------------------"

sudo apt install -y python3-pip
sudo pip3 install --upgrade cloudsmith-cli
ls -a
API_KEY=$(cat cloudsmith_api_key.txt)
DISTRO=$(cat distro.txt)
FLAVOR=$(cat flavor.txt)
REPO=$(cat repo.txt)

echo ${DISTRO}
echo ${FLAVOR}
./install_build_dep.sh rock5
sudo ./package.sh arm64 ${DISTRO} ${FLAVOR} || exit 1
mkdir -p /opt/out/
cp -v *.dep /opt/out/
echo "copied deb file"
echo "push to cloudsmith"
git describe --exact-match HEAD >/dev/null 2>&1
echo "Pushing the package to OpenHD 2.3 repository"
ls -a
cloudsmith push deb --api-key "$API_KEY" openhd/${REPO}/${DISTRO}/${FLAVOR} *.deb || exit 1

