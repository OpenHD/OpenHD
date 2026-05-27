#!/usr/bin/env bash
################################################################################
# OpenHD
# 
# Licensed under the GNU General Public License (GPL) Version 3.
# 
# This software is provided "as-is," without warranty of any kind, express or 
# implied, including but not limited to the warranties of merchantability, 
# fitness for a particular purpose, and non-infringement. For details, see the 
# full license in the LICENSE file provided with this source code.
# 
# Non-Military Use Only:
# This software and its associated components are explicitly intended for 
# civilian and non-military purposes. Use in any military or defense 
# applications is strictly prohibited unless explicitly and individually 
# licensed otherwise by the OpenHD Team.
# 
# Contributors:
# A full list of contributors can be found at the OpenHD GitHub repository:
# https://github.com/OpenHD
# 
# © OpenHD, All Rights Reserved.
################################################################################

set -e

PLATFORM="$1"


BASE_PACKAGES="libpoco-dev clang-format libusb-1.0-0-dev libpcap-dev libsodium-dev libnl-3-dev libnl-genl-3-dev libnl-route-3-dev libsdl2-dev"
VIDEO_PACKAGES="libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-bad libv4l-dev"
BUILD_PACKAGES="git build-essential autotools-dev automake libtool python3-pip autoconf apt-transport-https ruby-dev cmake"


function install_pi_packages {
PLATFORM_PACKAGES="libcamera-openhd"
PLATFORM_PACKAGES_REMOVE="python3-libcamera libcamera0"
}
function install_x86_packages {
PLATFORM_PACKAGES="libunwind-dev gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly"
PLATFORM_PACKAGES_REMOVE=""
}
function install_rock_packages {
PLATFORM_PACKAGES="libpoco-dev gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly"
PLATFORM_PACKAGES_REMOVE=""
}

function extract_rock_gstreamer_dev_files {
    echo "Extracting GStreamer development files without Mesa dev dependencies..."
    tmpdir="$(mktemp -d)"
    (
        cd "${tmpdir}"
        apt-get clean
        apt-get download libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
        for deb in ./*.deb; do
            dpkg-deb -x "${deb}" /
        done
    )
    rm -rf "${tmpdir}"
    mkdir -p /usr/lib/pkgconfig
    find /usr/lib/aarch64-linux-gnu/pkgconfig -name 'gstreamer*.pc' -exec cp -a {} /usr/lib/pkgconfig/ \;
    export PKG_CONFIG_PATH="/usr/lib/pkgconfig:/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig:${PKG_CONFIG_PATH:-}"
    if ! pkg-config --exists gstreamer-1.0 gstreamer-app-1.0 gstreamer-sdp-1.0 gstreamer-video-1.0; then
        echo "GStreamer development pkg-config files are still unavailable"
        pkg-config --print-errors --exists gstreamer-1.0 gstreamer-app-1.0 gstreamer-sdp-1.0 gstreamer-video-1.0 || true
        find /usr -name 'gstreamer*.pc' -print
        exit 1
    fi
    apt-get clean
}

# Main function
 
 if [[ "${PLATFORM}" == "rpi" ]]; then
    install_pi_packages
 elif [[ "${PLATFORM}" == "ubuntu-x86" ]] ; then
    install_x86_packages
elif [[ "${PLATFORM}" == "rock5" ]] ; then
    install_rock_packages
    for package in clang-format libsdl2-dev; do
        BASE_PACKAGES="${BASE_PACKAGES/${package}/}"
    done
    VIDEO_PACKAGES="${VIDEO_PACKAGES/libgstreamer-plugins-base1.0-dev/}"
    VIDEO_PACKAGES="${VIDEO_PACKAGES} libglib2.0-dev liborc-0.4-dev"
else
    echo "platform not supported"
fi

 # Add OpenHD Repository only for platform-specific packages that need it.
 apt update
 if [[ "${PLATFORM}" == "rpi" ]]; then
     if ! command -v curl >/dev/null 2>&1; then
         apt-get clean
         apt-get install -y --no-upgrade --no-install-recommends curl
         apt-get clean
     fi
     curl -1sLf 'https://dl.cloudsmith.io/public/openhd/release/setup.deb.sh' | sudo -E bash
     apt update
 fi
 #apt upgrade -y -o Dpkg::Options::="--force-overwrite" --no-install-recommends --allow-downgrades


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
     apt-get clean
     apt-get install -y --no-upgrade -o Dpkg::Options::="--force-overwrite" --no-install-recommends ${package}
     if [ $? -ne 0 ]; then
         echo "Failed to install ${package}!"
         exit 1
     fi
     apt-get clean
 done

 if [[ "${PLATFORM}" == "rock5" ]]; then
     extract_rock_gstreamer_dev_files
 fi
 


# Installing ruby packages
# Work around a known conflict if another dotenv executable is already present.
if command -v dotenv >/dev/null 2>&1; then
    DOTENV_BIN="$(command -v dotenv)"
    if [ "${DOTENV_BIN}" = "/usr/local/bin/dotenv" ]; then
        echo "Removing conflicting ${DOTENV_BIN} before gem install"
        rm -f "${DOTENV_BIN}"
    fi
fi
gem install dotenv -v 2.8.1 --no-document
gem install fpm --no-document
