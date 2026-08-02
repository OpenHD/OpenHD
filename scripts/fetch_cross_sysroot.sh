#!/usr/bin/env bash
set -euo pipefail

url="${1:?Usage: fetch_cross_sysroot.sh <url> <sha256> <output-dir>}"
expected_sha256="${2:?Usage: fetch_cross_sysroot.sh <url> <sha256> <output-dir>}"
output_dir="$(realpath -m "${3:?Usage: fetch_cross_sysroot.sh <url> <sha256> <output-dir>}")"

if [[ ! "${expected_sha256}" =~ ^[0-9a-fA-F]{64}$ ]]; then
  echo "The expected SHA-256 must contain exactly 64 hexadecimal characters." >&2
  exit 1
fi
case "${output_dir}" in
  /|/usr|/opt|/var|/home|"$(pwd)")
    echo "Refusing unsafe sysroot output directory: ${output_dir}" >&2
    exit 1
    ;;
esac

archive="$(mktemp --suffix=.tar.zst)"
cleanup() {
  rm -f "${archive}"
}
trap cleanup EXIT

curl --fail --location --retry 3 --output "${archive}" "${url}"
actual_sha256="$(sha256sum "${archive}" | awk '{ print $1 }')"
if [[ "${actual_sha256,,}" != "${expected_sha256,,}" ]]; then
  echo "Sysroot checksum mismatch: expected ${expected_sha256}, got ${actual_sha256}" >&2
  exit 1
fi

rm -rf "${output_dir}"
mkdir -p "${output_dir}"
# The extract-only mmdebstrap archive may contain harmless /dev placeholders.
# Hosted GitHub runners cannot create device nodes and the compiler sysroot does
# not need them, so leave that subtree out when unpacking.
tar --zstd --exclude='./dev/*' -xf "${archive}" -C "${output_dir}"
test -f "${output_dir}/openhd-sysroot.manifest"
echo "Installed verified OpenHD sysroot at ${output_dir}"
