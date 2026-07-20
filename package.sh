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

join_by() {
  local IFS="$1"
  shift
  echo "$*"
}

build_deb_package() {
  local package_name="$1"
  local package_arch="$2"
  shift 2
  local dependencies=("$@")
  local debian_dir="${PKGDIR}DEBIAN"

  rm -rf "${debian_dir}"
  mkdir -p "${debian_dir}"
  chmod 0755 "${debian_dir}"
  {
    echo "Package: ${package_name}"
    echo "Version: ${VERSION}"
    echo "Section: misc"
    echo "Priority: optional"
    echo "Architecture: ${package_arch}"
    echo "Maintainer: OpenHD <openhd@openhdfpv.org>"
    if [[ "${#dependencies[@]}" -gt 0 ]]; then
      echo "Depends: $(join_by ', ' "${dependencies[@]}")"
    fi
    echo "Description: OpenHD runtime package"
    echo " OpenHD runtime files and service definitions."
  } >"${debian_dir}/control"

  if [[ -f before-install.sh ]]; then
    cp before-install.sh "${debian_dir}/preinst"
    chmod 0755 "${debian_dir}/preinst"
  fi
  if [[ -f after-install.sh ]]; then
    cp after-install.sh "${debian_dir}/postinst"
    chmod 0755 "${debian_dir}/postinst"
  fi

  dpkg-deb --build "${PKGDIR}" "${package_name}_${VERSION}_${package_arch}.deb"
}

require_staged_gst_perf() {
  if [[ "${OPENHD_BUNDLE_GST_PERF:-1}" == "0" ]]; then
    return
  fi
  local matches=()
  mapfile -t matches < <(find "${PKGDIR}usr/lib" -path "*/gstreamer-1.0/libgstperf.so" -type f 2>/dev/null)
  if [[ "${#matches[@]}" -ne 1 ]]; then
    echo "Expected exactly one staged gst-perf plugin in package tree, found ${#matches[@]}." >&2
    find "${PKGDIR}" -path "*/gstreamer-1.0/*" -type f -print >&2 || true
    exit 1
  fi
  echo "Verified staged gst-perf plugin: ${matches[0]}"
}

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

build_and_stage_gst_perf() {
  if [[ "${OPENHD_BUNDLE_GST_PERF:-1}" == "0" ]]; then
    echo "Skipping bundled gst-perf because OPENHD_BUNDLE_GST_PERF=0."
    return
  fi

  local multiarch
  multiarch="$(dpkg-architecture -qDEB_HOST_MULTIARCH)"
  if [[ -z "${multiarch}" ]]; then
    echo "Could not determine Debian multiarch triplet for gst-perf install." >&2
    exit 1
  fi

  local gst_perf_repo="${GST_PERF_REPO:-https://github.com/RidgeRun/gst-perf.git}"
  local gst_perf_ref="${GST_PERF_REF:-}"
  local gst_perf_src="/out/gst-perf-src"
  local gst_perf_prefix="/out/gst-perf-install"
  local make_jobs
  make_jobs="$(nproc 2>/dev/null || echo 1)"
  rm -rf "${gst_perf_src}" "${gst_perf_prefix}"

  echo "Building bundled gst-perf from ${gst_perf_repo}${gst_perf_ref:+ at ${gst_perf_ref}}..."
  git clone --depth 1 "${gst_perf_repo}" "${gst_perf_src}"
  if [[ -n "${gst_perf_ref}" ]]; then
    git -C "${gst_perf_src}" fetch --depth 1 origin "${gst_perf_ref}"
    git -C "${gst_perf_src}" checkout FETCH_HEAD
  fi

  (
    cd "${gst_perf_src}"
    ./autogen.sh
    ./configure --prefix=/usr --libdir="/usr/lib/${multiarch}"
    make -j"${make_jobs}"
    make install DESTDIR="${gst_perf_prefix}"
  )

  local plugin_src="${gst_perf_prefix}/usr/lib/${multiarch}/gstreamer-1.0/libgstperf.so"
  if [[ ! -f "${plugin_src}" ]]; then
    echo "gst-perf build did not produce ${plugin_src}" >&2
    find "${gst_perf_prefix}" -maxdepth 6 -type f -print >&2 || true
    exit 1
  fi

  local plugin_dst="${PKGDIR}usr/lib/${multiarch}/gstreamer-1.0"
  mkdir -p "${plugin_dst}"
  install -m 0644 "${plugin_src}" "${plugin_dst}/libgstperf.so"

  mkdir -p "${PKGDIR}usr/share/doc/openhd"
  if [[ -f "${gst_perf_src}/LICENSE" ]]; then
    install -m 0644 "${gst_perf_src}/LICENSE" \
      "${PKGDIR}usr/share/doc/openhd/gst-perf-LICENSE"
  fi

  echo "Bundled gst-perf plugin staged at ${plugin_dst}/libgstperf.so"
}


