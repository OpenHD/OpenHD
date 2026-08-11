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
script_dir="$(cd "$(dirname "$0")" && pwd)"
toolchain_file="${script_dir}/../OpenHD/cmake/portable-linux-toolchain.cmake"
test -f "${toolchain_file}"
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

OPENHD_SYSROOT="${sysroot}" OPENHD_CROSS_TRIPLET=aarch64-linux-gnu \
cmake -S "${work_dir}/mpp" -B "${work_dir}/build" \
  -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" \
  -DCMAKE_C_FLAGS=-mno-outline-atomics \
  -DCMAKE_CXX_FLAGS=-mno-outline-atomics \
  -DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TEST=OFF \
  -DBUILD_SHARED_LIBS=ON
cmake --build "${work_dir}/build" --parallel "$(nproc)"
DESTDIR="${sysroot}" cmake --install "${work_dir}/build"

# MPP installs its public headers below include/rockchip, while its upstream
# pkg-config templates advertise only include/. Correct the installed metadata
# so pkg-config consumers can include <rk_mpi.h> and the other public headers.
for pc_file in \
  "${sysroot}/usr/lib/pkgconfig/rockchip_mpp.pc" \
  "${sysroot}/usr/lib/pkgconfig/rockchip_vpu.pc"; do
  test -f "${pc_file}"
  sed -i 's|^includedir=${prefix}/include$|includedir=${prefix}/include/rockchip|' \
    "${pc_file}"
done

test -f "${sysroot}/usr/include/rockchip/rk_mpi.h"
find "${sysroot}/usr/lib" -name 'librockchip_mpp.so*' -print -quit | grep -q .
printf '%s\n' "rockchip_mpp_commit=${commit}" \
  >>"${sysroot}/openhd-sysroot.manifest"
