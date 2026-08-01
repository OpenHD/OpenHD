#!/usr/bin/env bash
set -euo pipefail

architecture="${1:?Usage: create_cross_sysroot.sh <arm64|armhf> <output-dir> [suite]}"
output_dir="${2:?Usage: create_cross_sysroot.sh <arm64|armhf> <output-dir> [suite]}"
suite="${3:-bullseye}"
mirror="${OPENHD_DEBIAN_MIRROR:-http://deb.debian.org/debian}"

case "${architecture}" in
  arm64|armhf) ;;
  *)
    echo "Unsupported Debian architecture: ${architecture}" >&2
    exit 1
    ;;
esac

output_dir="$(realpath -m "${output_dir}")"
case "${output_dir}" in
  /|/usr|/opt|/var|/home|"$(pwd)")
    echo "Refusing unsafe sysroot output directory: ${output_dir}" >&2
    exit 1
    ;;
esac

if ! command -v mmdebstrap >/dev/null 2>&1; then
  echo "mmdebstrap is required to create the cross sysroot." >&2
  exit 1
fi

packages=(
  libc6-dev
  linux-libc-dev
  libstdc++-10-dev
  libpoco-dev
  libusb-1.0-0-dev
  libpcap-dev
  libsodium-dev
  libnl-3-dev
  libnl-genl-3-dev
  libnl-route-3-dev
  libsdl2-dev
  libgstreamer1.0-dev
  libgstreamer-plugins-base1.0-dev
  libv4l-dev
)
package_csv="$(IFS=,; echo "${packages[*]}")"

rm -rf "${output_dir}"
mkdir -p "${output_dir}"

# The extract variant downloads and unpacks the requested packages and their
# hard dependencies without executing target-architecture maintainer scripts.
# It therefore needs neither QEMU nor a target runner.
mmdebstrap \
  --mode=root \
  --variant=extract \
  --architectures="${architecture}" \
  --include="${package_csv}" \
  --components=main \
  "${suite}" \
  "${output_dir}" \
  "${mirror}"

printf '%s\n' \
  "format=1" \
  "suite=${suite}" \
  "architecture=${architecture}" \
  "mirror=${mirror}" \
  "packages=${package_csv}" \
  "generated_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
  >"${output_dir}/openhd-sysroot.manifest"

test -f "${output_dir}/usr/include/Poco/Poco.h"
test -f "${output_dir}/usr/include/gstreamer-1.0/gst/gst.h"
find "${output_dir}/usr/lib" -name 'libPocoFoundation.so*' -print -quit | grep -q .

echo "Created ${suite}/${architecture} OpenHD sysroot at ${output_dir}"
