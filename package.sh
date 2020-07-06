#!/bin/bash


PLATFORM=$1
DISTRO=$2


if [[ "${PLATFORM}" == "pi" ]]; then
    OS="raspbian"
    ARCH="arm"
    PACKAGE_ARCH="armhf"
fi

apt-get install -y apt-transport-https || exit 1
curl -1sLf 'https://dl.cloudsmith.io/public/openhd/openhd-2-0/cfg/gpg/gpg.B9F0E99CF5787237.key' | apt-key add - || exit 1


echo "deb https://dl.cloudsmith.io/public/openhd/openhd-2-0/deb/${OS} ${DISTRO} main" > /etc/apt/sources.list.d/openhd-2-0.list || exit 1

apt -y update || exit 1

PACKAGE_NAME=openhd

TMPDIR=/tmp/${PACKAGE_NAME}-installdir

rm -rf ${TMPDIR}/*

mkdir -p ${TMPDIR}/root || exit 1

mkdir -p ${TMPDIR}/boot || exit 1
mkdir -p ${TMPDIR}/boot/osdfonts || exit 1

mkdir -p ${TMPDIR}/etc/systemd/system || exit 1

mkdir -p ${TMPDIR}/usr/bin || exit 1
mkdir -p ${TMPDIR}/usr/sbin || exit 1
mkdir -p ${TMPDIR}/usr/share || exit 1
mkdir -p ${TMPDIR}/usr/lib || exit 1
mkdir -p ${TMPDIR}/usr/include || exit 1

mkdir -p ${TMPDIR}/usr/local/bin || exit 1
mkdir -p ${TMPDIR}/usr/local/share || exit 1

mkdir -p ${TMPDIR}/usr/local/share/openhd/osdfonts || exit 1
mkdir -p ${TMPDIR}/usr/local/share/openhd/gnuplot || exit 1
mkdir -p ${TMPDIR}/usr/local/share/RemoteSettings || exit 1
mkdir -p ${TMPDIR}/usr/local/share/cameracontrol || exit 1
mkdir -p ${TMPDIR}/usr/local/share/wifibroadcast-scripts || exit 1


apt -y install build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libpng-dev libsdl2-dev libsdl1.2-dev libconfig++-dev \
            libreadline-dev libjpeg8-dev libusb-1.0-0-dev libsodium-dev \
            libfontconfig1-dev libfreetype6-dev ttf-dejavu-core \
            libboost-dev libboost-program-options-dev libboost-system-dev libasio-dev libboost-chrono-dev \
            libboost-regex-dev libboost-filesystem-dev libboost-thread-dev wiringpi indent || exit 1


build_pi_dep() {
    pushd /opt/vc/src/hello_pi/libs/ilclient
    make || exit 1
    popd
}


build_source() {
    if [[ "${PLATFORM}" == "pi" ]]; then
        cp openhd-camera/openhdvid ${TMPDIR}/usr/local/bin/ || exit 1
        chmod +x ${TMPDIR}/usr/local/bin/openhdvid || exit 1
    fi
    
    cp UDPSplitter/udpsplitter.py ${TMPDIR}/usr/local/bin/ || exit 1

    if [[ "${PLATFORM}" == "pi" && "${DISTRO}" == "stretch" ]]; then
        pushd openvg
        make clean
        make library || exit 1
        make install DESTDIR=${TMPDIR} || exit 1
        popd
    fi

    pushd wifibroadcast-base
    make clean
    make || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    popd

    if [[ "${PLATFORM}" == "pi" && "${DISTRO}" == "stretch" ]]; then
        pushd wifibroadcast-status
        make clean
        make || exit 1
        make install DESTDIR=${TMPDIR} || exit 1
        popd
    fi

    pushd openhd-status
    make clean
    make || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    popd

    #
    # Copy to root so it runs on startup
    #
    pushd wifibroadcast-scripts
    cp -a .profile ${TMPDIR}/root/
    popd

    pushd wifibroadcast-misc
    cp -a ftee ${TMPDIR}/usr/local/bin/ || exit 1
    if [[ "${PLATFORM}" == "pi" ]]; then
        cp -a raspi2raspi ${TMPDIR}/usr/local/bin/ || exit 1
    fi
    cp -a gpio-IsAir.py ${TMPDIR}/usr/local/bin/ || exit 1
    cp -a gpio-config.py ${TMPDIR}/usr/local/bin/ || exit 1
    cp -a openhdconfig.sh ${TMPDIR}/usr/local/bin/ || exit 1
    popd


    if [[ "${PLATFORM}" == "pi" ]]; then
        pushd wifibroadcast-hello_video
        make clean
        make || exit 1
        make install DESTDIR=${TMPDIR} || exit 1
        popd
    fi

    pushd JoystickIn/JoystickIn
    make clean
    make || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    pushd


    cp -a RemoteSettings/* ${TMPDIR}/usr/local/share/RemoteSettings/ || exit 1



    pushd cameracontrol/RCParseChSrc
    make clean
    make RCParseCh || exit 1
    make install DESTDIR=${TMPDIR} || exit 1
    popd

    pushd cameracontrol/IPCamera/svpcom_wifibroadcast
    chmod 755 version.py
    make || exit 1
    ./wfb_keygen || exit 1
    popd

    cp -a cameracontrol/* ${TMPDIR}/usr/local/share/cameracontrol/ || exit 1


    pushd wifibroadcast-rc-Ath9k
    ./buildlora.sh || exit 1
    chmod 775 lora || exit 1
    cp -a lora ${TMPDIR}/usr/local/bin/ || exit 1
    
    ./build.sh || exit 1
    chmod 775 rctx || exit 1
    cp -a rctx ${TMPDIR}/usr/local/bin/ || exit 1
    popd

    if [[ "${PLATFORM}" == "pi" && "${DISTRO}" == "stretch" ]]; then
        pushd wifibroadcast-osd
        make clean
        make || exit 1
        make install DESTDIR=${TMPDIR} || exit 1
        cp -a osdfonts/* ${TMPDIR}/usr/local/share/openhd/osdfonts/ || exit 1
        popd
    fi

    if [[ "${PLATFORM}" == "pi" && "${DISTRO}" == "stretch" ]]; then
        pushd wifibroadcast-misc/LCD
        make || exit 1
        chmod 755 MouseListener || exit 1
        cp -a MouseListener ${TMPDIR}/usr/local/bin/ || exit 1
        popd
    fi

    cp -a wifibroadcast-scripts/* ${TMPDIR}/usr/local/share/wifibroadcast-scripts/ || exit 1

    cp -a systemd/* ${TMPDIR}/etc/systemd/system/ || exit 1

    cp -a gnuplot/* ${TMPDIR}/usr/local/share/openhd/gnuplot/ || exit 1

    cp -a config/* ${TMPDIR}/boot/ || exit 1
    if [[ "${PLATFORM}" == "pi" && "${DISTRO}" == "buster" ]]; then
        cat << EOF >> ${TMPDIR}/boot/config.txt
[all]
dtoverlay=vc4-fkms-v3d
EOF
    fi
    cp -a config/openhd-settings-1.txt ${TMPDIR}/boot/openhd-settings-2.txt || exit 1
    cp -a config/openhd-settings-1.txt ${TMPDIR}/boot/openhd-settings-3.txt || exit 1
    cp -a config/openhd-settings-1.txt ${TMPDIR}/boot/openhd-settings-4.txt || exit 1

    cp -a driver-helpers/* ${TMPDIR}/usr/local/bin/ || exit 1
}

if [[ "${PLATFORM}" == "pi" ]]; then
    build_pi_dep
fi

build_source


VERSION=$(git describe)

rm ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb > /dev/null 2>&1

fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION//v} -C ${TMPDIR} \
  --config-files /boot/openhd-settings-1.txt \
  --config-files /boot/openhd-settings-2.txt \
  --config-files /boot/openhd-settings-3.txt \
  --config-files /boot/openhd-settings-4.txt \
  --config-files /boot/apconfig.txt \
  --config-files /boot/cmdline.txt \
  --config-files /boot/config.txt \
  --config-files /boot/joyconfig.txt \
  --config-files /boot/osdconfig.txt \
  -p ${PACKAGE_NAME}_VERSION_ARCH.deb \
  --after-install after-install.sh \
  --before-install before-install.sh \
  -d "wiringpi" \
  -d "libasio-dev >= 1.10" \
  -d "libboost-system-dev >= 1.62.0" \
  -d "libboost-program-options-dev >= 1.62.0" \
  -d "openhd-router = 0.1.7" \
  -d "openhd-microservice = 0.1.13" \
  -d "qopenhd" \
  -d "flirone-driver = 20200704.3" \
  -d "veye-raspberrypi = 20200706.1" \
  -d "lifepoweredpi = 20200704.2" \
  -d "mavlink-router = 20200704.3" \
  -d "gnuplot-nox" \
  -d "hostapd" \
  -d "iw" \
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
  -d "libpcap-dev" \
  -d "libpng-dev" \
  -d "libsdl2-2.0-0" \
  -d "libsdl1.2debian" \
  -d "libconfig++9v5" \
  -d "libreadline-dev" \
  -d "libjpeg8" \
  -d "libsodium-dev" \
  -d "libfontconfig1" \
  -d "libfreetype6" \
  -d "ttf-dejavu-core" \
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
  -d "gstreamer1.0-pulseaudio" \
  -d "gstreamer1.0-omx-rpi-config" || exit 1

#
# Only push to cloudsmith for tags. If you don't want something to be pushed to the repo, 
# don't create a tag. You can build packages and test them locally without tagging.
#
git describe --exact-match HEAD > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
    echo "Pushing package to OpenHD repository"
    cloudsmith push deb openhd/openhd-2-0/${OS}/${DISTRO} ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb || exit 1
else
    echo "Not a tagged release, skipping push to OpenHD repository"
fi

