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
TARGET_RELEASE="${4:-}"

PKGDIR="/out/openhd-installdir/"
VERSION="3.0-alpha-$(date '+%Y%m%d%H%M')-$(git rev-parse --short HEAD)"

join_by() {
  local IFS="$1"
  shift
  echo "$*"
}

resolve_system_poco_dir() {
  local poco_config=""
  poco_config="$(dpkg-query -L libpoco-dev 2>/dev/null \
    | awk '/\/PocoConfig\.cmake$/ { print; exit }')"
  if [[ -z "${poco_config}" || ! -f "${poco_config}" ]]; then
    echo "Cannot locate the distro-provided PocoConfig.cmake from libpoco-dev." >&2
    exit 1
  fi
  dirname "${poco_config}"
}

append_elf_runtime_dependencies() {
  local binary="$1"
  local dependency_array_name="$2"
  local -n dependency_array="${dependency_array_name}"
  local ldd_output=""

  ldd_output="$(
    LD_LIBRARY_PATH="${PKGDIR}usr/local/lib:${LD_LIBRARY_PATH:-}" \
      ldd "${binary}"
  )"
  echo "${ldd_output}"
  if grep -q "not found" <<<"${ldd_output}"; then
    echo "OpenHD has unresolved shared-library dependencies." >&2
    exit 1
  fi

  local soname=""
  while IFS= read -r soname; do
    [[ -n "${soname}" ]] || continue
    local library=""
    library="$(
      awk -v soname="${soname}" '
        $1 == soname && $2 == "=>" && $3 ~ /^\// { print $3; exit }
        $1 == soname && $2 ~ /^\// { print $2; exit }
        $1 ~ /^\// {
          direct_name = $1
          sub(/^.*\//, "", direct_name)
          if (direct_name == soname) { print $1; exit }
        }
      ' <<<"${ldd_output}"
    )"
    if [[ -z "${library}" ]]; then
      echo "Direct shared library ${soname} could not be resolved for ${binary}." >&2
      exit 1
    fi
    if [[ "${library}" == "${PKGDIR}"* ]]; then
      # This dependency is bundled inside the OpenHD package and its own
      # external dependencies are validated when that staged ELF is scanned.
      continue
    fi
    local resolved_library=""
    local owner=""
    resolved_library="$(readlink -f "${library}")"
    owner="$(dpkg-query -S "${resolved_library}" 2>/dev/null \
      | awk -F': ' 'NR == 1 { sub(/:[^:]+$/, "", $1); print $1 }' || true)"
    if [[ -z "${owner}" ]]; then
      owner="$(dpkg-query -S "${library}" 2>/dev/null \
        | awk -F': ' 'NR == 1 { sub(/:[^:]+$/, "", $1); print $1 }' || true)"
    fi
    if [[ -z "${owner}" ]]; then
      echo "Shared library ${library} is not owned by a Debian package." >&2
      exit 1
    fi
    dependency_array+=("${owner}")
  done < <(readelf -d "${binary}" \
    | awk -F'[][]' '/NEEDED/ { print $2 }' | sort -u)
}

verify_packaged_poco_abi() {
  local binary="$1"
  local expected_soversion=""
  case "${TARGET_RELEASE}" in
    bookworm)
      expected_soversion="80"
      ;;
    bullseye)
      expected_soversion="70"
      ;;
  esac

  local needed=""
  needed="$(readelf -d "${binary}" \
    | awk -F'[][]' '/NEEDED.*libPoco(Foundation|Net)\.so/ { print $2 }')"
  echo "OpenHD Poco ELF dependencies:"
  echo "${needed}"
  if [[ -z "${needed}" ]]; then
    echo "OpenHD does not declare its expected Poco dependencies." >&2
    exit 1
  fi
  if [[ -n "${expected_soversion}" ]] &&
     grep -Evq "\.so\.${expected_soversion}$" <<<"${needed}"; then
    echo "OpenHD was linked against the wrong Poco ABI for ${TARGET_RELEASE}; expected .so.${expected_soversion}." >&2
    exit 1
  fi
}

resolve_package_name() {
  local package_arch="$1"
  local custom="$2"

  if [[ "${package_arch}" == "armhf" && "${custom}" != "standard" ]]; then
    echo "openhd-x20"
    return
  fi
  echo "openhd"
}

normalize_debian_arch() {
  case "$1" in
    x86_64)
      echo "amd64"
      ;;
    *)
      echo "$1"
      ;;
  esac
}

