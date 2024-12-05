#!/bin/bash
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

set -euo pipefail  # Enable strict error handling

CUSTOM="${1:-}"
PACKAGE_ARCH="${2:-}"
OS="${3:-}"

PKGDIR="/out/openhd-installdir/"
VERSION="2.6.2-$(date '+%Y%m%d%H%M')-$(git rev-parse --short HEAD)"

# Function to create the package directory structure
create_package_directory() {
  echo "Creating package directory structure..."
  rm -rf /tmp/openhd-installdir
  mkdir -p \
    "${PKGDIR}usr/local/bin" \
    "${PKGDIR}tmp" \
    "${PKGDIR}settings" \
    "${PKGDIR}etc/systemd/system"

  if [[ "${PACKAGE_ARCH}" != "x86_64" ]]; then
    echo "Non-x86 architecture detected"
    if [[ "${CUSTOM}" == "standard" ]]; then
      cp systemd/openhd.service "${PKGDIR}etc/systemd/system/openhd.service"
    else
      cp systemd/openhd-x20.service "${PKGDIR}etc/systemd/system/openhd.service"
    fi
  else
    mkdir -p "${PKGDIR}usr/share/applications/"
    cp shortcuts/* "${PKGDIR}usr/share/applications/"
    mkdir -p "${PKGDIR}usr/local/share/openhd_misc/"
    cp shortcuts/OpenHD.ico "${PKGDIR}usr/local/share/openhd_misc/"
    mkdir -p "${PKGDIR}etc/profile.d/"
    cp desktop-truster.sh "${PKGDIR}etc/profile.d/"
    chmod 777 "${PKGDIR}etc/profile.d/desktop-truster.sh"
  fi

  # Copy hardware.config based on architecture and custom type
  echo "Copying hardware.config..."
  if [[ "${PACKAGE_ARCH}" == "armhf" ]]; then
    local config_dir
    if [[ "${CUSTOM}" == "standard" ]]; then
      config_dir="${PKGDIR}/boot/openhd/"
    else
      config_dir="${PKGDIR}/config/openhd/"
    fi
    mkdir -p "${config_dir}"
    cp OpenHD/ohd_common/config/hardware.config "${config_dir}hardware.config"
  else
    echo "Skipping hardware.config copy for non-armhf architecture"
  fi
}

# Function to build the package
build_package() {
  echo "Building package..."
  local package_name="openhd"
  local packages=()

  if [[ "${PACKAGE_ARCH}" == "armhf" ]]; then
    if [[ "${CUSTOM}" == "standard" ]]; then
      package_name="openhd"
      packages+=(
        libpoco-dev libcamera-openhd gst-openhd-plugins iw nmap aircrack-ng
        i2c-tools libv4l-dev libusb-1.0-0 libpcap-dev libnl-3-dev libnl-genl-3-dev
        libsdl2-2.0-0 libsodium-dev gstreamer1.0-plugins-{base,good,bad,ugly,libav}
        gstreamer1.0-{tools,alsa,pulseaudio}
      )
    else
      package_name="openhd-x20"
      packages+=(
        libpoco-dev iw i2c-tools libv4l-dev libusb-1.0-0 libpcap-dev
        libnl-3-dev libnl-genl-3-dev libsdl2-2.0-0 libsodium-dev
        gstreamer1.0-plugins-{base,good,bad} gstreamer1.0-tools
      )
    fi
  elif [[ "${PACKAGE_ARCH}" == "x86_64" ]]; then
    packages+=(
      libpoco-dev dkms qopenhd git iw nmap aircrack-ng i2c-tools libv4l-dev
      libusb-1.0-0 libpcap-dev libnl-3-dev libnl-genl-3-dev libsdl2-2.0-0
      libsodium-dev gstreamer1.0-plugins-{base,good,bad,ugly,libav}
      gstreamer1.0-{tools,alsa,pulseaudio}
    )
  fi

  if dpkg -l | grep -q "qti-gstreamer1.0-plugins-bad-waylandsink"; then
    package_name="${package_name}-QCom"
  fi

  rm -f "${package_name}_${VERSION}_${PACKAGE_ARCH}.deb"
  cmake OpenHD/
  make -j$(nproc)

  mkdir -p "${PKGDIR}usr/local/bin/"
  cp openhd "${PKGDIR}usr/local/bin/"

  # Build the package using fpm
  fpm -a "${PACKAGE_ARCH}" -s dir -t deb -n "${package_name}" -v "${VERSION}" -C "${PKGDIR}" \
    -p "${package_name}_${VERSION}_${PACKAGE_ARCH}.deb" \
    --after-install after-install.sh \
    --before-install before-install.sh \
    -d "$(IFS=','; echo "${packages[*]}")"

  cp *.deb /out/
}

# Main execution
create_package_directory
build_package
