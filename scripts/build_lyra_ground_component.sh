#!/usr/bin/env bash
set -euo pipefail

usage="Usage: build_lyra_ground_component.sh <sdk-or-sysroot-dir> <output-dir> [toolchain-file]"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sdk_dir="$(realpath "${1:?${usage}}")"
output_dir="$(realpath -m "${2:?${usage}}")"
custom_toolchain="${3:-}"

# 1. Environment and toolchain setup
if [[ -x "${sdk_dir}/relocate-sdk.sh" ]]; then
  "${sdk_dir}/relocate-sdk.sh"
fi

if [[ -f "${sdk_dir}/environment-setup" ]]; then
  # shellcheck disable=SC1091
  source "${sdk_dir}/environment-setup"
fi

if [[ -z "${CROSS_COMPILE:-}" ]]; then
  if command -v arm-rockchip830-linux-gnueabihf-gcc >/dev/null 2>&1; then
    CROSS_COMPILE="arm-rockchip830-linux-gnueabihf-"
  elif command -v arm-linux-gnueabihf-gcc >/dev/null 2>&1; then
    CROSS_COMPILE="arm-linux-gnueabihf-"
  elif command -v arm-linux-gnueabihf-gcc-10 >/dev/null 2>&1; then
    CROSS_COMPILE="arm-linux-gnueabihf-"
  else
    echo "No 32-bit ARM cross compiler found. Please install gcc-arm-linux-gnueabihf." >&2
    exit 1
  fi
fi

CC="${CC:-${CROSS_COMPILE}gcc}"
CXX="${CXX:-${CROSS_COMPILE}g++}"
STRIP="${STRIP:-${CROSS_COMPILE}strip}"
READELF="${READELF:-${CROSS_COMPILE}readelf}"

command -v "${CC}" >/dev/null || { echo "CC (${CC}) not found" >&2; exit 1; }
command -v "${CXX}" >/dev/null || { echo "CXX (${CXX}) not found" >&2; exit 1; }

work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT
build_dir="${work_dir}/build"
stage_dir="${work_dir}/component"

# Determine toolchain file or cmake sysroot arguments
cmake_flags=()
if [[ -n "${custom_toolchain}" && -f "${custom_toolchain}" ]]; then
  cmake_flags+=("-DCMAKE_TOOLCHAIN_FILE=${custom_toolchain}")
elif [[ -f "${sdk_dir}/share/buildroot/toolchainfile.cmake" ]]; then
  cmake_flags+=("-DCMAKE_TOOLCHAIN_FILE=${sdk_dir}/share/buildroot/toolchainfile.cmake")
elif [[ -f "${repo_root}/OpenHD/cmake/portable-linux-toolchain.cmake" ]]; then
  export OPENHD_SYSROOT="${sdk_dir}"
  export OPENHD_CROSS_TRIPLET="arm-linux-gnueabihf"
  cmake_flags+=("-DCMAKE_TOOLCHAIN_FILE=${repo_root}/OpenHD/cmake/portable-linux-toolchain.cmake")
else
  cmake_flags+=(
    "-DCMAKE_SYSTEM_NAME=Linux"
    "-DCMAKE_SYSTEM_PROCESSOR=arm"
    "-DCMAKE_C_COMPILER=${CC}"
    "-DCMAKE_CXX_COMPILER=${CXX}"
    "-DCMAKE_SYSROOT=${sdk_dir}"
    "-DCMAKE_FIND_ROOT_PATH=${sdk_dir}"
    "-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER"
    "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY"
    "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY"
  )
fi

# 2. Build OpenHD with Ground-only configuration
# Lyra (RK3506) is a ground module: ENABLE_AIR=OFF, ENABLE_USB_CAMERAS=OFF
cmake -S "${repo_root}/OpenHD" -B "${build_dir}" \
  "${cmake_flags[@]}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_AIR=OFF \
  -DENABLE_USB_CAMERAS=OFF \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_CXX_FLAGS="-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard"

cmake --build "${build_dir}" --parallel "$(nproc)" --target openhd

mkdir -p "${stage_dir}/usr/bin" "${stage_dir}/usr/lib"
install -m 0755 "${build_dir}/openhd" "${stage_dir}/usr/bin/openhd"
"${STRIP}" "${stage_dir}/usr/bin/openhd"

