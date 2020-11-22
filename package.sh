#!/bin/bash

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

PACKAGE_ARCH=$1
OS=$2
DISTRO=$3
BUILD_TYPE=$4


if [[ "${OS}" == "raspbian" ]]; then
    PLATFORM_PACKAGES=("-d wiringpi"  "-d veye-raspberrypi >= 20200706.1"  "-d lifepoweredpi >= 20200704.2"  "-d raspi2png >= 20200704.2"  "-d gstreamer1.0-omx-rpi-config" "-d gst-rpicamsrc")
fi

if [[ "${OS}" == "ubuntu" ]] && [[ "${PACKAGE_ARCH}" == "armhf" || "${PACKAGE_ARCH}" == "arm64" ]]; then
    PLATFORM_PACKAGES=("-d wiringpi")
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
curl -1sLf 'https://dl.cloudsmith.io/public/openhd/openhd-2-1/cfg/gpg/gpg.0AD501344F75A993.key' | apt-key add - || exit 1


echo "deb https://dl.cloudsmith.io/public/openhd/openhd-2-1/deb/${OS} ${DISTRO} main" > /etc/apt/sources.list.d/openhd-2-1.list || exit 1

apt -y update || exit 1

PACKAGE_NAME=openhd

TMPDIR=/tmp/${PACKAGE_NAME}-installdir

