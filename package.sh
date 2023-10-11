#!/bin/bash

set -euo pipefail

PACKAGE_ARCH="${1}"
OS="${2}"

PACKAGE_NAME="openhd"
PKGDIR="/tmp/${PACKAGE_NAME}-installdir"
VERSION="2.5.1-evo-alpha-$(date '+%Y%m%d%H%M')-$(git rev-parse --short HEAD)"

create_package_directory() {
  rm -rf "${PKGDIR}"
  mkdir -p "${PKGDIR}"/{usr/local/bin,tmp,settings,etc/systemd/system}
  # rpi only - copy the camera config files
  if [[ "${OS}" == "raspbian" ]]; then
    mkdir -p "${PKGDIR}/boot/openhd/rpi_camera_configs"
    cp -r rpi_camera_configs/* "${PKGDIR}/boot/openhd/rpi_camera_configs/" || exit 1
  elif [[ "${OS}" == "debian" ]]; then
    mkdir -p "${PKGDIR}/boot/openhd/rock5_camera_configs"
    cp -r rock5_camera_configs/* "${PKGDIR}/boot/openhd/rock5_camera_configs/" || exit 1
  fi
  # We do not copy the openhd service for x86, since there we have launcher on the desktop
  # (Otherwise, we always copy it)
  if [[ "${PACKAGE_ARCH}" != "x86_64" ]]; then
    cp systemd/openhd.service "${PKGDIR}/etc/systemd/system/openhd.service" || exit 1
  fi
  # always - copy the hardware.config file, the custom unmanaged camera service and the .sh file for it
  mkdir -p "${PKGDIR}/boot/openhd/"
  cp OpenHD/ohd_common/config/hardware.config "${PKGDIR}/boot/openhd/hardware.config" || exit 1
  mkdir -p "${PKGDIR}/boot/openhd/scripts/"
  cp scripts/custom_unmanaged_camera.sh ${PKGDIR}/boot/openhd/scripts/ || exit 1
  cp systemd/custom_unmanaged_camera.service "${PKGDIR}/etc/systemd/system/custom_unmanaged_camera.service" || exit 1
}

build_package() {
  rm "${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb" > /dev/null 2>&1 || true
  
  # cmake OpenHD/
  # make -j4
  touch openhd
  cp openhd ${PKGDIR}/usr/local/bin/openhd

  if [[ "${PACKAGE_ARCH}" == "armhf" ]]; then
    PLATFORM_PACKAGES="-d libcamera-openhd -d gst-openhd-plugins"
    PLATFORM_CONFIGS=""
  else
    PLATFORM_PACKAGES=""
    PLATFORM_CONFIGS=""
  fi

  fpm -a "${PACKAGE_ARCH}" -s dir -t deb -n "${PACKAGE_NAME}" -v "${VERSION}" -C "${PKGDIR}" \
    ${PLATFORM_CONFIGS} \
    -p "${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb" \
    --after-install after-install.sh \
    --before-install before-install.sh \
    ${PLATFORM_PACKAGES} \
    -d "iw" \
    -d "nmap" \
    -d "aircrack-ng" \
    -d "i2c-tools" \
    -d "libv4l-dev" \
    -d "libusb-1.0-0" \
    -d "libpcap-dev" \
    -d "libnl-3-dev" \
    -d "libnl-genl-3-dev" \
    -d "libsdl2-2.0-0" \
    -d "libsodium-dev" \
    -d "gstreamer1.0-plugins-base" \
    -d "gstreamer1.0-plugins-good" \
    -d "gstreamer1.0-plugins-bad" \
    -d "gstreamer1.0-plugins-ugly" \
    -d "gstreamer1.0-libav" \
    -d "gstreamer1.0-tools" \
    -d "gstreamer1.0-alsa" \
    -d "gstreamer1.0-pulseaudio"
}

  #Main Build
  create_package_directory
  build_package
