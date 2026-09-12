#!/usr/bin/env bash
set -euo pipefail

usage="Usage: build_luckfox_component.sh <sdk-dir> <output-dir>"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sdk_dir="$(realpath "${1:?${usage}}")"
output_dir="$(realpath -m "${2:?${usage}}")"

if [[ -x "${sdk_dir}/relocate-sdk.sh" ]]; then
  "${sdk_dir}/relocate-sdk.sh"
fi

test -f "${sdk_dir}/environment-setup"
test -f "${sdk_dir}/share/buildroot/toolchainfile.cmake"

# shellcheck disable=SC1091
source "${sdk_dir}/environment-setup"

test "${ARCH}" = "arm"
test "${CROSS_COMPILE}" = "arm-rockchip830-linux-uclibcgnueabihf-"
command -v "${CC}" >/dev/null
command -v "${CXX}" >/dev/null

work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT
build_dir="${work_dir}/build"
stage_dir="${work_dir}/component"
toolchain_file="${sdk_dir}/share/buildroot/toolchainfile.cmake"

cmake -S "${repo_root}/OpenHD" -B "${build_dir}" \
  -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_USB_CAMERAS=OFF \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_EXE_LINKER_FLAGS="-lstdc++fs"
cmake --build "${build_dir}" --parallel "$(nproc)" --target openhd

mkdir -p "${stage_dir}/usr/bin" "${stage_dir}/usr/lib"
install -m 0755 "${build_dir}/openhd" "${stage_dir}/usr/bin/openhd"
"${STRIP}" "${stage_dir}/usr/bin/openhd"

copy_library_family() {
  local library_glob="$1"
  local optional="${2:-0}"
  local matches=()
  mapfile -t matches < <(compgen -G "${STAGING_DIR}/usr/lib/${library_glob}" || compgen -G "${STAGING_DIR}/lib/${library_glob}" || true)
  if ((${#matches[@]} == 0)); then
    if ((optional)); then
      return 0
    fi
    echo "Missing Luckfox Pico runtime library family: ${library_glob}" >&2
    return 1
  fi
  cp -a "${matches[@]}" "${stage_dir}/usr/lib/"
}

copy_library_family 'libPocoFoundation.so*'
copy_library_family 'libPocoNet.so*'
copy_library_family 'libPocoEncodings.so*' 1
copy_library_family 'libpcap.so*'
copy_library_family 'libsodium.so*'
copy_library_family 'libusb-1.0.so*' 1
copy_library_family 'librockchip_mpp.so*' 1
copy_library_family 'librga.so*' 1
copy_library_family 'libatomic.so*' 1

version_header="${repo_root}/OpenHD/ohd_common/inc/openhd_global_constants.hpp"
major="$(awk '/MAJOR_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
minor="$(awk '/MINOR_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
patch="$(awk '/PATCH_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
openhd_version="${major}.${minor}.${patch}-evo"
openhd_commit="$(git -C "${repo_root}" rev-parse HEAD)"
package_version="${openhd_version}-${openhd_commit:0:12}"
package_name="openhd-luckfox-pico-${package_version}.tar.gz"

cat >"${stage_dir}/component-manifest.json" <<EOF
{
  "schema": 1,
  "component": "openhd",
  "component_version": "${openhd_version}",
  "package_version": "${package_version}",
  "platform": "luckfox-pico",
  "targets": ["rv1103", "rv1106"],
  "architecture": "armhf",
  "source_commit": "${openhd_commit}",
  "sdk_sha256": "${LUCKFOX_SDK_SHA256:-unknown}",
  "sdk_buildroot_commit": "${LUCKFOX_SDK_BUILDROOT_COMMIT:-unknown}",
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

  # Provide canonical and per-target alias packages
  for alias in openhd-luckfox-pico-latest.tar.gz openhd-luckfox-rv1106-latest.tar.gz openhd-luckfox-rv1103-latest.tar.gz; do
    cp "${package_name}" "${alias}"
    sha256sum "${alias}" >"${alias}.sha256"
    cp "${package_name}.manifest.json" "${alias}.manifest.json"
  done
)

"${READELF}" -h "${stage_dir}/usr/bin/openhd" | grep -q 'Machine:.*ARM'
"${READELF}" -d "${stage_dir}/usr/bin/openhd" | grep NEEDED
echo "Created ${output_dir}/${package_name}"
