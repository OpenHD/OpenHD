#!/usr/bin/env bash
set -euo pipefail

usage="Usage: build_luckfox_sysroot.sh <luckfox-sdk-dir> <output-dir>"
sdk_source="$(realpath "${1:?${usage}}")"
output_dir="$(realpath -m "${2:?${usage}}")"

test -d "${sdk_source}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf" || {
  echo "Error: Toolchain directory missing in ${sdk_source}" >&2
  exit 1
}
test -d "${sdk_source}/sysdrv" || {
  echo "Error: sysdrv directory missing in ${sdk_source}" >&2
  exit 1
}
test -d "${sdk_source}/media" || {
  echo "Error: media directory missing in ${sdk_source}" >&2
  exit 1
}

echo "=== Configuring Luckfox Pico build ==="
cd "${sdk_source}"

# Select RV1106/RV1103 SD Card Buildroot configuration
# Both chips share Cortex-A7 architecture, uClibc toolchain, and luckfox_pico_defconfig
board_cfg="${sdk_source}/project/cfg/BoardConfig_IPC/BoardConfig-SD_CARD-Buildroot-RV1106_Luckfox_Pico_Pro_Max-IPC.mk"
test -f "${board_cfg}" || {
  echo "Error: Board configuration not found: ${board_cfg}" >&2
  exit 1
}
ln -sf "${board_cfg}" .BoardConfig.mk

# Disable in-tree OpenHD builds during sysroot creation; OpenHD is cross-compiled
# separately against the resulting SDK tarball.
buildroot_defconfig="${sdk_source}/sysdrv/tools/board/buildroot/luckfox_pico_defconfig"
if [[ -f "${buildroot_defconfig}" ]]; then
  sed -i -E 's/^BR2_PACKAGE_OPENHD=y/# BR2_PACKAGE_OPENHD is not set/' "${buildroot_defconfig}"
  sed -i -E 's/^BR2_PACKAGE_OPENHD_SYSUTILS=y/# BR2_PACKAGE_OPENHD_SYSUTILS is not set/' "${buildroot_defconfig}"
  for pkg in BR2_PACKAGE_POCO BR2_PACKAGE_POCO_NET BR2_PACKAGE_LIBPCAP BR2_PACKAGE_LIBSODIUM BR2_PACKAGE_LIBUSB; do
    if ! grep -q "^${pkg}=y" "${buildroot_defconfig}"; then
      echo "${pkg}=y" >> "${buildroot_defconfig}"
    fi
  done
fi

echo "=== Building Media Libraries (Rockchip MPP & RGA) ==="
./build.sh media

echo "=== Building Buildroot Base System & Sysroot ==="
export PATH="${sdk_source}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin:${PATH}"
make -C sysdrv buildroot

buildroot_dir="${sdk_source}/sysdrv/source/buildroot/buildroot-2023.02.6"
staging_dir="${buildroot_dir}/output/staging"
host_dir="${buildroot_dir}/output/host"

test -d "${staging_dir}" || {
  echo "Error: Buildroot staging directory not found: ${staging_dir}" >&2
  exit 1
}
test -d "${host_dir}" || {
  echo "Error: Buildroot host directory not found: ${host_dir}" >&2
  exit 1
}

echo "=== Installing Rockchip MPP and RGA into sysroot ==="
media_out="${sdk_source}/output/out/media_out"
mpp_release="${sdk_source}/media/mpp/release_mpp_rv1106_arm-rockchip830-linux-uclibcgnueabihf"
rga_release="${sdk_source}/media/rga/release_rga_rv1106_arm-rockchip830-linux-uclibcgnueabihf"

mkdir -p "${staging_dir}/usr/include" "${staging_dir}/usr/lib"

if [[ -d "${mpp_release}/include" ]]; then
  cp -a "${mpp_release}/include/." "${staging_dir}/usr/include/"
fi
if [[ -d "${mpp_release}/lib" ]]; then
  cp -a "${mpp_release}/lib/"*.so* "${staging_dir}/usr/lib/" 2>/dev/null || true
  cp -a "${mpp_release}/lib/"*.a "${staging_dir}/usr/lib/" 2>/dev/null || true
fi

if [[ -d "${rga_release}/include" ]]; then
  cp -a "${rga_release}/include/." "${staging_dir}/usr/include/"
fi
if [[ -d "${rga_release}/lib" ]]; then
  cp -a "${rga_release}/lib/"*.so* "${staging_dir}/usr/lib/" 2>/dev/null || true
  cp -a "${rga_release}/lib/"*.a "${staging_dir}/usr/lib/" 2>/dev/null || true
fi

if [[ -d "${media_out}/include" ]]; then
  cp -a "${media_out}/include/." "${staging_dir}/usr/include/" 2>/dev/null || true
fi
if [[ -d "${media_out}/lib" ]]; then
  cp -a "${media_out}/lib/"*.so* "${staging_dir}/usr/lib/" 2>/dev/null || true
fi

echo "=== Generating Relocatable SDK ==="
work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT
sdk_stage="${work_dir}/openhd-luckfox-pico-sdk"
mkdir -p "${sdk_stage}"

# Copy host tools, toolchain, and target sysroot
cp -a "${host_dir}/." "${sdk_stage}/"

# Buildroot make sdk populates relocate-sdk.sh and environment-setup
if make -C "${buildroot_dir}" sdk 2>/dev/null; then
  echo "Buildroot make sdk completed"
  if [[ -f "${host_dir}/relocate-sdk.sh" ]]; then
    cp -a "${host_dir}/relocate-sdk.sh" "${sdk_stage}/"
  fi
  if [[ -f "${host_dir}/environment-setup" ]]; then
    cp -a "${host_dir}/environment-setup" "${sdk_stage}/"
  fi
