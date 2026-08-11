#!/usr/bin/env bash
set -euo pipefail

architecture="${1:?Usage: install_rockchip_mpp_sysroot.sh <arm64> <sysroot>}"
sysroot="$(realpath "${2:?Usage: install_rockchip_mpp_sysroot.sh <arm64> <sysroot>}")"

if [[ "${architecture}" != "arm64" ]]; then
  echo "Rockchip RV1126 MPP is only installed for the arm64 sysroot."
  exit 0
fi
test -f "${sysroot}/openhd-sysroot.manifest"
grep -qx 'architecture=arm64' "${sysroot}/openhd-sysroot.manifest"

commit="2e93ab791e0b2c803022622c30fea79b80450830"
work_dir="$(mktemp -d -t openhd-rockchip-mpp.XXXXXXXX)"
cleanup() {
  case "${work_dir}" in
    /tmp/openhd-rockchip-mpp.*) rm -rf -- "${work_dir}" ;;
  esac
}
trap cleanup EXIT

git clone --quiet --filter=blob:none https://github.com/rockchip-linux/mpp.git \
  "${work_dir}/mpp"
git -C "${work_dir}/mpp" checkout --quiet "${commit}"

cmake -S "${work_dir}/mpp" -B "${work_dir}/build" \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_SYSROOT="${sysroot}" \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc-10 \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++-10 \
  -DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TEST=OFF \
  -DBUILD_SHARED_LIBS=ON
cmake --build "${work_dir}/build" --parallel "$(nproc)"
DESTDIR="${sysroot}" cmake --install "${work_dir}/build"

test -f "${sysroot}/usr/include/rockchip/rk_mpi.h"
find "${sysroot}/usr/lib" -name 'librockchip_mpp.so*' -print -quit | grep -q .
printf '%s\n' "rockchip_mpp_commit=${commit}" \
  >>"${sysroot}/openhd-sysroot.manifest"
