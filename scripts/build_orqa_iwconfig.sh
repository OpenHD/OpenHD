#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 <orqa-sdk-root> [output-file]" >&2
  exit 1
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
sdk_root="$(realpath "$1")"
output_file="$(realpath -m "${2:-${repo_root}/build-orqa/iwconfig}")"
setup_script="${sdk_root}/environment-setup-armv8a-poky-linux"

if [[ ! -f "${setup_script}" ]]; then
  echo "Unable to find the Orqa SDK environment at ${setup_script}" >&2
  exit 1
fi

# shellcheck source=/dev/null
source "${setup_script}"

: "${CC:?Orqa SDK did not set CC}"
: "${AR:?Orqa SDK did not set AR}"
: "${RANLIB:?Orqa SDK did not set RANLIB}"
: "${READELF:?Orqa SDK did not set READELF}"
: "${STRIP:?Orqa SDK did not set STRIP}"

version="30.pre9"
archive="wireless_tools.${version}.tar.gz"
url="https://hewlettpackard.github.io/wireless-tools/${archive}"
sha256="abd9c5c98abf1fdd11892ac2f8a56737544fe101e1be27c6241a564948f34c63"
work_dir="$(mktemp -d)"
cleanup() {
  rm -rf "${work_dir}"
}
trap cleanup EXIT

curl --fail --location --retry 5 --output "${work_dir}/${archive}" "${url}"
printf '%s  %s\n' "${sha256}" "${work_dir}/${archive}" | sha256sum --check
tar -xf "${work_dir}/${archive}" -C "${work_dir}"

source_dir="${work_dir}/wireless_tools.30"
make -C "${source_dir}" \
  CC="${CC}" \
  AR="${AR}" \
  RANLIB="${RANLIB}" \
  CFLAGS="${CFLAGS:-} -Os -I." \
  LDFLAGS="${LDFLAGS:-}" \
  iwconfig

mkdir -p "$(dirname "${output_file}")"
install -m 0755 "${source_dir}/iwconfig" "${output_file}"
"${READELF}" -h "${output_file}" | grep -q 'Machine:.*AArch64'
if "${READELF}" -d "${output_file}" | grep -q 'Shared library: \[libiw'; then
  echo "Orqa iwconfig unexpectedly requires a shared libiw runtime." >&2
  exit 1
fi
"${STRIP}" --strip-unneeded "${output_file}"
echo "Orqa iwconfig built at ${output_file}"