fi

# Ensure relocate-sdk.sh exists
if [[ ! -f "${sdk_stage}/relocate-sdk.sh" ]]; then
  cat >"${sdk_stage}/relocate-sdk.sh" <<'EOF'
#!/usr/bin/env bash
set -e
SDK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "Relocating SDK to ${SDK_DIR}..."
EOF
  chmod +x "${sdk_stage}/relocate-sdk.sh"
fi

# Ensure environment-setup exists
cat >"${sdk_stage}/environment-setup" <<'EOF'
SDK_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export SDK_PATH
export PATH="${SDK_PATH}/bin:${SDK_PATH}/sbin:${PATH}"
export ARCH="arm"
export CROSS_COMPILE="arm-rockchip830-linux-uclibcgnueabihf-"
export CC="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc"
export CXX="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-g++"
export AR="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-ar"
export AS="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-as"
export LD="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-ld"
export NM="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-nm"
export OBJCOPY="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-objcopy"
export OBJDUMP="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-objdump"
export RANLIB="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-ranlib"
export READELF="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-readelf"
export STRIP="${SDK_PATH}/bin/arm-rockchip830-linux-uclibcgnueabihf-strip"

if [[ -d "${SDK_PATH}/arm-rockchip830-linux-uclibcgnueabihf/sysroot" ]]; then
  export STAGING_DIR="${SDK_PATH}/arm-rockchip830-linux-uclibcgnueabihf/sysroot"
elif [[ -d "${SDK_PATH}/sysroot" ]]; then
  export STAGING_DIR="${SDK_PATH}/sysroot"
else
  export STAGING_DIR="${SDK_PATH}"
fi

export PKG_CONFIG="${SDK_PATH}/bin/pkg-config"
export PKG_CONFIG_SYSROOT_DIR="${STAGING_DIR}"
export PKG_CONFIG_LIBDIR="${STAGING_DIR}/usr/lib/pkgconfig:${STAGING_DIR}/usr/share/pkgconfig"
export CMAKE_PREFIX_PATH="${STAGING_DIR}/usr"
EOF

# Ensure share/buildroot/toolchainfile.cmake exists
mkdir -p "${sdk_stage}/share/buildroot"
cat >"${sdk_stage}/share/buildroot/toolchainfile.cmake" <<'EOF'
get_filename_component(RELOCATED_HOST_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(EXISTS "${RELOCATED_HOST_DIR}/arm-rockchip830-linux-uclibcgnueabihf/sysroot")
  set(CMAKE_SYSROOT "${RELOCATED_HOST_DIR}/arm-rockchip830-linux-uclibcgnueabihf/sysroot")
elseif(EXISTS "${RELOCATED_HOST_DIR}/sysroot")
  set(CMAKE_SYSROOT "${RELOCATED_HOST_DIR}/sysroot")
endif()

set(CMAKE_C_COMPILER "${RELOCATED_HOST_DIR}/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc")
set(CMAKE_CXX_COMPILER "${RELOCATED_HOST_DIR}/bin/arm-rockchip830-linux-uclibcgnueabihf-g++")

set(CMAKE_C_FLAGS "" CACHE STRING "Buildroot CFLAGS")
set(CMAKE_CXX_FLAGS "" CACHE STRING "Buildroot CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "Buildroot LDFLAGS")

set(CMAKE_INSTALL_SO_NO_EXE 0)

set(CMAKE_PROGRAM_PATH "${RELOCATED_HOST_DIR}/bin")
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

luckfox_commit="$(git -C "${sdk_source}" rev-parse HEAD 2>/dev/null || echo "unknown")"
sdk_manifest="${sdk_stage}/openhd-luckfox-pico-sdk.manifest.json"
cat >"${sdk_manifest}" <<EOF
{
  "schema": 1,
  "platform": "luckfox-pico",
  "targets": ["rv1103", "rv1106"],
  "architecture": "armhf",
  "toolchain": "arm-rockchip830-linux-uclibcgnueabihf",
  "luckfox_sdk_commit": "${luckfox_commit}",
  "generated_at": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF

echo "=== Packaging SDK Archive ==="
mkdir -p "${output_dir}"
sdk_tarball="${output_dir}/openhd-luckfox-pico-sdk.tar.gz"

tar --numeric-owner --owner=0 --group=0 \
  -C "${sdk_stage}" \
  -czf "${sdk_tarball}" .

cp "${sdk_manifest}" "${output_dir}/openhd-luckfox-pico-sdk.manifest.json"

(
  cd "${output_dir}"
  sha256sum "openhd-luckfox-pico-sdk.tar.gz" >"openhd-luckfox-pico-sdk.tar.gz.sha256"

  for target in rv1106 rv1103; do
    cp -f "openhd-luckfox-pico-sdk.tar.gz" "openhd-luckfox-${target}-sdk.tar.gz"
    sha256sum "openhd-luckfox-${target}-sdk.tar.gz" >"openhd-luckfox-${target}-sdk.tar.gz.sha256"
    cp -f "openhd-luckfox-pico-sdk.manifest.json" "openhd-luckfox-${target}-sdk.manifest.json"
  done
)

echo "=== Luckfox Pico SDK created successfully ==="
ls -lh "${output_dir}/openhd-luckfox-pico-sdk"*
