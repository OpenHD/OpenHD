#!/bin/bash

CUSTOM="${1}"
PACKAGE_ARCH="${2}"
OS="${3}"

PKGDIR="/out/openhd-installdir/"
VERSION="2.5.4-rc-$(date '+%Y%m%d%H%M')-$(git rev-parse --short HEAD)"

create_package_directory() {
  rm -rf /tmp/openhd-installdir
  mkdir -p ${PKGDIR}usr/local/bin
  mkdir -p ${PKGDIR}tmp
  #Nobody should create a topdir for such things like settings, but for now it'll stay
  mkdir -p ${PKGDIR}settings
  mkdir -p ${PKGDIR}etc/systemd/system

  # We do not copy the openhd service for x86, since there we have launcher on the desktop
  # (Otherwise, we always copy it)
  if [[ "${PACKAGE_ARCH}" != "x86_64" ]]; then
    echo "we're not on x86"
      if [[ "${CUSTOM}" == "standard" ]]; then
      cp systemd/openhd.service ${PKGDIR}etc/systemd/system/openhd.service || exit 1
      else
      cp systemd/openhd-x20.service ${PKGDIR}etc/systemd/system/openhd.service || exit 1
      fi
    else
      mkdir -p ${PKGDIR}usr/share/applications/
      cp shortcuts/* ${PKGDIR}usr/share/applications/
      mkdir -p ${PKGDIR}usr/local/share/openhd_misc/
      cp shortcuts/OpenHD.ico ${PKGDIR}usr/local/share/openhd_misc/
      mkdir -p ${PKGDIR}etc/profile.d/
      cp desktop-truster.sh ${PKGDIR}etc/profile.d/
      sudo chmod +777 ${PKGDIR}etc/profile.d/desktop-truster.sh
  fi
  # always - copy the hardware.config file
  mkdir -p "${PKGDIR}/boot/openhd/"
  cp OpenHD/ohd_common/config/hardware.config "${PKGDIR}/boot/openhd/hardware.config" || exit 1
}
build_package() {

  if [[ "${PACKAGE_ARCH}" == "armhf" ]]; then
    if [[ "${CUSTOM}" == "standard" ]]; then
      PACKAGE_NAME="openhd"
      PACKAGES="-d libcamera-openhd -d gst-openhd-plugins -d iw -d nmap -d aircrack-ng -d i2c-tools -d libv4l-dev -d libusb-1.0-0 -d libpcap-dev -d libnl-3-dev -d libnl-genl-3-dev -d libsdl2-2.0-0 -d libsodium-dev -d gstreamer1.0-plugins-base -d gstreamer1.0-plugins-good -d gstreamer1.0-plugins-bad -d gstreamer1.0-plugins-ugly -d gstreamer1.0-libav -d gstreamer1.0-tools -d gstreamer1.0-alsa -d gstreamer1.0-pulseaudio"
      PLATFORM_CONFIGS=""
    else
      PACKAGE_NAME="openhd-x20"
      PACKAGES="-d iw -d i2c-tools -d libv4l-dev -d libusb-1.0-0 -d libpcap-dev -d libnl-3-dev -d libnl-genl-3-dev -d libsdl2-2.0-0 -d libsodium-dev -d gstreamer1.0-plugins-base -d gstreamer1.0-plugins-good -d gstreamer1.0-plugins-bad -d gstreamer1.0-tools"
      PLATFORM_CONFIGS=""
    fi
  elif [[ "${PACKAGE_ARCH}" == "x86_64" ]]; then
    PACKAGE_NAME="openhd"
    PACKAGES="-d dkms -d qopenhd -d git -d iw -d nmap -d aircrack-ng -d i2c-tools -d libv4l-dev -d libusb-1.0-0 -d libpcap-dev -d libnl-3-dev -d libnl-genl-3-dev -d libsdl2-2.0-0 -d libsodium-dev -d gstreamer1.0-plugins-base -d gstreamer1.0-plugins-good -d gstreamer1.0-plugins-bad -d gstreamer1.0-plugins-ugly -d gstreamer1.0-libav -d gstreamer1.0-tools -d gstreamer1.0-alsa -d gstreamer1.0-pulseaudio"
    PLATFORM_CONFIGS=""
  else
    PACKAGE_NAME="openhd"
    PACKAGES="-d iw -d nmap -d aircrack-ng -d i2c-tools -d libv4l-dev -d libusb-1.0-0 -d libpcap-dev -d libnl-3-dev -d libnl-genl-3-dev -d libsdl2-2.0-0 -d libsodium-dev -d gstreamer1.0-plugins-base -d gstreamer1.0-plugins-good -d gstreamer1.0-plugins-bad -d gstreamer1.0-plugins-ugly -d gstreamer1.0-libav -d gstreamer1.0-tools -d gstreamer1.0-alsa -d gstreamer1.0-pulseaudio"
    PLATFORM_CONFIGS=""
  fi

  rm "${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb" > /dev/null 2>&1 || true
  cmake OpenHD/
  make -j4
  mkdir -p ${PKGDIR}/usr/local/bin/openhd
  cp openhd ${PKGDIR}/usr/local/bin/openhd || exit 1
  echo "$PWD"
  ls -a  
   # Assuming fpm is installed and properly configured
  fpm -a "${PACKAGE_ARCH}" -s dir -t deb -n "${PACKAGE_NAME}" -v "${VERSION}" -C "${PKGDIR}" \
    ${PLATFORM_CONFIGS} \
    -p "${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb" \
    --after-install after-install.sh \
    --before-install before-install.sh \
    ${PACKAGES}
  ls -a
  cp *.deb /out/
}
#Main Build
create_package_directory
build_package
