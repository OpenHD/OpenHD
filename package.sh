#!/bin/bash

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

PACKAGE_ARCH=$1
OS=$2
DISTRO=$3
BUILD_TYPE=$4


if [[ "${DISTRO}" == "buster" ]]; then
    PLATFORM_PACKAGES="-d wiringpi -d veye-raspberrypi -d lifepoweredpi -d raspi2png -d gstreamer1.0-omx-rpi-config -d gst-rpicamsrc -d qopenhd -d openhd-linux-pi >= 20201122.2 -d libjpeg62-turbo"
    PLATFORM_CONFIGS="--config-files /boot/cmdline.txt --config-files /boot/config.txt --config-files /usr/local/share/openhd/joyconfig.txt"
fi

if [[ "${DISTRO}" == "bullseye" ]]; then
    PLATFORM_PACKAGES="-d veye-raspberrypi -d lifepoweredpi -d raspi2png -d gst-rpicamsrc -d qopenhd -d openhd-linux-pi >= 20201122.2 -d libjpeg62-turbo"
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

curl -1sLf 'https://dl.cloudsmith.io/public/openhd/openhd-2-1-testing/setup.deb.sh' | sudo -E bash && \
curl -1sLf 'https://dl.cloudsmith.io/public/openhd/openhd-2-1/setup.deb.sh' | sudo -E bash

apt -y update || exit 1

PACKAGE_NAME=openhd

PKGDIR=/tmp/${PACKAGE_NAME}-installdir

