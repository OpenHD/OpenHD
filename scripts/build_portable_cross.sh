#!/usr/bin/env bash
set -euo pipefail

architecture="${1:?Usage: build_portable_cross.sh <arm64|armhf> <sysroot> [build-dir]}"
sysroot="$(realpath "${2:?Usage: build_portable_cross.sh <arm64|armhf> <sysroot> [build-dir]}")"
build_dir="${3:-/tmp/openhd-cross-${architecture}}"

case "${architecture}" in
  arm64)
    triplet="aarch64-linux-gnu"
    expected_elf_machine="AArch64"
    ;;
  armhf)
    triplet="arm-linux-gnueabihf"
    expected_elf_machine="ARM"
    ;;
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
if [[ "${OPENHD_REQUIRE_ARTOSYN_DAEMON:-0}" == "1" &&
      ( -z "${ARTOSYN_SDK_DAEMON:-}" || ! -f "${ARTOSYN_SDK_DAEMON}" ) ]]; then
  echo "Artosyn daemon is required but could not be cross-built." >&2
  exit 1
fi

validate_target_artifact() {
  local label="$1"
  local artifact="$2"
  local machines=""
  [[ -f "${artifact}" ]] || {
    echo "Missing ${label}: ${artifact}" >&2
    return 1
  }
  machines="$(readelf -h "${artifact}" 2>/dev/null |
    sed -n 's/^[[:space:]]*Machine:[[:space:]]*//p' | sort -u)"
  if [[ -z "${machines}" ]]; then
    echo "${label} is not an ELF binary or ELF archive: ${artifact}" >&2
    return 1
  fi
  while IFS= read -r machine; do
    if [[ "${machine}" != "${expected_elf_machine}" ]]; then
      echo "${label} has target '${machine}', expected '${expected_elf_machine}': ${artifact}" >&2
      return 1
    fi
  done <<<"${machines}"
}

if [[ -n "${ARTOSYN_SDK_LIB:-}" ]]; then
  IFS=';' read -ra artosyn_sdk_libs <<<"${ARTOSYN_SDK_LIB}"
  for artosyn_sdk_lib in "${artosyn_sdk_libs[@]}"; do
    validate_target_artifact "Artosyn SDK library" "${artosyn_sdk_lib}"
  done
fi
if [[ -n "${ARTOSYN_SDK_DAEMON:-}" ]]; then
  validate_target_artifact "Artosyn daemon" "${ARTOSYN_SDK_DAEMON}"
fi
if [[ -n "${ARTOSYN_SDK_TUNTAP:-}" ]]; then
  validate_target_artifact "Artosyn tuntap helper" "${ARTOSYN_SDK_TUNTAP}"
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
if ! find "${build_dir}" -type f -name 'artosyn_link.cpp.o' -print -quit |
    grep -q .; then
  echo "Cross-built OpenHD omitted the Artosyn link backend." >&2
  exit 1
fi
if readelf -d "${binary}" | grep -qi libcamera; then
  echo "Cross-built OpenHD unexpectedly links libcamera." >&2
  exit 1
fi

file "${binary}"
readelf -d "${binary}" | grep NEEDED
env_file="${build_dir}/openhd-cross.env"
{
  printf 'export ARTOSYN_SDK_ROOT=%q\n' "${ARTOSYN_SDK_ROOT:-}"
  printf 'export ARTOSYN_SDK_LIB=%q\n' "${ARTOSYN_SDK_LIB:-}"
  printf 'export ARTOSYN_SDK_DAEMON=%q\n' "${ARTOSYN_SDK_DAEMON:-}"
  printf 'export ARTOSYN_SDK_TUNTAP=%q\n' "${ARTOSYN_SDK_TUNTAP:-}"
} >"${env_file}"
echo "Portable ${architecture} core built at ${binary}"
