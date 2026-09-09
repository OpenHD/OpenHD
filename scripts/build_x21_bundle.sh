#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sdk_dir="$(realpath "${1:?Usage: build_x21_bundle.sh <sdk-dir> <sysutils-source> <output-dir>}")"
sysutils_source="$(realpath "${2:?Usage: build_x21_bundle.sh <sdk-dir> <sysutils-source> <output-dir>}")"
output_dir="$(realpath -m "${3:?Usage: build_x21_bundle.sh <sdk-dir> <sysutils-source> <output-dir>}")"

test -x "${sdk_dir}/relocate-sdk.sh"
test -f "${sdk_dir}/environment-setup"
test -f "${sdk_dir}/share/buildroot/toolchainfile.cmake"
test -f "${sysutils_source}/CMakeLists.txt"

"${sdk_dir}/relocate-sdk.sh"
# shellcheck disable=SC1091
source "${sdk_dir}/environment-setup"

test "${ARCH}" = "arm64"
test "${CROSS_COMPILE}" = "aarch64-buildroot-linux-gnu-"
command -v "${CC}" >/dev/null
command -v "${CXX}" >/dev/null

work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT
openhd_build="${work_dir}/openhd"
sysutils_build="${work_dir}/sysutils"
stage_dir="${work_dir}/bundle"
toolchain_file="${sdk_dir}/share/buildroot/toolchainfile.cmake"

# SysUtils invokes the host compiler as a CMake command. Buildroot's
# CXX_FOR_BUILD contains a command plus arguments (for example, ccache g++),
# which cannot be used as a single executable path there.
export HOST_CXX="${HOST_CXX:-/usr/bin/g++}"
export OPENHD_SYSROOT="${STAGING_DIR}"
export OPENHD_CROSS_TRIPLET="aarch64-buildroot-linux-gnu"
export OPENHD_CMAKE_TOOLCHAIN_FILE="${toolchain_file}"

cmake -S "${repo_root}/OpenHD" -B "${openhd_build}" \
  -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_USB_CAMERAS=OFF \
  -DBUILD_SHARED_LIBS=OFF
cmake --build "${openhd_build}" --parallel "$(nproc)" --target openhd

cmake -S "${sysutils_source}" -B "${sysutils_build}" \
  -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "${sysutils_build}" --parallel "$(nproc)" --target openhd_sys_utils

mkdir -p "${stage_dir}/usr/bin" "${stage_dir}/usr/lib"
install -m 0755 "${openhd_build}/openhd" "${stage_dir}/usr/bin/openhd"
install -m 0755 "${sysutils_build}/openhd_sys_utils" \
  "${stage_dir}/usr/bin/openhd_sys_utils"
"${STRIP}" "${stage_dir}/usr/bin/openhd" "${stage_dir}/usr/bin/openhd_sys_utils"

copy_library_family() {
  local library_glob="$1"
  local matches=()
  mapfile -t matches < <(compgen -G "${STAGING_DIR}/usr/lib/${library_glob}" || true)
  if ((${#matches[@]} == 0)); then
    echo "Missing X21 runtime library family: ${library_glob}" >&2
    return 1
  fi
  cp -a "${matches[@]}" "${stage_dir}/usr/lib/"
}

copy_library_family 'libPocoFoundation.so*'
copy_library_family 'libPocoNet.so*'
copy_library_family 'libPocoEncodings.so*'
copy_library_family 'libpcap.so*'
copy_library_family 'libsodium.so*'
copy_library_family 'libusb-1.0.so*'

openhd_commit="$(git -C "${repo_root}" rev-parse HEAD)"
sysutils_commit="$(git -C "${sysutils_source}" rev-parse HEAD)"
bundle_version="${openhd_commit:0:12}-${sysutils_commit:0:12}"
bundle_name="openhd-x21b-bundle-${bundle_version}.tar.zst"

cat >"${stage_dir}/manifest.json" <<EOF
{
  "schema": 1,
  "platform": "x21b",
  "architecture": "aarch64",
  "openhd_commit": "${openhd_commit}",
  "sysutils_commit": "${sysutils_commit}",
  "sdk_sha256": "${X21_SDK_SHA256:-unknown}",
  "sdk_buildroot_commit": "${X21_SDK_BUILDROOT_COMMIT:-unknown}",
  "generated_at": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF

(
  cd "${stage_dir}"
  checksum_file="${work_dir}/bundle-sha256sums"
  find . -type f ! -name sha256sums -print0 | sort -z | xargs -0 sha256sum \
    >"${checksum_file}"
  mv "${checksum_file}" sha256sums
)

mkdir -p "${output_dir}"
tar --numeric-owner --owner=0 --group=0 --zstd \
  -C "${stage_dir}" -cf "${output_dir}/${bundle_name}" .
(
  cd "${output_dir}"
  sha256sum "${bundle_name}" >"${bundle_name}.sha256"
)

"${READELF}" -h "${stage_dir}/usr/bin/openhd" | grep -q 'Machine:.*AArch64'
"${READELF}" -h "${stage_dir}/usr/bin/openhd_sys_utils" | grep -q 'Machine:.*AArch64'
"${READELF}" -d "${stage_dir}/usr/bin/openhd" | grep NEEDED
"${READELF}" -d "${stage_dir}/usr/bin/openhd_sys_utils" | grep NEEDED

cp "${stage_dir}/manifest.json" "${output_dir}/${bundle_name}.manifest.json"
echo "Created ${output_dir}/${bundle_name}"
