#!/usr/bin/env bash
set -euo pipefail

usage="Usage: build_x21_bundle.sh <sdk-dir> <sysutils-source> <ohd-root-seed> <output-dir>"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sdk_dir="$(realpath "${1:?${usage}}")"
sysutils_source="$(realpath "${2:?${usage}}")"
ohd_root_seed="$(realpath "${3:?${usage}}")"
output_dir="$(realpath -m "${4:?${usage}}")"

test -x "${sdk_dir}/relocate-sdk.sh"
test -f "${sdk_dir}/environment-setup"
test -f "${sdk_dir}/share/buildroot/toolchainfile.cmake"
test -f "${sysutils_source}/CMakeLists.txt"
test -x "${ohd_root_seed}/start-ohd.sh"
test -f "${ohd_root_seed}/drivers/88x2eu_ohd.ko"

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

mkdir -p "${stage_dir}"
cp -a "${ohd_root_seed}/." "${stage_dir}/"
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
  "ohd_seed_sha256": "${X21_OHD_SEED_SHA256:-unknown}",
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

update_name="OpenHD-X21B-latest.ohd"
update_work="${work_dir}/swupdate"
mkdir -p "${update_work}"

# Match the X21B factory image geometry: 2 KiB NAND pages, 128 KiB erase
# blocks and the fixed 100 MiB OHD partition from openhd_x21b_defconfig.
mkfs.ubifs -x lzo -e 126976 -m 2048 -c 825 \
  -d "${stage_dir}" -F -o "${update_work}/ohd.img.ubifs"
cat >"${update_work}/ubinize.cfg" <<EOF
[ubi]
mode=ubi
vol_id=0
vol_type=dynamic
vol_name=ohd
vol_alignment=1
vol_flags=autoresize
image=${update_work}/ohd.img.ubifs
EOF
ubinize -o "${update_work}/ohd.img" -m 2048 -p 0x20000 \
  "${update_work}/ubinize.cfg"

cat >"${update_work}/sw-description" <<EOF
software =
{
    version = "${bundle_version}";
    description = "OpenHD X21B OHD partition update";

    scripts: (
        {
            filename = "prepare-ohd.sh";
            type = "shellscript";
        }
    );

    images: (
        {
            filename = "ohd.img";
            device = "mtd9";
            type = "flash";
        }
    );
}
EOF
cat >"${update_work}/prepare-ohd.sh" <<'EOF'
#!/bin/sh
set -eu

# The SWUpdate flash handler erases only enough PEBs for a compact image.
# Erase the entire shared OHD MTD first so stale UBI image-sequence headers
# cannot remain beyond the end of the new image and make ubiattach reject it.
if [ "${1:-}" = "preinst" ]; then
    umount /ohd 2>/dev/null || true
    ubidetach /dev/ubi_ctrl -m 9 2>/dev/null || true
    flash_erase /dev/mtd9 0 0
fi
EOF
chmod 0755 "${update_work}/prepare-ohd.sh"
(
  cd "${update_work}"
  printf '%s\n' sw-description prepare-ohd.sh ohd.img | cpio -ov -H crc -L \
    >"${output_dir}/${update_name}"
)

update_sha256="$(sha256sum "${output_dir}/${update_name}" | awk '{print $1}')"
update_size="$(stat -c '%s' "${output_dir}/${update_name}")"
printf '%s  %s\n' "${update_sha256}" "${update_name}" \
  >"${output_dir}/${update_name}.sha256"
cp "${stage_dir}/manifest.json" "${output_dir}/${update_name}.manifest.json"
cat >"${output_dir}/openhd-x21b-updates.json" <<EOF
{
  "os_list": [
    {
      "name": "OpenHD X21B",
      "description": "SWUpdate package for an existing X21B installation",
      "icon": "https://fra1.digitaloceanspaces.com/openhd-images/Downloader/OpenHD-advanced.png",
      "subitems": [
        {
          "name": "OpenHD X21B latest",
          "description": "Replace the read-only OHD NAND partition using SWUpdate",
          "icon": "https://fra1.digitaloceanspaces.com/openhd-images/Downloader/OpenHD-advanced.png",
          "url": "https://dl.cloudsmith.io/public/openhd/dev-release/raw/files/${update_name}",
          "image_download_size": ${update_size},
          "extract_size": ${update_size},
          "update_sha256": "${update_sha256}",
          "update_destination": "root",
          "update_filename": "${update_name}",
          "release_date": "$(date -u +%Y-%m-%d)"
        }
      ]
    }
  ]
}
EOF
echo "Created ${output_dir}/${bundle_name}"
echo "Created ${output_dir}/${update_name}"
