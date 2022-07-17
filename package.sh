#!/bin/bash

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

PACKAGE_ARCH=$1
OS=$2
DISTRO=$3
BUILD_TYPE=$4

if [[ "${DISTRO}" == "bullseye" ]]; then
    PLATFORM_PACKAGES="-d lifepoweredpi -d gst-rpicamsrc -d openhd-linux-pi -d libjpeg62-turbo"
    PLATFORM_CONFIGS="--config-files /usr/local/share/openhd/joyconfig.txt"
fi

if [[ "${OS}" == "ubuntu" ]] && [[ "${PACKAGE_ARCH}" == "armhf" || "${PACKAGE_ARCH}" == "arm64" ]]; then
    echo "--------------ADDING nvidia-l4t-gstreamer to package list--------------- "
    PLATFORM_PACKAGES="-d nvidia-l4t-gstreamer -d gcc-8 -d g++-8 -d gcc-9 -d g++-9 -d gcc-10 -d g++-10 -d libboost1.74-dev"
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

PACKAGE_NAME=openhd


PKGDIR=/tmp/${PACKAGE_NAME}-installdir
sudo rm -rf ${PKGDIR}/*

echo "getting hash"
cd /opt/Open.HD
ls -a
VER2=$(git rev-parse --short HEAD) 
echo ${VER2}
cd OpenHD

if [[ "${OS}" == "ubuntu" ]] && [[ "${PACKAGE_ARCH}" == "armhf" || "${PACKAGE_ARCH}" == "arm64" ]]; then
cd /opt
mkdir temp
cd temp
git clone -b 2.1-milestones https://github.com/OpenHD/Open.HD
cd Open.HD
git rev-parse --short HEAD ||exit
VER2=$(git rev-parse --short HEAD) 
echo ${VER2}
cd /opt/Open.HD/OpenHD
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

cp OpenHD ${PKGDIR}/usr/local/bin/OpenHD || exit 1
cp ../../openhd.service  ${PKGDIR}/etc/systemd/system/

echo "copied files"
echo ${PKGDIR}

VERSION="2.2.0-evo-$(date '+%m%d%H%M')-${VER2}"
echo ${VERSION}

rm ${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb > /dev/null 2>&1

echo $PACKAGE_ARCH
echo $PACKAGE_NAME
echo $VERSION
echo $PKGDIR

fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION} -C ${PKGDIR} \
  $PLATFORM_CONFIGS \
  -p ${PACKAGE_NAME}_VERSION_ARCH.deb \
  --after-install ../../after-install.sh \
  --before-install ../../before-install.sh \
  $PLATFORM_PACKAGES \
  -d "libasio-dev >= 1.10" \
  -d "gnuplot-nox" \
  -d "hostapd" \
  -d "iw" \
  -d "isc-dhcp-common" \
  -d "pump" \
  -d "dnsmasq" \
  -d "aircrack-ng" \
  -d "ser2net" \
  -d "i2c-tools" \
  -d "dos2unix" \
  -d "fuse" \
  -d "socat" \
  -d "ffmpeg" \
  -d "indent" \
  -d "libv4l-dev" \
  -d "libusb-1.0-0" \
  -d "libpcap-dev" \
  -d "libpng-dev" \
  -d "libnl-3-dev" \
  -d "libnl-genl-3-dev" \
  -d "libsdl2-2.0-0" \
  -d "libsdl1.2debian" \
  -d "libconfig++9v5" \
  -d "libreadline-dev" \
  -d "libjpeg-dev" \
  -d "libsodium-dev" \
  -d "libfontconfig1" \
  -d "libfreetype6" \
  -d "libgles2-mesa-dev" \
  -d "gstreamer1.0-plugins-base" \
  -d "gstreamer1.0-plugins-good" \
  -d "gstreamer1.0-plugins-bad" \
  -d "gstreamer1.0-plugins-ugly" \
  -d "gstreamer1.0-libav" \
  -d "gstreamer1.0-tools" \
  -d "gstreamer1.0-alsa" \
  -d "gstreamer1.0-pulseaudio" || exit 1

cp *.deb ../../

git describe --exact-match HEAD >/dev/null 2>&1

if [[ "${DISTRO}" == "bullseye" ]]; then
            echo "raspberry"
fi

if [[ "${DISTRO}" == "bionic" ]]; then
            echo "ubuntu"
fi 
