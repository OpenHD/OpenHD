#!/usr/bin/env bash
set -euo pipefail

usage="Usage: build_x21_component.sh <sdk-dir> <output-dir>"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sdk_dir="$(realpath "${1:?${usage}}")"
output_dir="$(realpath -m "${2:?${usage}}")"

test -x "${sdk_dir}/relocate-sdk.sh"
test -f "${sdk_dir}/environment-setup"
test -f "${sdk_dir}/share/buildroot/toolchainfile.cmake"

"${sdk_dir}/relocate-sdk.sh"
# shellcheck disable=SC1091
source "${sdk_dir}/environment-setup"

test "${ARCH}" = "arm64"
test "${CROSS_COMPILE}" = "aarch64-buildroot-linux-gnu-"
command -v "${CC}" >/dev/null
command -v "${CXX}" >/dev/null

work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT
build_dir="${work_dir}/build"
stage_dir="${work_dir}/component"
toolchain_file="${sdk_dir}/share/buildroot/toolchainfile.cmake"

# Configurable options
ENABLE_USB_CAMERAS="${ENABLE_USB_CAMERAS:-OFF}"

cmake -S "${repo_root}/OpenHD" -B "${build_dir}" \
  -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_USB_CAMERAS="${ENABLE_USB_CAMERAS}" \
  -DBUILD_SHARED_LIBS=OFF
cmake --build "${build_dir}" --parallel "$(nproc)" --target openhd

mkdir -p "${stage_dir}/usr/bin" "${stage_dir}/usr/lib"
install -m 0755 "${build_dir}/openhd" "${stage_dir}/usr/bin/openhd"
"${STRIP}" "${stage_dir}/usr/bin/openhd"

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

version_header="${repo_root}/OpenHD/ohd_common/inc/openhd_global_constants.hpp"
major="$(awk '/MAJOR_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
minor="$(awk '/MINOR_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
patch="$(awk '/PATCH_VERSION =/{gsub(/;/, "", $NF); print $NF}' "${version_header}")"
openhd_version="${major}.${minor}.${patch}-evo"
openhd_commit="$(git -C "${repo_root}" rev-parse HEAD)"
package_version="${openhd_version}-${openhd_commit:0:12}"
package_name="openhd-x21b-${package_version}.tar.gz"

# Add convenience runtime installer script into the package
cat >"${stage_dir}/install-openhd.sh" <<'INSTALL_EOF'
#!/bin/sh
set -eu
DEST_BIN="${DEST_BIN:-/usr/bin}"
DEST_LIB="${DEST_LIB:-/usr/lib}"
DIR="$(cd "$(dirname "$0")" && pwd)"

mkdir -p "${DEST_BIN}" "${DEST_LIB}"
install -m 0755 "${DIR}/usr/bin/openhd" "${DEST_BIN}/openhd"

if [ -d "${DIR}/usr/lib" ]; then
  cp -af "${DIR}/usr/lib/"* "${DEST_LIB}/"
  ldconfig 2>/dev/null || true
fi

echo "OpenHD installed to ${DEST_BIN}/openhd"
INSTALL_EOF
chmod 0755 "${stage_dir}/install-openhd.sh"

cat >"${stage_dir}/component-manifest.json" <<EOF
{
  "schema": 1,
  "component": "openhd",
  "component_version": "${openhd_version}",
  "package_version": "${package_version}",
  "platform": "x21b",
  "targets": ["x21b", "luckfox-aura", "rv1126b"],
  "architecture": "aarch64",
  "source_commit": "${openhd_commit}",
  "sdk_sha256": "${X21_SDK_SHA256:-unknown}",
  "sdk_buildroot_commit": "${X21_SDK_BUILDROOT_COMMIT:-unknown}",
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
  cp "${package_name}" openhd-x21b-latest.tar.gz
  sha256sum openhd-x21b-latest.tar.gz >openhd-x21b-latest.tar.gz.sha256
  cp "${package_name}.manifest.json" openhd-x21b-latest.tar.gz.manifest.json

  # Provide Luckfox Aura (RV1126B) packages and manifests
  aura_package_name="openhd-aura-${package_version}.tar.gz"
  cp "${package_name}" "${aura_package_name}"
  sha256sum "${aura_package_name}" >"${aura_package_name}.sha256"
  sed 's/"platform": "x21b"/"platform": "luckfox-aura"/' "${package_name}.manifest.json" >"${aura_package_name}.manifest.json"

  for alias in openhd-aura-latest.tar.gz openhd-luckfox-aura-latest.tar.gz openhd-rv1126-latest.tar.gz; do
    cp "${aura_package_name}" "${alias}"
    sha256sum "${alias}" >"${alias}.sha256"
    cp "${aura_package_name}.manifest.json" "${alias}.manifest.json"
  done
)

"${READELF}" -h "${stage_dir}/usr/bin/openhd" | grep -q 'Machine:.*AArch64'
"${READELF}" -d "${stage_dir}/usr/bin/openhd" | grep NEEDED
echo "Created ${output_dir}/${package_name}"