rm -rf ${PKGDIR}/*

mkdir -p ${PKGDIR}/root || exit 1

mkdir -p ${PKGDIR}/conf/openhd || exit 1
mkdir -p ${PKGDIR}/boot || exit 1
mkdir -p ${PKGDIR}/boot/osdfonts || exit 1

mkdir -p ${PKGDIR}/etc/network || exit 1
mkdir -p ${PKGDIR}/etc/sysctl.d || exit 1
mkdir -p ${PKGDIR}/etc/systemd/system || exit 1

mkdir -p ${PKGDIR}/home/openhd || exit 1
mkdir -p ${PKGDIR}/root || exit 1

mkdir -p ${PKGDIR}/usr/bin || exit 1
mkdir -p ${PKGDIR}/usr/sbin || exit 1
mkdir -p ${PKGDIR}/usr/share || exit 1
mkdir -p ${PKGDIR}/usr/lib || exit 1
mkdir -p ${PKGDIR}/usr/include || exit 1

mkdir -p ${PKGDIR}/usr/local/bin || exit 1
mkdir -p ${PKGDIR}/usr/local/etc || exit 1
mkdir -p ${PKGDIR}/usr/local/include || exit 1
mkdir -p ${PKGDIR}/usr/local/share || exit 1
mkdir -p ${PKGDIR}/usr/local/share/openhd || exit 1
mkdir -p ${PKGDIR}/usr/local/share/openhd/osdfonts || exit 1
mkdir -p ${PKGDIR}/usr/local/share/openhd/gnuplot || exit 1
mkdir -p ${PKGDIR}/usr/local/share/wifibroadcast-scripts || exit 1

./install_dep.sh || exit 1


build_source() {
    pushd lib/fmt
    rm -r build
    mkdir -p build
    pushd build
    cmake ../
    make -j3 || exit 1
    popd
    popd

    pushd openhd-system
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    pushd openhd-security
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    pushd openhd-interface
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    pushd openhd-status
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    pushd openhd-video
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    pushd openhd-power
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd
    
    pushd openhd-status
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    pushd openhd-telemetry
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    cp openhd-common/* ${PKGDIR}/usr/local/include || exit 1
    

    # legacy stuff, we should be working to reduce and eventually eliminate most of the stuff below
    # this line, aside from overlay files and default settings templates
    cp UDPSplitter/udpsplitter.py ${PKGDIR}/usr/local/bin/ || exit 1

    # if [[ "${OS}" == "raspbian" ]]; then
    #     echo "-------------BUILDING HELLO VIDEO FOR RASPBIAN--------"
    #     pushd wifibroadcast-hello_video
    #     make clean
    #     make -j3 || exit 1
    #     make install DESTDIR=${PKGDIR} || exit 1
    #     popd
    # fi

    pushd wifibroadcast-rc-Ath9k
    ./buildlora.sh || exit 1
    chmod 775 lora || exit 1
    cp -a lora ${PKGDIR}/usr/local/bin/ || exit 1
    
    ./build.sh || exit 1
    chmod 775 rctx || exit 1
    cp -a rctx ${PKGDIR}/usr/local/bin/ || exit 1

    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    cp -a wifibroadcast-scripts/* ${PKGDIR}/usr/local/share/wifibroadcast-scripts/ || exit 1

    cp -a overlay/etc/* ${PKGDIR}/etc/ || exit 1
    
    # note: this is non-standard behavior, packaging stuff in /root and /home, but it's temporary
    cp -a overlay/root/.bashrc ${PKGDIR}/root/ || exit 1
    cp -a overlay/home/openhd/.bashrc ${PKGDIR}/home/openhd/ || exit 1

    cp -a overlay/usr/local/etc/* ${PKGDIR}/usr/local/etc/ || exit 1

    cp -a overlay/etc/systemd/system/* ${PKGDIR}/etc/systemd/system/ || exit 1

    cp -a gnuplot/* ${PKGDIR}/usr/local/share/openhd/gnuplot/ || exit 1


    cp -a config/config.txt ${PKGDIR}/boot/ || exit 1
    cp -a config/cmdline.txt ${PKGDIR}/boot/ || exit 1
    cp -a config/ssh ${PKGDIR}/boot/ || exit 1

    cp -a config/apconfig.txt ${PKGDIR}/usr/local/share/openhd/ || exit 1
    cp -a config/joyconfig.txt ${PKGDIR}/usr/local/share/openhd/ || exit 1

    cp -a config/camera.template ${PKGDIR}/usr/local/share/openhd/ || exit 1
    cp -a config/ethernetcard.template ${PKGDIR}/usr/local/share/openhd/ || exit 1
    cp -a config/general.template ${PKGDIR}/usr/local/share/openhd/ || exit 1
    cp -a config/ltecard.template ${PKGDIR}/usr/local/share/openhd/ || exit 1
    cp -a config/vpn.template ${PKGDIR}/usr/local/share/openhd/ || exit 1
    cp -a config/wificard.template ${PKGDIR}/usr/local/share/openhd/ || exit 1
    cp -a config/telemetry.template ${PKGDIR}/usr/local/share/openhd/ || exit 1

    if [[ "${DISTRO}" == "buster" ]]; then
        cat << EOF >> ${PKGDIR}/boot/config.txt
[all]
dtoverlay=vc4-fkms-v3d
EOF

    fi
}

if [[ "${DISTRO}" == "bullseye" ]]; then
        cat << EOF >> ${PKGDIR}/boot/config.txt
[all]
dtoverlay=vc4-kms-v3d
EOF
fi

if [[ "${OS}" == "raspbian" ]]; then
    build_pi_dep
fi

build_source

VERSION=$(git describe)

rm ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb > /dev/null 2>&1

fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION//v} -C ${PKGDIR} \
  $PLATFORM_CONFIGS \
  -p ${PACKAGE_NAME}_VERSION_ARCH.deb \
  --after-install after-install.sh \
  --before-install before-install.sh \
  $PLATFORM_PACKAGES \
  -d "libasio-dev >= 1.10" \
  -d "libboost-system-dev >= 1.62.0" \
  -d "libboost-program-options-dev >= 1.62.0" \
  -d "libseek-thermal >= 20201118.1" \
  -d "flirone-driver >= 20200704.3" \
  -d "wifibroadcast >= 20200930.1" \
  -d "openhd-dump1090-mutability >= 20201122.2" \
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
