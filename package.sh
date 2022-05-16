#!/bin/bash

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

PACKAGE_ARCH=$1
OS=$2
DISTRO=$3
BUILD_TYPE=$4



if [[ "${DISTRO}" == "buster" ]]; then
    PLATFORM_PACKAGES="-d wiringpi -d veye-raspberrypi -d lifepoweredpi -d raspi2png -d gstreamer1.0-omx-rpi-config -d gst-rpicamsrc -d qopenhd -d openhd-linux-pi -d libjpeg62-turbo"
    PLATFORM_CONFIGS="--config-files /boot/cmdline.txt --config-files /boot/config.txt --config-files /usr/local/share/openhd/joyconfig.txt"
fi

if [[ "${DISTRO}" == "bullseye" ]]; then
    PLATFORM_PACKAGES="-d veye-raspberrypi -d lifepoweredpi -d raspi2png -d gst-rpicamsrc -d qopenhd -d openhd-linux-pi -d libjpeg62-turbo"
    PLATFORM_CONFIGS="--config-files /boot/cmdline.txt --config-files /boot/config.txt --config-files /usr/local/share/openhd/joyconfig.txt"
fi

if [[ "${OS}" == "ubuntu" ]] && [[ "${PACKAGE_ARCH}" == "armhf" || "${PACKAGE_ARCH}" == "arm64" ]]; then
    echo "--------------ADDING nvidia-l4t-gstreamer to package list--------------- "
    PLATFORM_PACKAGES="-d nvidia-l4t-gstreamer -d qopenhd"
    PLATFORM_CONFIGS="--config-files /usr/local/share/openhd/joyconfig.txt"
fi

if [ "${BUILD_TYPE}" == "docker" ]; then
    cat << EOF > /etc/resolv.conf
options rotate
options timeout:1
nameserver 8.8.8.8
nameserver 8.8.4.4
EOF
fi


apt-get install -y apt-transport-https curl || exit 1

curl -1sLf 'https://dl.cloudsmith.io/public/openhd/openhd-2-1-alpha/setup.deb.sh' | sudo -E bash && \

pip install --upgrade cloudsmith-cli
cloudsmith token

apt -y update || exit 1

PACKAGE_NAME=openhd
PACKAGE_ARCH=armhf
OS=raspbian
DISTRO=bullseye



PKGDIR=/tmp/${PACKAGE_NAME}-installdir
sudo rm -rf ${PKGDIR}/*

./install_dep.sh || exit 1

cd OpenHD

rm -rf build

mkdir build

cd build

cmake ..
make -j4

ls -a

mkdir -p ${PKGDIR}/usr/local/bin || exit 1
tree
cp OpenHD ${PKGDIR}/usr/local/bin/OpenHD || exit 1
echo "copied files"
echo ${PKGDIR}

VERSION="2.1-$(date '+%m%d%H')"

rm ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb > /dev/null 2>&1