bundle_poco_runtime() {
  local binary="$1"
  local bundle_dir="${PKGDIR}usr/local/lib"
  local soname=""
  local library=""
  local bundled=0

  mkdir -p "${bundle_dir}"
  while IFS= read -r soname; do
    [[ -n "${soname}" ]] || continue
    if [[ -n "${OPENHD_SYSROOT:-}" ]]; then
      library="$(find "${OPENHD_SYSROOT}" -name "${soname}" -print -quit)"
    else
      library="$(
        ldd "${binary}" \
          | awk -v soname="${soname}" '
              $1 == soname && $2 == "=>" && $3 ~ /^\// { print $3; exit }
            '
      )"
    fi
    if [[ -z "${library}" || ! -f "${library}" ]]; then
      echo "Cannot resolve ${soname} for portable OpenHD package." >&2
      exit 1
    fi
    install -m 0644 "$(readlink -f "${library}")" "${bundle_dir}/${soname}"
    echo "Bundled ${soname} from ${library}"
    bundled=$((bundled + 1))
  done < <(
    readelf -d "${binary}" \
      | awk -F'[][]' '/NEEDED.*libPoco.*\.so/ { print $2 }' \
      | sort -u
  )

  if [[ "${bundled}" -eq 0 ]]; then
    echo "OpenHD has no Poco runtime libraries to bundle." >&2
    exit 1
  fi
}

append_staged_elf_runtime_dependencies() {
  local package_root="$1"
  local dependency_array_name="$2"
  local staged_file=""

  while IFS= read -r -d '' staged_file; do
    if ! readelf -d "${staged_file}" 2>/dev/null | grep -q "NEEDED"; then
      continue
    fi
    echo "Validating staged ELF runtime: ${staged_file#${package_root}}"
    if readelf -d "${staged_file}" 2>/dev/null \
      | grep -q "NEEDED.*libPoco"; then
      verify_packaged_poco_abi "${staged_file}"
    fi
    append_elf_runtime_dependencies "${staged_file}" \
      "${dependency_array_name}"
  done < <(find "${package_root}" -type f -print0)
}

