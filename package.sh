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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

CUSTOM="${1:-}"
PACKAGE_ARCH="${2:-}"
OS="${3:-}"

PKGDIR="/out/openhd-installdir/"
VERSION="3.0-alpha-$(date '+%Y%m%d%H%M')-$(git rev-parse --short HEAD)"

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
    if [[ "${PACKAGE_ARCH}" == "armhf" && "${OS}" == "raspbian" ]]; then
      echo "Using Raspberry Pi-specific systemd service for armhf Bullseye"
      cp systemd/openhd_rpi.service "${PKGDIR}etc/systemd/system/openhd.service"
    elif [[ "${CUSTOM}" == "standard" ]]; then
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

  # hardware.config support removed; no package copy needed.
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
        libsdl2-2.0-0 libsodium-dev gstreamer1.0-plugins-{base,good,bad,ugly}
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
      libsodium-dev gstreamer1.0-plugins-{base,good,bad,ugly}
      gstreamer1.0-{tools,alsa,pulseaudio}
    )
  fi

  if dpkg -l | grep -q "qti-gstreamer1.0-plugins-bad-waylandsink"; then
    package_name="${package_name}-QCom"
  fi

  source "${SCRIPT_DIR}/OpenHD/scripts/resolve_artosyn_sdk.sh"
  resolve_artosyn_sdk

  rm -f "${package_name}_${VERSION}_${PACKAGE_ARCH}.deb"
  cmake OpenHD/ \
    -DARTOSYN_SDK_ROOT="${ARTOSYN_SDK_ROOT}" \
    -DARTOSYN_SDK_LIB="${ARTOSYN_SDK_LIB}"
  make -j$(nproc)

  mkdir -p "${PKGDIR}usr/local/bin/"
  cp openhd "${PKGDIR}usr/local/bin/"
  mkdir -p "${PKGDIR}usr/local/lib/"

  local daemon_candidates=(
    "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/artosyn_daemon"
    "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/ar8030_daemon"
    "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/artlinkd"
    "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/bbd"
    "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/bb_daemon"
    "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/artosyn_daemon"
    "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/ar8030_daemon"
    "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/artlinkd"
    "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/bbd"
    "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/bb_daemon"
    "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/artosyn_daemon"
    "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/ar8030_daemon"
    "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/artlinkd"
    "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/bbd"
    "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/bb_daemon"
  )
  local daemon_src=""
  for candidate in "${daemon_candidates[@]}"; do
    if [[ -f "${candidate}" ]]; then
      daemon_src="${candidate}"
      break
    fi
  done
  if [[ -z "${daemon_src}" ]]; then
    if [[ -d "${ARTOSYN_SDK_ROOT}/host_drv" ]]; then
      daemon_src="$(find "${ARTOSYN_SDK_ROOT}/host_drv" -type f \
        \( -iname "artosyn_daemon" -o -iname "ar8030_daemon" -o -iname "artlinkd" -o -iname "bbd" -o -iname "bb_daemon" \) \
        | head -n 1 || true)"
    fi
  fi
  if [[ -z "${daemon_src}" ]]; then
    echo "Warning: Artosyn daemon binary not found in SDK root ${ARTOSYN_SDK_ROOT}" >&2
    echo "Runtime can still work if a system service provides the daemon." >&2
  else
    cp "${daemon_src}" "${PKGDIR}usr/local/bin/$(basename "${daemon_src}")"
    chmod +x "${PKGDIR}usr/local/bin/$(basename "${daemon_src}")"
  fi

  local copied_runtime_lib=0
  IFS=';' read -ra sdk_libs <<< "${ARTOSYN_SDK_LIB}"
  for lib in "${sdk_libs[@]}"; do
    if [[ -f "${lib}" && "${lib}" == *.so* ]]; then
      cp "${lib}" "${PKGDIR}usr/local/lib/"
      copied_runtime_lib=1
    fi
  done
  local extra_runtime_libs=(
    "${ARTOSYN_SDK_ROOT}/host_drv/com/libcom.so"
    "${ARTOSYN_SDK_ROOT}/host_drv/build/com/libcom.so"
    "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/libcom.so"
  )
  for lib in "${extra_runtime_libs[@]}"; do
    if [[ -f "${lib}" ]]; then
      cp "${lib}" "${PKGDIR}usr/local/lib/"
      copied_runtime_lib=1
    fi
  done
  if [[ "${copied_runtime_lib}" -eq 0 ]]; then
    echo "No Artosyn runtime .so copied (static linking may still be fine)." >&2
  fi

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
