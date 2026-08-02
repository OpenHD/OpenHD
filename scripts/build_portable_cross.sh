#!/usr/bin/env bash
set -euo pipefail

architecture="${1:?Usage: build_portable_cross.sh <arm64|armhf> <sysroot> [build-dir]}"
sysroot="$(realpath "${2:?Usage: build_portable_cross.sh <arm64|armhf> <sysroot> [build-dir]}")"
build_dir="${3:-/tmp/openhd-cross-${architecture}}"

case "${architecture}" in
  arm64) triplet="aarch64-linux-gnu" ;;
  armhf) triplet="arm-linux-gnueabihf" ;;
  *)
    echo "Unsupported architecture: ${architecture}" >&2
    exit 1
    ;;
esac

build_dir="$(realpath -m "${build_dir}")"
case "${build_dir}" in
  /|/usr|/opt|/var|/home|"$(pwd)")
    echo "Refusing unsafe cross-build directory: ${build_dir}" >&2
    exit 1
    ;;
esac

test -f "${sysroot}/openhd-sysroot.manifest"
grep -qx "architecture=${architecture}" "${sysroot}/openhd-sysroot.manifest"
command -v "${triplet}-g++-10" >/dev/null

export OPENHD_SYSROOT="${sysroot}"
export OPENHD_CROSS_TRIPLET="${triplet}"
toolchain_file="$(pwd)/OpenHD/cmake/portable-linux-toolchain.cmake"
export OPENHD_CMAKE_TOOLCHAIN_FILE="${toolchain_file}"
export OPENHD_ARTOSYN_FORCE_SOURCE_BUILD=1
poco_config="$(find "${sysroot}/usr" -name PocoConfig.cmake -print -quit)"
if [[ -z "${poco_config}" ]]; then
  echo "PocoConfig.cmake is missing from the target sysroot." >&2
  exit 1
fi
poco_dir="$(dirname "${poco_config}")"
sdl2_config="$(find "${sysroot}/usr" \( -name sdl2-config.cmake -o -name SDL2Config.cmake \) -print -quit)"
if [[ -z "${sdl2_config}" ]]; then
  echo "SDL2 CMake configuration is missing from the target sysroot." >&2
  exit 1
fi
sdl2_dir="$(dirname "${sdl2_config}")"

source OpenHD/scripts/resolve_artosyn_sdk.sh
resolve_artosyn_sdk
if [[ "${OPENHD_REQUIRE_ARTOSYN:-1}" == "1" &&
      ( -z "${ARTOSYN_SDK_ROOT:-}" || -z "${ARTOSYN_SDK_LIB:-}" ) ]]; then
  echo "Artosyn SDK is required but could not be cross-built." >&2
  exit 1
fi

rm -rf "${build_dir}"
cmake -S OpenHD -B "${build_dir}" \
  -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DPoco_DIR="${poco_dir}" \
  -DSDL2_DIR="${sdl2_dir}" \
  -DENABLE_LIBCAMERA=OFF \
  -DARTOSYN_SDK_ROOT="${ARTOSYN_SDK_ROOT:-}" \
  -DARTOSYN_SDK_LIB="${ARTOSYN_SDK_LIB:-}" \
  -DARTOSYN_SDK_DAEMON="${ARTOSYN_SDK_DAEMON:-}" \
  -DARTOSYN_SDK_TUNTAP="${ARTOSYN_SDK_TUNTAP:-}"
cmake --build "${build_dir}" --parallel "$(nproc)"

binary="${build_dir}/openhd"
test -f "${binary}"
if readelf -d "${binary}" | grep -qi libcamera; then
  echo "Cross-built OpenHD unexpectedly links libcamera." >&2
  exit 1
fi

file "${binary}"
readelf -d "${binary}" | grep NEEDED
echo "Portable ${architecture} core built at ${binary}"
