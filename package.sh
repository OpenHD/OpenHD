#!/bin/bash

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

PACKAGE_ARCH=$1
OS=$2
DISTRO=$3
BUILD_TYPE=$4


PACKAGE_NAME=openhd


PKGDIR=/tmp/${PACKAGE_NAME}-installdir
sudo rm -rf ${PKGDIR}/*

echo "getting hash"
cd /opt/OpenHD
ls -a
VER2=$(git rev-parse --short HEAD) 
echo ${VER2}
cd OpenHD

if [[ "${OS}" == "ubuntu" ]] && [[ "${PACKAGE_ARCH}" == "armhf" || "${PACKAGE_ARCH}" == "arm64" ]]; then
cd /opt
mkdir temp
cd temp
git clone https://github.com/OpenHD/OpenHD
cd OpenHD
git rev-parse --short HEAD ||exit
VER2=$(git rev-parse --short HEAD) 
echo ${VER2}
cd /opt/OpenHD/OpenHD
fi

rm -rf build

mkdir build

cd build

cmake ..
make -j4

ls -a

mkdir -p ${PKGDIR}/usr/local/bin || exit 1
mkdir -p ${PKGDIR}/tmp
mkdir -p ${PKGDIR}/settings
mkdir -p ${PKGDIR}/etc/systemd/system

if [[ "${OS}" == "raspbian" ]]; then
  mkdir -p ${PKGDIR}/boot/openhd/rpi_camera_configs
  cp ../../rpi_camera_configs/* ${PKGDIR}/boot/openhd/rpi_camera_configs/
fi

cp openhd ${PKGDIR}/usr/local/bin/openhd || exit 1

if [[ "${PACKAGE_ARCH}" != "x86_64" ]]; then
cp ../../openhd.service  ${PKGDIR}/etc/systemd/system/
fi

echo "copied files"
echo ${PKGDIR}


VERSION="2.3-evo-$(date '+%Y%m%d%H%M')-${VER2}"
echo ${VERSION}

rm ${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb > /dev/null 2>&1

echo $PACKAGE_ARCH
echo $PACKAGE_NAME
echo $VERSION
echo $PKGDIR

if [[ "${PACKAGE_ARCH}" != "x86_64" && "${PACKAGE_ARCH}" != "arm64" ]]; then
fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION} -C ${PKGDIR} \
  $PLATFORM_CONFIGS \
  -p ${PACKAGE_NAME}_VERSION_ARCH.deb \
  --after-install ../../after-install.sh \
  --before-install ../../before-install.sh \
  $PLATFORM_PACKAGES \
  -d "iw" \
  -d "libcamera-openhd" \
  -d "aircrack-ng" \
  -d "i2c-tools" \
  -d "libv4l-dev" \
  -d "libusb-1.0-0" \
  -d "libpcap-dev" \
  -d "libpng-dev" \
  -d "libnl-3-dev" \
  -d "libnl-genl-3-dev" \
  -d "libsdl2-2.0-0" \
  -d "libconfig++9v5" \
  -d "libreadline-dev" \
  -d "libsodium-dev" \
  -d "gstreamer1.0-plugins-base" \
  -d "gstreamer1.0-plugins-good" \
  -d "gstreamer1.0-plugins-bad" \
  -d "gstreamer1.0-plugins-ugly" \
  -d "gstreamer1.0-libav" \
  -d "gstreamer1.0-tools" \
  -d "gstreamer1.0-alsa" \
  -d "gstreamer1.0-pulseaudio" || exit 1
elif [[ "${PACKAGE_ARCH}" == "arm64" ]]; then
fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION} -C ${PKGDIR} \
  $PLATFORM_CONFIGS \
  -p ${PACKAGE_NAME}_VERSION_ARCH.deb \
  --after-install ../../after-install.sh \
  --before-install ../../before-install.sh \
  $PLATFORM_PACKAGES \
  -d "iw" \
  -d "aircrack-ng" \
  -d "i2c-tools" \
  -d "libv4l-dev" \
  -d "libusb-1.0-0" \
  -d "libpcap-dev" \
  -d "libpng-dev" \
  -d "libnl-3-dev" \
  -d "libnl-genl-3-dev" \
  -d "libsdl2-2.0-0" \
  -d "libconfig++9v5" \
  -d "libreadline-dev" \
  -d "libsodium-dev" \
  -d "gstreamer1.0-plugins-base" \
  -d "gstreamer1.0-plugins-good" \
  -d "gstreamer1.0-plugins-bad" \
  -d "gstreamer1.0-plugins-ugly" \
  -d "gstreamer1.0-libav" \
  -d "gstreamer1.0-tools" \
  -d "gstreamer1.0-alsa" \
  -d "gstreamer1.0-pulseaudio" || exit 1
else
fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION} -C ${PKGDIR} \
  $PLATFORM_CONFIGS \
  -p ${PACKAGE_NAME}_VERSION_ARCH.deb \
  --after-install ../../after-install_x86.sh \
  --before-install ../../before-install.sh \
  $PLATFORM_PACKAGES \
  -d "iw" \
  -d "aircrack-ng" \
  -d "i2c-tools" \
  -d "libv4l-dev" \
  -d "libusb-1.0-0" \
  -d "libpcap-dev" \
  -d "libnl-3-dev" \
  -d "libnl-genl-3-dev" \
  -d "libsdl2-2.0-0" \
  -d "libconfig++9v5" \
  -d "libreadline-dev" \
  -d "libsodium-dev" \
  -d "gstreamer1.0-plugins-base" \
  -d "gstreamer1.0-plugins-good" \
  -d "gstreamer1.0-plugins-bad" \
  -d "gstreamer1.0-plugins-ugly" \
  -d "gstreamer1.0-libav" \
  -d "gstreamer1.0-tools" \
  -d "gstreamer1.0-alsa" \
  -d "gstreamer1.0-pulseaudio" || exit 1
fi 
cp *.deb ../../