rm -rf ${TMPDIR}/*

mkdir -p ${TMPDIR}/root || exit 1

mkdir -p ${TMPDIR}/boot || exit 1
mkdir -p ${TMPDIR}/boot/osdfonts || exit 1

mkdir -p ${TMPDIR}/etc/network || exit 1
mkdir -p ${TMPDIR}/etc/sysctl.d || exit 1
mkdir -p ${TMPDIR}/etc/systemd/system || exit 1

mkdir -p ${TMPDIR}/home/pi || exit 1
mkdir -p ${TMPDIR}/root || exit 1

mkdir -p ${TMPDIR}/usr/bin || exit 1
mkdir -p ${TMPDIR}/usr/sbin || exit 1
mkdir -p ${TMPDIR}/usr/share || exit 1
mkdir -p ${TMPDIR}/usr/lib || exit 1
mkdir -p ${TMPDIR}/usr/include || exit 1

mkdir -p ${TMPDIR}/usr/local/bin || exit 1
mkdir -p ${TMPDIR}/usr/local/etc || exit 1
mkdir -p ${TMPDIR}/usr/local/include || exit 1
mkdir -p ${TMPDIR}/usr/local/share || exit 1
mkdir -p ${TMPDIR}/usr/local/share/openhd || exit 1
mkdir -p ${TMPDIR}/usr/local/share/openhd/osdfonts || exit 1
mkdir -p ${TMPDIR}/usr/local/share/openhd/gnuplot || exit 1
mkdir -p ${TMPDIR}/usr/local/share/wifibroadcast-scripts || exit 1

./install_dep.sh || exit 1

build_pi_dep() {
    pushd /opt/vc/src/hello_pi/libs/ilclient
    make || exit 1
    popd
}


build_source() {
    pushd openhd-system
    make clean
    make || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    popd

    pushd openhd-security
    make clean
    make || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    popd

    pushd openhd-interface
    make clean
    make || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    popd

    pushd openhd-status
    make clean
    make || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    popd

    cp openhd-common/* ${TMPDIR}/usr/local/include || exit 1
    

    # legacy stuff, we should be working to reduce and eventually eliminate most of the stuff below
    # this line, aside from overlay files and default settings templates
    cp UDPSplitter/udpsplitter.py ${TMPDIR}/usr/local/bin/ || exit 1

    if [[ "${PLATFORM}" == "pi" && "${DISTRO}" == "stretch" ]]; then
        pushd openvg
        make clean
        make library || exit 1
        make install DESTDIR=${TMPDIR} || exit 1
        popd
    fi

    if [[ "${PLATFORM}" == "pi" ]]; then
        pushd wifibroadcast-hello_video
        make clean
        make || exit 1
        make install DESTDIR=${TMPDIR} || exit 1
        popd
    fi

    pushd wifibroadcast-rc-Ath9k
    ./buildlora.sh || exit 1
    chmod 775 lora || exit 1
    cp -a lora ${TMPDIR}/usr/local/bin/ || exit 1
    
    ./build.sh || exit 1
    chmod 775 rctx || exit 1
    cp -a rctx ${TMPDIR}/usr/local/bin/ || exit 1

    make clean
    make || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    popd

    if [[ "${PLATFORM}" == "pi" && "${DISTRO}" == "stretch" ]]; then
        pushd wifibroadcast-osd
        make clean
        make || exit 1
        make install DESTDIR=${TMPDIR} || exit 1
        cp -a osdfonts/* ${TMPDIR}/usr/local/share/openhd/osdfonts/ || exit 1
        popd
    fi

    cp -a wifibroadcast-scripts/* ${TMPDIR}/usr/local/share/wifibroadcast-scripts/ || exit 1

    cp -a overlay/etc/* ${TMPDIR}/etc/ || exit 1
    
    # note: this is non-standard behavior, packaging stuff in /root and /home, but it's temporary
    cp -a overlay/root/.bashrc ${TMPDIR}/root/ || exit 1
    cp -a overlay/home/pi/.bashrc ${TMPDIR}/home/pi/ || exit 1

    cp -a overlay/usr/local/etc/* ${TMPDIR}/usr/local/etc/ || exit 1

    cp -a overlay/etc/systemd/system/* ${TMPDIR}/etc/systemd/system/ || exit 1

    cp -a gnuplot/* ${TMPDIR}/usr/local/share/openhd/gnuplot/ || exit 1

    cp -a config/* ${TMPDIR}/boot/ || exit 1
    if [[ "${PLATFORM}" == "pi" && "${DISTRO}" == "buster" ]]; then
        cat << EOF >> ${TMPDIR}/boot/config.txt
[all]
dtoverlay=vc4-fkms-v3d
EOF
    fi
}

if [[ "${PLATFORM}" == "pi" ]]; then
    build_pi_dep
fi

build_source


VERSION=$(git describe)

rm ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb > /dev/null 2>&1

fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION//v} -C ${TMPDIR} \
  --config-files /boot/apconfig.txt \
  --config-files /boot/cmdline.txt \
  --config-files /boot/config.txt \
  --config-files /boot/joyconfig.txt \
  --config-files /boot/osdconfig.txt \
  -p ${PACKAGE_NAME}_VERSION_ARCH.deb \
  --after-install after-install.sh \
  --before-install before-install.sh \
  "${PLATFORM_PACAKGES[@]}" \
  -d "libasio-dev >= 1.10" \
  -d "libboost-system-dev >= 1.62.0" \
  -d "libboost-program-options-dev >= 1.62.0" \
  -d "openhd-router >= 0.1.8" \
  -d "openhd-microservice >= 0.1.18" \
  -d "libseek-thermal >= 20200801.1" \
  -d "flirone-driver >= 20200704.3" \
  -d "mavlink-router >= 20200704.3" \
  -d "wifibroadcast >= 20200930.1" \
  -d "openhd-dump1090-mutability >= 20201122.2" \
  -d "gnuplot-nox" \
  -d "hostapd" \
  -d "iw" \
  -d "isc-dhcp-common" \
  -d "dnsmasq" \
  -d "aircrack-ng" \
  -d "i2c-tools" \
  -d "dos2unix" \
  -d "fuse" \
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
  -d "ttf-dejavu-core" \
  -d "libgles2-mesa-dev" \
  -d "libboost-chrono-dev" \
  -d "libboost-regex-dev" \
  -d "libboost-filesystem-dev" \
  -d "libboost-thread-dev" \
  -d "gstreamer1.0-plugins-base" \
  -d "gstreamer1.0-plugins-good" \
  -d "gstreamer1.0-plugins-bad" \
  -d "gstreamer1.0-plugins-ugly" \
  -d "gstreamer1.0-libav" \
  -d "gstreamer1.0-tools" \
  -d "gstreamer1.0-alsa" \
  -d "gstreamer1.0-pulseaudio" || exit 1

#
# Only push to cloudsmith for tags. If you don't want something to be pushed to the repo, 
# don't create a tag. You can build packages and test them locally without tagging.
#
git describe --exact-match HEAD > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
    echo "Pushing package to OpenHD repository"
    cloudsmith push deb openhd/openhd-2-1/${OS}/${DISTRO} ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb || exit 1
else
    echo "Pushing package to OpenHD testing repository"
    cloudsmith push deb openhd/openhd-2-1-testing/${OS}/${DISTRO} ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb || exit 1
fi

