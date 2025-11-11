#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <sdk-root> [poco-archive]" >&2
  exit 1
fi

sdk_root="$1"
shift || true

make_absolute_path() {
  local path="$1"
  if [[ "$path" == /* ]]; then
    printf '%s\n' "$path"
  else
    printf '%s/%s\n' "$(pwd)" "$path"
  fi
}

sdk_root="$(make_absolute_path "$sdk_root")"

output_archive="${1:-}"
if [[ -n "$output_archive" ]]; then
  shift || true
else
  output_archive="$sdk_root/poco-libraries.zip"
fi

output_archive="$(make_absolute_path "$output_archive")"

if [[ ! -d "$sdk_root" ]]; then
  echo "SDK root '$sdk_root' does not exist" >&2
  exit 1
fi

setup_script="$sdk_root/environment-setup-armv8a-poky-linux"
if [[ ! -f "$setup_script" ]]; then
  echo "Unable to find Yocto environment setup script at '$setup_script'" >&2
  exit 1
fi

# shellcheck source=/dev/null
source "$setup_script"

work_dir="$(mktemp -d)"
cleanup() {
  rm -rf "$work_dir"
}
trap cleanup EXIT

jobs="$(nproc)"

download() {
  local url="$1"
  local destination="$2"
  curl --fail --location --retry 5 --output "$destination" "$url"
}

package_poco_libs() {
  if ! command -v zip >/dev/null 2>&1; then
    echo "zip command not found; unable to package Poco libraries" >&2
    exit 1
  fi

  local archive="$1"
  local lib_dir="$SDKTARGETSYSROOT/usr/lib"
  if [[ ! -d "$lib_dir" ]]; then
    echo "Poco library directory '$lib_dir' does not exist" >&2
    exit 1
  fi

  local libs=()
  while IFS= read -r -d '' lib; do
    libs+=("$lib")
  done < <(find "$lib_dir" -maxdepth 1 \
    \( -name 'libPoco*.so*' -o -name 'libPoco*.a' \) -print0)

  if [[ ${#libs[@]} -eq 0 ]]; then
    echo "No Poco libraries were found in '$lib_dir'" >&2
    exit 1
  fi

  local staging_dir="$work_dir/poco_libs"
  rm -rf "$staging_dir"
  mkdir -p "$staging_dir"

  for lib in "${libs[@]}"; do
    cp -a "$lib" "$staging_dir/"
  done

  mkdir -p "$(dirname "$archive")"
  rm -f "$archive"
  pushd "$staging_dir" >/dev/null
  zip -9 -r "$archive" ./* >/dev/null
  popd >/dev/null
  echo "Packaged Poco libraries into $archive"
}

build_libsodium() {
  local version="1.0.19"
  local tarball="libsodium-${version}.tar.gz"
  local url="https://download.libsodium.org/libsodium/releases/${tarball}"
  local archive="$work_dir/$tarball"

  download "$url" "$archive"
  tar -xf "$archive" -C "$work_dir"
  local src_dir
  if [[ -d "$work_dir/libsodium-${version}" ]]; then
    src_dir="$work_dir/libsodium-${version}"
  else
    src_dir="$work_dir/libsodium-stable"
  fi
  pushd "$src_dir" >/dev/null
  ./configure --host="${OECORE_TARGET_ARCH}-poky-linux" --prefix="$SDKTARGETSYSROOT/usr"
  make -j"$jobs"
  make install
  popd >/dev/null
}

build_poco() {
  local version="1.12.4-release"
  local tarball="poco-${version}.tar.gz"
  local url="https://github.com/pocoproject/poco/archive/refs/tags/${tarball}"
  local archive="$work_dir/$tarball"

  download "$url" "$archive"
  tar -xf "$archive" -C "$work_dir"
  local src_dir
  src_dir="$(find "$work_dir" -maxdepth 1 -type d -name 'poco-*' -print | head -n 1)"
  if [[ -z "$src_dir" ]]; then
    echo "Failed to locate extracted Poco sources" >&2
    exit 1
  fi
  pushd "$src_dir" >/dev/null
  cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$SDKTARGETSYSROOT/usr" \
    -DENABLE_APACHECONNECTOR=OFF \
    -DENABLE_DATA=OFF \
    -DENABLE_DATA_SQLITE=OFF \
    -DENABLE_DATA_MYSQL=OFF \
    -DENABLE_DATA_ODBC=OFF \
    -DENABLE_DATA_POSTGRESQL=OFF \
    -DENABLE_JSON=OFF \
    -DENABLE_JWT=OFF \
    -DENABLE_MONGODB=OFF \
    -DENABLE_NETSSL=OFF \
    -DENABLE_PDF=OFF \
    -DENABLE_PROMETHEUS=OFF \
    -DENABLE_REDIS=OFF \
    -DENABLE_SEVENZIP=OFF \
    -DENABLE_TESTS=OFF \
    -DENABLE_ZIP=OFF \
    -DENABLE_ENCODINGS=OFF \
    -DENABLE_ACTIVERECORD=OFF \
    -DENABLE_ACTIVERECORD_COMPILER=OFF
  cmake --build build -j"$jobs"
  cmake --install build
  popd >/dev/null
}

build_libsodium
build_poco
package_poco_libs "$output_archive"
