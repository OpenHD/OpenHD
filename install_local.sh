#!/bin/bash

#check if os is bionic
Release=$(lsb_release -r)
echo "$Release"

NumberOnly=$(cut -f2 <<< "$Release")

    if [[ ${NumberOnly} == 18.04 ]]; then
        echo "YOUR OS IS THE CORRECT TARGET FOR THIS BUILD!"
    elif [[ ${NumberOnly} < 18.04 ]]; then
        echo "YOUR OS VERSION IS OLDER THAN 18.04... EXITING"
        exit 1
    else 
        echo "YOUR OS IS NEWER THAN 18.04... THIS MIGHT NOT WORK..."
        echo "you might have unmet dependencies..."
    fi

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

PACKAGE_ARCH=x86
OS=ubuntu
DISTRO=bionic

PLATFORM_CONFIGS="--config-files /usr/local/share/openhd/joyconfig.txt"

echo "test fpm install"

 apt update && apt-get install --no-install-recommends -y ruby-dev build-essential
    gem i fpm -f
  


apt -y update || exit 1

PACKAGE_NAME=openhd

PKGDIR=/tmp/${PACKAGE_NAME}-installdir

rm -rf ${PKGDIR}/*

echo "++++++++++++++++++create dirs+++++++++++++++"

#mkdir -p ${PKGDIR}/root || exit 1

mkdir -p /conf/openhd || exit 1
#mkdir -p ${PKGDIR}/boot || exit 1
#mkdir -p ${PKGDIR}/boot/osdfonts || exit 1

#mkdir -p ${PKGDIR}/etc/network || exit 1
#mkdir -p ${PKGDIR}/etc/sysctl.d || exit 1
#mkdir -p ${PKGDIR}/etc/systemd/system || exit 1

#mkdir -p ${PKGDIR}/home/openhd || exit 1
#mkdir -p ${PKGDIR}/root || exit 1

#mkdir -p ${PKGDIR}/usr/bin || exit 1
#mkdir -p ${PKGDIR}/usr/sbin || exit 1
#mkdir -p ${PKGDIR}/usr/share || exit 1
#mkdir -p ${PKGDIR}/usr/lib || exit 1
#mkdir -p ${PKGDIR}/usr/include || exit 1

#mkdir -p ${PKGDIR}/usr/local/bin || exit 1
#mkdir -p ${PKGDIR}/usr/local/etc || exit 1
#mkdir -p ${PKGDIR}/usr/local/include || exit 1
#mkdir -p ${PKGDIR}/usr/local/share || exit 1
mkdir -p /usr/local/share/openhd || exit 1
#mkdir -p ${PKGDIR}/usr/local/share/openhd/osdfonts || exit 1
mkdir -p /usr/local/share/openhd/gnuplot || exit 1
mkdir -p /usr/local/share/wifibroadcast-scripts || exit 1

echo "+++++++++++++++install dependencies+++++++++++++"

apt -y install cmake build-essential autotools-dev automake libtool autoconf \
            libpcap-dev libpng-dev libsdl2-dev libsdl1.2-dev libconfig++-dev \
            libreadline-dev libjpeg-dev libusb-1.0-0-dev libsodium-dev \
            libfontconfig1-dev libfreetype6-dev \
            libgstreamer-plugins-base1.0-dev \
            libboost-dev libboost-program-options-dev libboost-system-dev libasio-dev libboost-chrono-dev libsystemd-dev \
            libboost-regex-dev libboost-filesystem-dev libboost-thread-dev indent libv4l-dev libnl-3-dev libnl-genl-3-dev \
            net-tools git || exit 1
#########################################################################################

# TODO FOR ALL BUILDS OPENHD AS A PACKAGE NEEDS TO BE ADDED 

#########################################################################################

if [[ ${NumberOnly} == 18.04 ]]; then
 apt -y install libasio-dev >= 1.10 libboost-system-dev >= 1.62.0 libboost-program-options-dev >= 1.62.0 \
    libseek-thermal >= 20200801.1 flirone-driver >= 20200704.3 wifibroadcast >= 20200930.1 \
    openhd-dump1090-mutability >= 20201122.2 gnuplot-nox hostapd iw isc-dhcp-common \
    pump dnsmasq aircrack-ng i2c-tools dos2unix fuse ffmpeg indent libv4l-dev \
    libusb-1.0-0 libpcap-dev libpng-dev libnl-3-dev libnl-genl-3-dev libsdl2-2.0-0 \
    libsdl1.2debian libconfig++9v5 libreadline-dev libjpeg-dev libsodium-dev \
    libfontconfig1 libfreetype6 libgles2-mesa-dev libboost-chrono-dev libboost-regex-dev \
    libboost-filesystem-dev libboost-thread-dev gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-alsa gstreamer1.0-pulseaudio || exit 1
fi

if [[ ${NumberOnly} > 18.04 ]]; then
apt -y install libasio-dev  gnuplot-nox hostapd iw isc-dhcp-common \
 dnsmasq aircrack-ng i2c-tools dos2unix fuse ffmpeg indent libv4l-dev   \
   libusb-1.0-0 libpcap-dev libpng-dev libnl-3-dev libnl-genl-3-dev \
   libsdl2-2.0-0 libsdl1.2debian libconfig++9v5 libreadline-dev \
   libjpeg-dev libsodium-dev libfontconfig1 libfreetype6 libgles2-mesa-dev \
   libboost-chrono-dev libboost-regex-dev libboost-filesystem-dev \
   libboost-thread-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
   gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav \
   gstreamer1.0-tools gstreamer1.0-alsa gstreamer1.0-pulseaudio || exit 1
fi


echo "+++++++++++++build openhd from source++++++++++++"
                
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

    pushd openhd-settings-m
    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    cp openhd-common/* /usr/local/include || exit 1
    
    cp UDPSplitter/udpsplitter.py /usr/local/bin/ || exit 1

    pushd wifibroadcast-rc-Ath9k
    ./buildlora.sh || exit 1
    chmod 775 lora || exit 1
    cp -a lora /usr/local/bin/ || exit 1
    
    ./build.sh || exit 1
    chmod 775 rctx || exit 1
    cp -a rctx /usr/local/bin/ || exit 1

    make clean
    make -j3 || exit 1
    make install DESTDIR=${PKGDIR} || exit 1
    popd

    cp -a wifibroadcast-scripts/* /usr/local/share/wifibroadcast-scripts/ 

echo "######################################################################"
echo "copying the /overlay/etc dir into your system... This could jack it up"
echo "######################################################################"
    cp -a overlay/etc/* /etc/ 
    
    # note: this is non-standard behavior, packaging stuff in /root and /home, but it's temporary
    #cp -a overlay/root/.bashrc /root/ 
    #cp -a overlay/home/openhd/.bashrc /home/openhd/ 

    cp -a overlay/usr/local/etc/* /usr/local/etc/ 

    #cp -a overlay/etc/systemd/system/* /etc/systemd/system/ 

    cp -a gnuplot/* /usr/local/share/openhd/gnuplot/ 


    #cp -a config/config.txt ${PKGDIR}/boot/ 
    #cp -a config/cmdline.txt ${PKGDIR}/boot/ 
    #cp -a config/ssh ${PKGDIR}/boot/ 

    cp -a config/apconfig.txt /usr/local/share/openhd/ 
    cp -a config/joyconfig.txt /usr/local/share/openhd/ 

    cp -a config/camera.template /usr/local/share/openhd/ 
    cp -a config/ethernetcard.template /usr/local/share/openhd/ 
    cp -a config/general.template /usr/local/share/openhd/ 
    cp -a config/ltecard.template /usr/local/share/openhd/ 
    cp -a config/vpn.template /usr/local/share/openhd/ 
    cp -a config/wificard.template /usr/local/share/openhd/ 
    cp -a config/telemetry.template /usr/local/share/openhd/

echo "###############MAKE WIFIBROADCAST FROM CONSTI SOURCE################"
echo "------------------->>untested below"
git clone https://github.com/Consti10/wifibroadcast.git

cd wifibroadcast
make
cp wfb_tx /usr/local/bin/
cp wfb_rx /usr/local/bin/
cp wfb_keygen /usr/local/bin/

  