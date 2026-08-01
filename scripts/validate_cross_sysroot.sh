#!/usr/bin/env bash
set -euo pipefail

architecture="${1:?Usage: validate_cross_sysroot.sh <arm64|armhf> <sysroot>}"
sysroot="$(realpath "${2:?Usage: validate_cross_sysroot.sh <arm64|armhf> <sysroot>}")"

case "${architecture}" in
  arm64) triplet="aarch64-linux-gnu" ;;
  armhf) triplet="arm-linux-gnueabihf" ;;
  *)
    echo "Unsupported architecture: ${architecture}" >&2
    exit 1
    ;;
esac

grep -qx "architecture=${architecture}" "${sysroot}/openhd-sysroot.manifest"
command -v "${triplet}-g++" >/dev/null

export PKG_CONFIG_SYSROOT_DIR="${sysroot}"
export PKG_CONFIG_LIBDIR="${sysroot}/usr/lib/${triplet}/pkgconfig:${sysroot}/usr/lib/pkgconfig:${sysroot}/usr/share/pkgconfig"
gcc_runtime_dir="$(find "${sysroot}/usr/lib/gcc/${triplet}" -mindepth 1 -maxdepth 1 -type d -print -quit)"
if [[ -z "${gcc_runtime_dir}" ]]; then
  echo "Target GCC runtime directory is missing from the sysroot." >&2
  exit 1
fi

output="$(mktemp)"
trap 'rm -f "${output}"' EXIT
printf '%s\n' \
  '#include <Poco/Net/IPAddress.h>' \
  '#include <gst/gst.h>' \
  'int main() { Poco::Net::IPAddress address; gst_init(nullptr, nullptr); return address.isWildcard(); }' \
  | "${triplet}-g++" --sysroot="${sysroot}" -x c++ - \
      $(pkg-config --cflags --libs gstreamer-1.0) \
      -L"${gcc_runtime_dir}" \
      -L"${sysroot}/usr/lib/${triplet}" \
      -L"${sysroot}/lib/${triplet}" \
      -Wl,-rpath-link,"${sysroot}/usr/lib/${triplet}" \
      -Wl,-rpath-link,"${sysroot}/lib/${triplet}" \
      -pthread \
      -lPocoNet -lPocoFoundation \
      -o "${output}"

file "${output}"
readelf -d "${output}" | grep NEEDED
echo "Validated ${architecture} OpenHD cross sysroot."