# Function to build the package
build_package() {
  echo "Building package..."
  local package_name="openhd"
  local packages=()

  if [[ "${PACKAGE_ARCH}" == "armhf" ||
        ( "${PACKAGE_ARCH}" == "arm64" && "${OS}" == "raspbian" ) ]]; then
    if [[ "${CUSTOM}" == "standard" ]]; then
      package_name="openhd"
      packages+=(
        libpoco-dev iw nmap aircrack-ng i2c-tools libv4l-dev libusb-1.0-0
        libpcap-dev libnl-3-dev libnl-genl-3-dev
        libsdl2-2.0-0 libsodium-dev gstreamer1.0-plugins-{base,good,bad,ugly}
        gstreamer1.0-{tools,alsa,pulseaudio}
      )
      if [[ "${PACKAGE_ARCH}" == "arm64" ]]; then
        packages+=(gstreamer1.0-libcamera gstreamer1.0-libav openhd-sys-utils)
      else
        packages+=(libcamera-openhd gst-openhd-plugins)
      fi
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
  local artosyn_enabled=0
  local require_artosyn="${OPENHD_REQUIRE_ARTOSYN:-0}"
  local require_artosyn_daemon="${OPENHD_REQUIRE_ARTOSYN_DAEMON:-0}"
  local current_branch=""
  current_branch="$(git rev-parse --abbrev-ref HEAD 2>/dev/null || true)"
  if [[ "${current_branch}" == "openhd-3.0" || "${current_branch}" == "openhd-3.0-test" ]]; then
    require_artosyn="${OPENHD_REQUIRE_ARTOSYN:-1}"
    require_artosyn_daemon="${OPENHD_REQUIRE_ARTOSYN_DAEMON:-1}"
  fi
  if [[ -n "${ARTOSYN_SDK_ROOT:-}" && -n "${ARTOSYN_SDK_LIB:-}" ]]; then
    artosyn_enabled=1
  fi
  if [[ "${require_artosyn}" == "1" && "${artosyn_enabled}" -ne 1 ]]; then
    echo "Artosyn SDK is required for this build, but resolver did not provide ARTOSYN_SDK_ROOT/ARTOSYN_SDK_LIB." >&2
    exit 1
  fi

  rm -f "${package_name}_${VERSION}_${PACKAGE_ARCH}.deb"
  local build_dir="/out/openhd-build"
  local build_tmp="/out/openhd-build-tmp"
  rm -rf "${build_dir}" "${build_tmp}"
  mkdir -p "${build_dir}" "${build_tmp}"
  export TMPDIR="${build_tmp}"

  cmake -S OpenHD/ -B "${build_dir}" \
    -DARTOSYN_SDK_ROOT="${ARTOSYN_SDK_ROOT}" \
    -DARTOSYN_SDK_LIB="${ARTOSYN_SDK_LIB}" \
    -DARTOSYN_SDK_DAEMON="${ARTOSYN_SDK_DAEMON:-}" \
    -DARTOSYN_SDK_TUNTAP="${ARTOSYN_SDK_TUNTAP:-}"
  cmake --build "${build_dir}" --parallel "$(nproc)"

  mkdir -p "${PKGDIR}usr/local/bin/"
  cp "${build_dir}/openhd" "${PKGDIR}usr/local/bin/"
  mkdir -p "${PKGDIR}usr/local/lib/"
  build_and_stage_gst_perf

  if [[ "${artosyn_enabled}" -eq 1 ]]; then
    local daemon_candidates=(
      "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/artosyn_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/ar8030_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/artlinkd"
      "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/bbd"
      "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/bb_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/daemon/daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/artosyn_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/ar8030_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/artlinkd"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/bbd"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/bb_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/daemon/daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/artosyn_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/ar8030_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/artlinkd"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/bbd"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/bb_daemon"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/daemon"
    )
    local daemon_src="${ARTOSYN_SDK_DAEMON:-}"
    if [[ -n "${daemon_src}" && ! -f "${daemon_src}" ]]; then
      daemon_src=""
    fi
    for candidate in "${daemon_candidates[@]}"; do
      if [[ -z "${daemon_src}" && -f "${candidate}" ]]; then
        daemon_src="${candidate}"
        break
      fi
    done
    if [[ -z "${daemon_src}" && -d "${ARTOSYN_SDK_ROOT}/host_drv" ]]; then
      daemon_src="$(find "${ARTOSYN_SDK_ROOT}/host_drv" -type f \
        \( -iname "artosyn_daemon" -o -iname "ar8030_daemon" -o -iname "artlinkd" -o -iname "bbd" -o -iname "bb_daemon" -o -iname "daemon" \) \
        | head -n 1 || true)"
    fi
    if [[ -n "${daemon_src}" ]]; then
      local daemon_dst_name
      daemon_dst_name="$(basename "${daemon_src}")"
      if [[ "${daemon_dst_name}" == "daemon" ]]; then
        daemon_dst_name="artosyn_daemon"
      fi
      cp "${daemon_src}" "${PKGDIR}usr/local/bin/${daemon_dst_name}"
      chmod +x "${PKGDIR}usr/local/bin/${daemon_dst_name}"
    else
      if [[ "${require_artosyn_daemon}" == "1" ]]; then
        echo "Artosyn daemon is required for this build, but daemon binary was not found in SDK." >&2
        exit 1
      fi
      echo "Artosyn SDK detected but daemon binary not found; continuing without daemon install." >&2
    fi

    local tuntap_candidates=(
      "${ARTOSYN_SDK_TUNTAP:-}"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/dev_helper/tuntap_bb"
      "${ARTOSYN_SDK_ROOT}/host_drv/dev_helper/tuntap_bb"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/dev_helper/tuntap_bb"
    )
    local tuntap_src=""
    for candidate in "${tuntap_candidates[@]}"; do
      if [[ -n "${candidate}" && -f "${candidate}" ]]; then
        tuntap_src="${candidate}"
        break
      fi
    done
    if [[ -z "${tuntap_src}" && -d "${ARTOSYN_SDK_ROOT}/host_drv" ]]; then
      tuntap_src="$(find "${ARTOSYN_SDK_ROOT}/host_drv" -type f -name "tuntap_bb" | head -n 1 || true)"
    fi
    if [[ -n "${tuntap_src}" ]]; then
      cp "${tuntap_src}" "${PKGDIR}usr/local/bin/tuntap_bb"
      chmod +x "${PKGDIR}usr/local/bin/tuntap_bb"
    else
      echo "Artosyn SDK detected but tuntap_bb binary not found; continuing without LAN tunnel helper install." >&2
    fi

    local copied_runtime_lib=0
    IFS=';' read -ra sdk_libs <<< "${ARTOSYN_SDK_LIB}"
    for lib in "${sdk_libs[@]}"; do
      if [[ -f "${lib}" && ( "${lib}" == *.so* || "${lib}" == *.a ) ]]; then
        cp "${lib}" "${PKGDIR}usr/local/lib/"
        copied_runtime_lib=1
      fi
    done
    local extra_runtime_libs=(
      "${ARTOSYN_SDK_ROOT}/host_drv/com/libcom.so"
      "${ARTOSYN_SDK_ROOT}/host_drv/com/libcom.a"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/com/libcom.so"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/com/libcom.a"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/libcom.so"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/libcom.a"
    )
    for lib in "${extra_runtime_libs[@]}"; do
      if [[ -f "${lib}" ]]; then
        cp "${lib}" "${PKGDIR}usr/local/lib/"
        copied_runtime_lib=1
      fi
    done
    local extra_runtime_dirs=(
      "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030"
      "${ARTOSYN_SDK_ROOT}/host_drv/install/bin"
      "${ARTOSYN_SDK_ROOT}/host_drv/com"
      "${ARTOSYN_SDK_ROOT}/host_drv/build/com"
    )
    local lib_dir
    for lib_dir in "${extra_runtime_dirs[@]}"; do
      if [[ -d "${lib_dir}" ]]; then
        while IFS= read -r lib; do
          cp "${lib}" "${PKGDIR}usr/local/lib/"
          copied_runtime_lib=1
        done < <(find "${lib_dir}" -maxdepth 3 -type f \( -name "*.so" -o -name "*.so.*" -o -name "*.a" \) | sort -u)
      fi
    done
    if [[ "${copied_runtime_lib}" -eq 0 ]]; then
      echo "No Artosyn SDK libraries (.so/.a) copied." >&2
    fi
  else
    echo "Artosyn SDK not resolved; skipping Artosyn daemon/runtime library packaging." >&2
  fi

  if command -v fpm >/dev/null 2>&1; then
    # Build the package using fpm
    require_staged_gst_perf
    fpm -a "${PACKAGE_ARCH}" -s dir -t deb -n "${package_name}" -v "${VERSION}" -C "${PKGDIR}" \
      -p "${package_name}_${VERSION}_${PACKAGE_ARCH}.deb" \
      --after-install after-install.sh \
      --before-install before-install.sh \
      -d "$(IFS=','; echo "${packages[*]}")"
  else
    echo "fpm not available; building package with dpkg-deb."
    require_staged_gst_perf
    build_deb_package "${package_name}" "${PACKAGE_ARCH}" "${packages[@]}"
  fi

  cp *.deb /out/
}

# Main execution
create_package_directory
build_package
