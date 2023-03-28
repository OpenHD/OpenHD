#!/usr/bin/env bash
set -e

PLATFORM="$1"


BASE_PACKAGES="libusb-1.0-0-dev libpcap-dev libsodium-dev libnl-3-dev libnl-genl-3-dev libnl-route-3-dev libsdl2-dev"
VIDEO_PACKAGES="libgstreamer-plugins-base1.0-dev libv4l-dev"
BUILD_PACKAGES="git build-essential autotools-dev automake libtool python3-pip autoconf apt-transport-https ruby-dev cmake"


function install_jetson_packages {
PLATFORM_PACKAGES="libasio-dev libavcodec-dev gcc-8 g++-8 gcc-9 g++-9 gcc-10 g++-10 libboost1.74-dev libboost1.74 libboost-filesystem-dev ruby-dev"
PLATFORM_PACKAGES_REMOVE=""
#adding Jetson repositories
add-apt-repository ppa:ubuntu-toolchain-r/test -y
add-apt-repository ppa:mhier/libboost-latest -y
add-apt-repository ppa:git-core/ppa -y
}
function update_jetson {
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 80 --slave /usr/bin/g++ g++ /usr/bin/g++-9 --slave /usr/bin/gcov gcov /usr/bin/gcov-9
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8 --slave /usr/bin/gcov gcov /usr/bin/gcov-8
gem install public_suffix -v 4.0.7
}
function install_pi_packages {
PLATFORM_PACKAGES="libboost-filesystem1.74-dev libasio-dev libcamera-openhd"
PLATFORM_PACKAGES_REMOVE="python3-libcamera libcamera0"
}
function install_x86_packages {
PLATFORM_PACKAGES="libunwind-dev libboost-dev libboost-filesystem-dev gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly"
PLATFORM_PACKAGES_REMOVE=""
}
function install_rock_packages {
PLATFORM_PACKAGES=""
PLATFORM_PACKAGES_REMOVE=""
}

 # Add OpenHD Repository platform-specific packages
 apt install -y curl
 curl -1sLf 'https://dl.cloudsmith.io/public/openhd/openhd-2-3-evo/setup.deb.sh'| sudo -E bash
 apt update

# Main function
 
 if [[ "${PLATFORM}" == "rpi" ]]; then
    install_pi_packages
 elif [[ "${PLATFORM}" == "jetson" ]] ; then
    install_jetson_packages
	 update_jetson
 elif [[ "${PLATFORM}" == "ubuntu-x86" ]] ; then
    install_x86_packages
 elif [[ "${PLATFORM}" == "rock5" ]] ; then
    install_rock_packages
 else
    echo "platform not supported"
 fi


 # Install platform-specific packages
 echo "Removing platform-specific packages..."
 for package in ${PLATFORM_PACKAGES_REMOVE}; do
     echo "Removing ${package}..."
     apt purge -y ${package}
     if [ $? -ne 0 ]; then
         echo "Failed to remove ${package}!"
         exit 1
     fi
 done

 # Install platform-specific packages
 echo "Installing platform-specific packages..."
 for package in ${PLATFORM_PACKAGES} ${BASE_PACKAGES} ${VIDEO_PACKAGES} ${BUILD_PACKAGES}; do
     echo "Installing ${package}..."
     apt install -y -o Dpkg::Options::="--force-overwrite" --no-install-recommends ${package}
     if [ $? -ne 0 ]; then
         echo "Failed to install ${package}!"
         exit 1
     fi
 done
 


# Installing ruby packages
gem install fpm