deduplicate_dependencies() {
  local dependency_array_name="$1"
  local -n dependency_array="${dependency_array_name}"
  local unique=()
  local dependency=""
  declare -A seen=()
  for dependency in "${dependency_array[@]}"; do
    [[ -n "${dependency}" ]] || continue
    if [[ -z "${seen[${dependency}]:-}" ]]; then
      unique+=("${dependency}")
      seen["${dependency}"]=1
    fi
  done
  dependency_array=("${unique[@]}")
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
    if [[ "${package_name}" == "openhd" ]]; then
      echo "Conflicts: openhd-${package_arch}"
      echo "Replaces: openhd-${package_arch}"
    fi
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

  # Bullseye-era dpkg cannot read control.tar.zst. Force XZ so the same
  # package remains installable on all supported OpenHD base images.
  dpkg-deb -Zxz --build "${PKGDIR}" \
    "${package_name}_${VERSION}_${package_arch}.deb"
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
  # Hosted runners are normally fresh, but local/reusable runners must never
  # leak files from a package built for another architecture.
  rm -rf "${PKGDIR}"
  mkdir -p \
    "${PKGDIR}usr/local/bin" \
    "${PKGDIR}tmp" \
    "${PKGDIR}settings" \
    "${PKGDIR}etc/systemd/system"

  install -m 0755 scripts/openhd-fleet-setup "${PKGDIR}usr/local/bin/openhd-fleet-setup"
  install -m 0644 systemd/openhd-fleet-setup.service "${PKGDIR}etc/systemd/system/openhd-fleet-setup.service"

  if [[ "${PACKAGE_ARCH}" != "x86_64" ]]; then
    echo "Non-x86 architecture detected"
    if [[ "${PACKAGE_ARCH}" == "armhf" && "${OS}" == "raspbian" ]]; then
      echo "Using Raspberry Pi-specific systemd service for armhf Bullseye"
      cp systemd/openhd_rpi.service "${PKGDIR}etc/systemd/system/openhd.service"
    elif [[ "${PACKAGE_ARCH}" == "armhf" &&
            "${CUSTOM}" != "standard" ]]; then
      cp systemd/openhd-x20.service "${PKGDIR}etc/systemd/system/openhd.service"
    else
      cp systemd/openhd.service "${PKGDIR}etc/systemd/system/openhd.service"
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
  multiarch="${OPENHD_CROSS_TRIPLET:-$(dpkg-architecture -qDEB_HOST_MULTIARCH)}"
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

  if [[ -n "${OPENHD_SYSROOT:-}" && -n "${OPENHD_CROSS_TRIPLET:-}" ]]; then
    local cross_cc="${OPENHD_CROSS_TRIPLET}-gcc-10"
    command -v "${cross_cc}" >/dev/null
    (
      cd "${gst_perf_src}"
      ./autogen.sh
      PKG_CONFIG_SYSROOT_DIR="${OPENHD_SYSROOT}" \
      PKG_CONFIG_LIBDIR="${OPENHD_SYSROOT}/usr/lib/${OPENHD_CROSS_TRIPLET}/pkgconfig:${OPENHD_SYSROOT}/usr/lib/pkgconfig:${OPENHD_SYSROOT}/usr/share/pkgconfig" \
      CC="${cross_cc} --sysroot=${OPENHD_SYSROOT}" \
        ./configure \
          --host="${OPENHD_CROSS_TRIPLET}" \
          --with-sysroot="${OPENHD_SYSROOT}" \
          --prefix=/usr \
          --libdir="/usr/lib/${multiarch}"
      make -j"${make_jobs}"
      make install DESTDIR="${gst_perf_prefix}"
    )
  else
    (
    cd "${gst_perf_src}"
    ./autogen.sh
    ./configure --prefix=/usr --libdir="/usr/lib/${multiarch}"
    make -j"${make_jobs}"
    make install DESTDIR="${gst_perf_prefix}"
    )
  fi

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
  local package_name=""
  local debian_arch=""
  local packages=(python3 openssl wireguard-tools gstreamer1.0-libav gstreamer1.0-plugins-ugly)

  package_name="$(resolve_package_name "${PACKAGE_ARCH}" "${CUSTOM}")"
  debian_arch="$(normalize_debian_arch "${PACKAGE_ARCH}")"

  if [[ "${PACKAGE_ARCH}" == "armhf" && "${CUSTOM}" != "standard" ]]; then
    packages+=(
      iw i2c-tools libv4l-dev libusb-1.0-0 libpcap-dev
      libnl-3-dev libnl-genl-3-dev libsdl2-2.0-0 libsodium-dev
      gstreamer1.0-plugins-{base,good,bad} gstreamer1.0-tools
    )
  elif [[ "${PACKAGE_ARCH}" == "armhf" ||
          "${PACKAGE_ARCH}" == "arm64" ]]; then
      packages+=(
        iw nmap aircrack-ng i2c-tools libv4l-dev libusb-1.0-0
        libpcap-dev libnl-3-dev libnl-genl-3-dev
        libsdl2-2.0-0 libsodium-dev gstreamer1.0-plugins-{base,good,bad,ugly}
        gstreamer1.0-{tools,alsa,pulseaudio}
      )
  elif [[ "${PACKAGE_ARCH}" == "x86_64" ||
          "${PACKAGE_ARCH}" == "amd64" ]]; then
    packages+=(
      dkms qopenhd git iw nmap aircrack-ng i2c-tools libv4l-dev
      libusb-1.0-0 libpcap-dev libnl-3-dev libnl-genl-3-dev libsdl2-2.0-0
      libsodium-dev gstreamer1.0-plugins-{base,good,bad,ugly}
      gstreamer1.0-{tools,alsa,pulseaudio}
    )
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

  # Avoid uploading a stale package with the former generic package name.
  rm -f ./openhd*.deb
  local build_dir="/out/openhd-build"
  local build_tmp="/out/openhd-build-tmp"
  local install_build_dir="${build_dir}"
  rm -rf "${build_dir}" "${build_tmp}"
  mkdir -p "${build_dir}" "${build_tmp}"
  export TMPDIR="${build_tmp}"
  # Camera backends are GStreamer plugins supplied by the image. Keeping the
  # core free of a direct libcamera link makes one binary usable on every board
  # of the same architecture (for example Pi 5, Rockchip and Allwinner arm64).
  local enable_libcamera="OFF"
  echo "OpenHD libcamera support: ${enable_libcamera}"

  if [[ -n "${OPENHD_PREBUILT_BINARY:-}" ]]; then
    test -f "${OPENHD_PREBUILT_BINARY}"
    cp "${OPENHD_PREBUILT_BINARY}" "${build_dir}/openhd"
    echo "Using prebuilt ${PACKAGE_ARCH} OpenHD binary: ${OPENHD_PREBUILT_BINARY}"
    if [[ -n "${OPENHD_PREBUILT_BUILD_DIR:-}" ]]; then
      test -f "${OPENHD_PREBUILT_BUILD_DIR}/cmake_install.cmake"
      install_build_dir="${OPENHD_PREBUILT_BUILD_DIR}"
    fi
  else
    local poco_dir=""
    poco_dir="$(resolve_system_poco_dir)"
    echo "Using distro Poco package configuration: ${poco_dir}"

    # Ensure Kconfig files are generated
    make config

    cmake -S OpenHD/ -B "${build_dir}" \
      -DPoco_DIR="${poco_dir}" \
      -DENABLE_LIBCAMERA="${enable_libcamera}" \
      -DARTOSYN_SDK_ROOT="${ARTOSYN_SDK_ROOT}" \
      -DARTOSYN_SDK_LIB="${ARTOSYN_SDK_LIB}" \
      -DARTOSYN_SDK_DAEMON="${ARTOSYN_SDK_DAEMON:-}" \
      -DARTOSYN_SDK_TUNTAP="${ARTOSYN_SDK_TUNTAP:-}"
    cmake --build "${build_dir}" --parallel "$(nproc)"
  fi

  mkdir -p "${PKGDIR}usr/local/bin/"
  cp "${build_dir}/openhd" "${PKGDIR}usr/local/bin/"
  DESTDIR="${PKGDIR}" cmake --install "${install_build_dir}" \
    --prefix /usr/local --component Nexmon
  if [[ -f "${PKGDIR}usr/local/lib/openhd/nexmon/manifest.json" ]]; then
    packages+=(network-manager systemd iw iproute2 kmod coreutils)
  fi
  bundle_poco_runtime "${PKGDIR}usr/local/bin/openhd"
  verify_packaged_poco_abi "${PKGDIR}usr/local/bin/openhd"
  build_and_stage_gst_perf

  if [[ "${artosyn_enabled}" -eq 1 ]]; then
    mkdir -p "${PKGDIR}usr/local/libexec"
    cp systemd/openhd-artosyn-start \
      "${PKGDIR}usr/local/libexec/openhd-artosyn-start"
    chmod 0755 "${PKGDIR}usr/local/libexec/openhd-artosyn-start"
    cp systemd/openhd-artosyn.service \
      "${PKGDIR}etc/systemd/system/openhd-artosyn.service"

    local daemon_candidates=(
      "${ARTOSYN_SDK_ROOT}/install/arm64/bin/l4_daemon"
      "${ARTOSYN_SDK_ROOT}/install/armhf/bin/l4_daemon"
      "${ARTOSYN_SDK_ROOT}/build/arm64/daemon/l4_daemon"
      "${ARTOSYN_SDK_ROOT}/build/armhf/daemon/l4_daemon"
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
        \( -iname "l4_daemon" -o -iname "artosyn_daemon" -o -iname "ar8030_daemon" -o -iname "artlinkd" -o -iname "bbd" -o -iname "bb_daemon" -o -iname "daemon" \) \
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
      "${ARTOSYN_SDK_ROOT}/install/arm64/bin/l4_tuntap"
      "${ARTOSYN_SDK_ROOT}/install/armhf/bin/l4_tuntap"
      "${ARTOSYN_SDK_ROOT}/build/arm64/app/tuntap/l4_tuntap"
      "${ARTOSYN_SDK_ROOT}/build/armhf/app/tuntap/l4_tuntap"
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
      tuntap_src="$(find "${ARTOSYN_SDK_ROOT}" -type f \
        \( -name "l4_tuntap" -o -name "tuntap_bb" \) | head -n 1 || true)"
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

  if [[ -z "${OPENHD_PREBUILT_BINARY:-}" ]]; then
    append_staged_elf_runtime_dependencies "${PKGDIR}" packages
  else
    echo "Cross package uses the reviewed architecture dependency baseline."
  fi
  deduplicate_dependencies packages
  if [[ "${enable_libcamera}" == "OFF" ]]; then
    local dependency=""
    for dependency in "${packages[@]}"; do
      if [[ "${dependency}" == libcamera* ||
            "${dependency}" == "gstreamer1.0-libcamera" ]]; then
        echo "Non-Raspberry-Pi package unexpectedly depends on ${dependency}." >&2
        exit 1
      fi
    done
  fi
  echo "Final package dependencies: $(join_by ', ' "${packages[@]}")"

  if command -v fpm >/dev/null 2>&1; then
    # Build the package using fpm
    require_staged_gst_perf
    local package_relationships=()
    if [[ "${package_name}" == "openhd" ]]; then
      package_relationships+=(
        --conflicts "openhd-${debian_arch}"
        --replaces "openhd-${debian_arch}"
      )
    fi
    fpm -a "${debian_arch}" -s dir -t deb -n "${package_name}" -v "${VERSION}" -C "${PKGDIR}" \
      -p "${package_name}_${VERSION}_${debian_arch}.deb" \
      --deb-compression xz \
      --after-install after-install.sh \
      --before-install before-install.sh \
      "${package_relationships[@]}" \
      -d "$(IFS=','; echo "${packages[*]}")"
  else
    echo "fpm not available; building package with dpkg-deb."
    require_staged_gst_perf
    build_deb_package "${package_name}" "${debian_arch}" "${packages[@]}"
  fi

  cp *.deb /out/
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  create_package_directory
  build_package
fi
