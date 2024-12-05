#!/bin/bash

CUSTOM="${1}"
PACKAGE_ARCH="${2}"
OS="${3}"

PKGDIR="/out/openhd-installdir/"
VERSION="2.6.2-$(date '+%Y%m%d%H%M')-$(git rev-parse --short HEAD)"

# Function to create the package directory structure
create_package_directory() {
  rm -rf /tmp/openhd-installdir
  mkdir -p ${PKGDIR}usr/local/bin
  mkdir -p ${PKGDIR}tmp
  mkdir -p ${PKGDIR}settings
  mkdir -p ${PKGDIR}etc/systemd/system

  # Copy systemd service based on architecture and custom type
  if [[ "${PACKAGE_ARCH}" != "x86_64" ]]; then
    echo "Not x86 architecture"
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

# Copy hardware.config based on architecture and custom type
echo "Starting hardware.config copy process..."
echo "PACKAGE_ARCH=${PACKAGE_ARCH}"
echo "CUSTOM=${CUSTOM}"

if [[ "${PACKAGE_ARCH}" == "armhf" ]]; then
  echo "PACKAGE_ARCH matches armhf"
  
  if [[ "${CUSTOM}" == "standard" ]]; then
    echo "CUSTOM matches standard"
    
    mkdir -p "${PKGDIR}/boot/openhd/"
    echo "Created directory: ${PKGDIR}/boot/openhd/"
    
    if cp OpenHD/ohd_common/config/hardware.config "${PKGDIR}/boot/openhd/hardware.config"; then
      echo "Copied hardware.config to ${PKGDIR}/boot/openhd/"
    else
      echo "Error copying hardware.config to ${PKGDIR}/boot/openhd/"
      exit 1
    fi
    
  else
    echo "CUSTOM does not match standard"
    
    mkdir -p "${PKGDIR}/config/openhd/"
    echo "Created directory: ${PKGDIR}/config/openhd/"
    
    if cp OpenHD/ohd_common/config/hardware.config "${PKGDIR}/config/openhd/hardware.config"; then
      echo "Copied hardware.config to ${PKGDIR}/config/openhd/"
    else
      echo "Error copying hardware.config to ${PKGDIR}/config/openhd/"
      exit 1
    fi
    
  fi
  
else
  echo "PACKAGE_ARCH does not match armhf, skipping hardware.config copy"
fi


# Function to build the package
build_package() {

  # Set initial package name
  if [[ "${PACKAGE_ARCH}" == "armhf" ]]; then
    if [[ "${CUSTOM}" == "standard" ]]; then
      PACKAGE_NAME="openhd"
      PACKAGES="-d libpoco-dev -d libcamera-openhd -d gst-openhd-plugins -d iw -d nmap -d aircrack-ng -d i2c-tools -d libv4l-dev -d libusb-1.0-0 -d libpcap-dev -d libnl-3-dev -d libnl-genl-3-dev -d libsdl2-2.0-0 -d libsodium-dev -d gstreamer1.0-plugins-base -d gstreamer1.0-plugins-good -d gstreamer1.0-plugins-bad -d gstreamer1.0-plugins-ugly -d gstreamer1.0-libav -d gstreamer1.0-tools -d gstreamer1.0-alsa -d gstreamer1.0-pulseaudio"
    else
      PACKAGE_NAME="openhd-x20"
      PACKAGES="-d libpoco-dev -d iw -d i2c-tools -d libv4l-dev -d libusb-1.0-0 -d libpcap-dev -d libnl-3-dev -d libnl-genl-3-dev -d libsdl2-2.0-0 -d libsodium-dev -d gstreamer1.0-plugins-base -d gstreamer1.0-plugins-good -d gstreamer1.0-plugins-bad -d gstreamer1.0-tools"
    fi
  elif [[ "${PACKAGE_ARCH}" == "x86_64" ]]; then
    PACKAGE_NAME="openhd"
    PACKAGES="-d libpoco-dev -d dkms -d qopenhd -d git -d iw -d nmap -d aircrack-ng -d i2c-tools -d libv4l-dev -d libusb-1.0-0 -d libpcap-dev -d libnl-3-dev -d libnl-genl-3-dev -d libsdl2-2.0-0 -d libsodium-dev -d gstreamer1.0-plugins-base -d gstreamer1.0-plugins-good -d gstreamer1.0-plugins-bad -d gstreamer1.0-plugins-ugly -d gstreamer1.0-libav -d gstreamer1.0-tools -d gstreamer1.0-alsa -d gstreamer1.0-pulseaudio"
  else
    PACKAGE_NAME="openhd"
    PACKAGES="-d libpoco-dev -d iw -d nmap -d aircrack-ng -d i2c-tools -d libv4l-dev -d libusb-1.0-0 -d libpcap-dev -d libnl-3-dev -d libnl-genl-3-dev -d libsdl2-2.0-0 -d libsodium-dev -d gstreamer1.0-plugins-base -d gstreamer1.0-plugins-good -d gstreamer1.0-plugins-bad -d gstreamer1.0-plugins-ugly -d gstreamer1.0-libav -d gstreamer1.0-tools -d gstreamer1.0-alsa -d gstreamer1.0-pulseaudio"
  fi

  # Check for the presence of qti-gstreamer1.0-plugins-bad-waylandsink and add -QCom to package name if installed
  if dpkg -l | grep -q "qti-gstreamer1.0-plugins-bad-waylandsink"; then
    PACKAGE_NAME="${PACKAGE_NAME}-QCom"
  fi

  rm "${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb" > /dev/null 2>&1 || true
  cmake OpenHD/
  make -j4
  mkdir -p ${PKGDIR}/usr/local/bin/
  cp openhd ${PKGDIR}/usr/local/bin/ || exit 1

  # Build the package using fpm
  fpm -a "${PACKAGE_ARCH}" -s dir -t deb -n "${PACKAGE_NAME}" -v "${VERSION}" -C "${PKGDIR}" \
    -p "${PACKAGE_NAME}_${VERSION}_${PACKAGE_ARCH}.deb" \
    --after-install after-install.sh \
    --before-install before-install.sh \
    ${PACKAGES}
  
  # Copy the resulting .deb package to the output directory
  cp *.deb /out/
}

# Main execution
create_package_directory
build_package
