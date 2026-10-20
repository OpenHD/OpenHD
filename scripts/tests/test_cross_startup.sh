#!/usr/bin/env bash
set -euo pipefail

architecture="${1:?Usage: test_cross_startup.sh <arm64|armhf> <sysroot> [build-dir]}"
sysroot="$(realpath "${2:?Missing sysroot}")"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${3:-$(mktemp -d)}"
if [[ $# -lt 3 ]]; then
  trap 'rm -rf "${build_dir}"' EXIT
fi
case "${architecture}" in
  armhf) triplet=arm-linux-gnueabihf; emulator=qemu-arm ;;
  arm64) triplet=aarch64-linux-gnu; emulator=qemu-aarch64 ;;
  *) echo "Unsupported architecture: ${architecture}" >&2; exit 1 ;;
esac
export OPENHD_SYSROOT="${sysroot}"
export OPENHD_CROSS_TRIPLET="${triplet}"
cmake -S "${repo_root}/scripts/tests/cross_startup" -B "${build_dir}" \
  -DCMAKE_TOOLCHAIN_FILE="${repo_root}/OpenHD/cmake/portable-linux-toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "${build_dir}" --parallel 2
# Run against the actual baseline loader/libc, not the CI host's newer libc.
"${emulator}" -L "${sysroot}" "${build_dir}/cross_startup"