# Copy runtime libraries from sysroot/staging
staging_root="${STAGING_DIR:-${sdk_dir}}"
copy_library_family() {
  local library_glob="$1"
  local optional="${2:-0}"
  local matches=()
  mapfile -t matches < <(compgen -G "${staging_root}/usr/lib/${library_glob}" || compgen -G "${staging_root}/lib/${library_glob}" || compgen -G "${staging_root}/usr/lib/arm-linux-gnueabihf/${library_glob}" || true)
  if ((${#matches[@]} == 0)); then
    if ((optional)); then
      return 0
    fi
    echo "Missing Lyra ground runtime library family: ${library_glob}" >&2
    return 1
  fi
  cp -a "${matches[@]}" "${stage_dir}/usr/lib/"
}

copy_library_family 'libPocoFoundation.so*' 1
copy_library_family 'libPocoNet.so*' 1
copy_library_family 'libPocoEncodings.so*' 1
copy_library_family 'libpcap.so*' 1
copy_library_family 'libsodium.so*' 1
copy_library_family 'libusb-1.0.so*' 1
copy_library_family 'libatomic.so*' 1

# Provide ground module systemd service and installer
mkdir -p "${stage_dir}/etc/systemd/system"
cat >"${stage_dir}/etc/systemd/system/openhd-ground.service" <<'EOF'
[Unit]
Description=OpenHD Ground Station Module (Luckfox Lyra RK3506)
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/openhd -g
Restart=always
RestartSec=3
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

cat >"${stage_dir}/install-lyra-ground.sh" <<'INSTALL_EOF'
#!/bin/sh
set -eu
DEST_BIN="${DEST_BIN:-/usr/bin}"
DEST_LIB="${DEST_LIB:-/usr/lib}"
DIR="$(cd "$(dirname "$0")" && pwd)"

mkdir -p "${DEST_BIN}" "${DEST_LIB}"
install -m 0755 "${DIR}/usr/bin/openhd" "${DEST_BIN}/openhd"

if [ -d "${DIR}/usr/lib" ] && [ "$(ls -A "${DIR}/usr/lib" 2>/dev/null)" ]; then
  cp -af "${DIR}/usr/lib/"* "${DEST_LIB}/"
  ldconfig 2>/dev/null || true
fi

if [ -f "${DIR}/etc/systemd/system/openhd-ground.service" ] && [ -d /etc/systemd/system ]; then
  cp -f "${DIR}/etc/systemd/system/openhd-ground.service" /etc/systemd/system/
  systemctl daemon-reload 2>/dev/null || true
  systemctl enable openhd-ground.service 2>/dev/null || true
fi

echo "OpenHD Ground Module installed for Luckfox Lyra (RK3506)."
INSTALL_EOF
chmod 0755 "${stage_dir}/install-lyra-ground.sh"

version_header="${repo_root}/OpenHD/ohd_common/inc/openhd_global_constants.hpp"
major="$(awk '/MAJOR_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
minor="$(awk '/MINOR_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
patch="$(awk '/PATCH_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
openhd_version="${major}.${minor}.${patch}-evo"
openhd_commit="$(git -C "${repo_root}" rev-parse HEAD)"
package_version="${openhd_version}-${openhd_commit:0:12}"
package_name="openhd-lyra-ground-${package_version}.tar.gz"

cat >"${stage_dir}/component-manifest.json" <<EOF
{
  "schema": 1,
  "component": "openhd",
  "role": "ground",
  "air_enabled": false,
  "component_version": "${openhd_version}",
  "package_version": "${package_version}",
  "platform": "luckfox-lyra",
  "target": "rk3506",
  "architecture": "armhf",
  "source_commit": "${openhd_commit}",
  "sdk_sha256": "${LYRA_SDK_SHA256:-unknown}",
  "generated_at": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF

(
  cd "${stage_dir}"
  find . -type f ! -name sha256sums -print0 | sort -z | xargs -0 sha256sum >sha256sums
)

mkdir -p "${output_dir}"
tar --numeric-owner --owner=0 --group=0 -C "${stage_dir}" \
  -czf "${output_dir}/${package_name}" .
cp "${stage_dir}/component-manifest.json" \
  "${output_dir}/${package_name}.manifest.json"
(
  cd "${output_dir}"
  sha256sum "${package_name}" >"${package_name}.sha256"
  for alias in openhd-lyra-ground-latest.tar.gz openhd-rk3506-ground-latest.tar.gz; do
    cp "${package_name}" "${alias}"
    sha256sum "${alias}" >"${alias}.sha256"
    cp "${package_name}.manifest.json" "${alias}.manifest.json"
  done
)

"${READELF}" -h "${stage_dir}/usr/bin/openhd" | grep -q 'Machine:.*ARM'
"${READELF}" -d "${stage_dir}/usr/bin/openhd" | grep NEEDED || true
echo "Created ${output_dir}/${package_name}"

