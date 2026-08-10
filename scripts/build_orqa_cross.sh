#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 <orqa-sdk-root> [build-dir]" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
sdk_root="$(realpath "$1")"
build_dir="$(realpath -m "${2:-${repo_root}/build-orqa}")"
setup_script="${sdk_root}/environment-setup-armv8a-poky-linux"

case "${build_dir}" in
  /|/usr|/opt|/var|/home|"${repo_root}"|"${sdk_root}")
    echo "Refusing unsafe Orqa build directory: ${build_dir}" >&2
    exit 1
    ;;
esac

if [[ ! -f "${setup_script}" ]]; then
  echo "Unable to find the Orqa SDK environment at ${setup_script}" >&2
  exit 1
fi

# The Orqa SDK supplies the cross compiler and its matching Yocto target
# sysroot. Do not substitute the portable Debian sysroot: the controller runs
# Yocto Scarthgap/glibc 2.39 and has a different runtime ABI.
# shellcheck source=/dev/null
source "${setup_script}"

: "${SDKTARGETSYSROOT:?Orqa SDK did not set SDKTARGETSYSROOT}"
: "${CXX:?Orqa SDK did not set CXX}"
: "${READELF:?Orqa SDK did not set READELF}"
: "${STRIP:?Orqa SDK did not set STRIP}"

test -f "${SDKTARGETSYSROOT}/usr/lib/libPocoFoundation.so.95"
test -f "${SDKTARGETSYSROOT}/usr/lib/libPocoNet.so.95"
test -f "${SDKTARGETSYSROOT}/usr/lib/libsodium.so.26"
test -f "${SDKTARGETSYSROOT}/usr/lib/gstreamer-1.0/libgstperf.so"

rm -rf "${build_dir}"
cmake -S "${repo_root}/OpenHD" -B "${build_dir}" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_DISABLE_FIND_PACKAGE_SDL2=TRUE
cmake --build "${build_dir}" --parallel "$(nproc)"

binary="${build_dir}/openhd"
needed_file="${build_dir}/openhd-needed.txt"
gst_perf_plugin="${SDKTARGETSYSROOT}/usr/lib/gstreamer-1.0/libgstperf.so"
gst_needed_file="${build_dir}/gst-perf-needed.txt"
gst_strings_file="${build_dir}/gst-perf-strings.txt"

test -f "${binary}"
"${READELF}" -h "${binary}" | grep -q 'Machine:.*AArch64'
"${READELF}" -d "${binary}" | tee "${needed_file}"
grep -q 'Shared library: \[libPocoFoundation.so.95\]' "${needed_file}"
grep -q 'Shared library: \[libPocoNet.so.95\]' "${needed_file}"
grep -q 'Shared library: \[libsodium.so.26\]' "${needed_file}"
if grep -q 'Shared library: \[libSDL2' "${needed_file}"; then
  echo "SDL2 is not installed on the Orqa controller image." >&2
  exit 1
fi

"${READELF}" -d "${gst_perf_plugin}" | tee "${gst_needed_file}"
grep -q 'Shared library: \[libgstreamer-1.0.so.0\]' "${gst_needed_file}"
grep -q 'Shared library: \[libgstbase-1.0.so.0\]' "${gst_needed_file}"
strings "${gst_perf_plugin}" >"${gst_strings_file}"
grep -qx 'on-bitrate' "${gst_strings_file}"
grep -qx 'bitrate-interval' "${gst_strings_file}"

"${STRIP}" --strip-unneeded "${binary}"
echo "Orqa OpenHD binary built at ${binary}"
