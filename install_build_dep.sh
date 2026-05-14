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

BASE_PACKAGES="libpoco-dev clang-format libusb-1.0-0-dev libpcap-dev libsodium-dev libnl-3-dev libnl-genl-3-dev libnl-route-3-dev libsdl2-dev libopencv-dev"
VIDEO_PACKAGES="libgstreamer-plugins-base1.0-dev libv4l-dev"
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

# [핵심 수정] 패키지 서버를 연결하기 전에, 타겟 플랫폼을 먼저 확인합니다.
if [[ "${PLATFORM}" == "rpi" ]]; then
   install_pi_packages
   # Debian 환경이지만, 서버에서 Raspbian 전용 패키지를 받아오도록 강제 설정
   export os=raspbian
   export dist=bullseye
elif [[ "${PLATFORM}" == "ubuntu-x86" ]] ; then
   install_x86_packages
elif [[ "${PLATFORM}" == "rock5" ]] ; then
   install_rock_packages
else
   echo "platform not supported"
fi

# Add OpenHD Repository platform-specific packages
apt update
apt install -y curl
# 위에서 설정한 os=raspbian 환경변수가 적용된 상태로 서버 스크립트 실행
curl -1sLf 'https://dl.cloudsmith.io/public/openhd/release/setup.deb.sh'| bash
apt update
apt upgrade -y -o Dpkg::Options::="--force-overwrite" --no-install-recommends --allow-downgrades

# Remove platform-specific packages (지우기 에러 무시 로직 유지)
echo "Removing platform-specific packages..."
for package in ${PLATFORM_PACKAGES_REMOVE}; do
    echo "Removing ${package}..."
    apt purge -y ${package} || echo "Warning: ${package} 패키지를 지울 수 없지만 무시하고 진행합니다."
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
gem install dotenv -v 2.8.1
gem install fpm