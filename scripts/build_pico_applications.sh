#!/usr/bin/env bash
# Build applications only. The existing SDK and board images are never rebuilt.
set -euo pipefail
sdk="$(realpath "$1")"
sysutils="$(realpath "$2")"
output="$(realpath -m "$3")"
repo="$(cd "$(dirname "$0")/.." && pwd)"
bash "$repo/scripts/build_luckfox_component.sh" "$sdk" "$output/openhd"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
# build_luckfox_component.sh relocates the verified SDK and repairs its sysroot.
source "$sdk/environment-setup"
export STAGING_DIR="$sdk/arm-buildroot-linux-uclibcgnueabihf/sysroot"
export PKG_CONFIG_SYSROOT_DIR="$STAGING_DIR"
export PKG_CONFIG_LIBDIR="$STAGING_DIR/usr/lib/pkgconfig:$STAGING_DIR/usr/share/pkgconfig"
export HOST_CXX=/usr/bin/g++
cmake -S "$sysutils" -B "$work/build" \
  -DCMAKE_TOOLCHAIN_FILE="$sdk/share/buildroot/toolchainfile.cmake" \
  -DCMAKE_SYSROOT="$STAGING_DIR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build "$work/build" --parallel "$(nproc)" --target openhd_sys_utils
mkdir -p "$work/stage/usr/bin"
install -m755 "$work/build/openhd_sys_utils" "$work/stage/usr/bin/openhd_sys_utils"
"$STRIP" "$work/stage/usr/bin/openhd_sys_utils"
install -Dm644 "$sysutils/misc/openhd-devourer-eu.conf" "$work/stage/lib/modprobe.d/openhd-devourer-eu.conf"
install -Dm644 "$sysutils/misc/70-openhd-devourer-eu.rules" "$work/stage/lib/udev/rules.d/70-openhd-devourer-eu.rules"
install -Dm644 "$sysutils/misc/wifi_cards.json" "$work/stage/usr/local/share/OpenHD/SysUtils/wifi_cards.json"
# The base need not carry this helper; SysUtils uses it for storage-mode radios.
READELF= bash "$sysutils/scripts/build_usb_modeswitch.sh" "$work/stage/usr/sbin/usb_modeswitch"
"$READELF" -h "$work/stage/usr/sbin/usb_modeswitch" | grep -q 'Machine:.*ARM'
# Include every transitive ELF dependency of both applications from this SDK.
mkdir -p "$output/root"
tar -xzf "$(find "$output/openhd" -name 'openhd-luckfox-pico-*-evo-*.tar.gz' -print -quit)" -C "$output/root"
rm "$output/root/component-manifest.json" "$output/root/sha256sums"
cp -a "$work/stage/." "$output/root/"
/usr/bin/python3 "$repo/scripts/bundle_pico_runtime.py" "$output/root" "$STAGING_DIR"
if "$READELF" -d "$output/root/usr/bin/openhd" | grep -q 'NEEDED.*libgst'; then
  echo 'Pico native camera build must not depend on GStreamer' >&2
  exit 1
fi
test -s "$output/root/usr/lib/librockit.so"
test -s "$output/root/usr/lib/librkaiq.so"
export SYSUTILS_COMMIT="$(git -C "$sysutils" rev-parse HEAD)"
export OPENHD_COMMIT="$(git -C "$repo" rev-parse HEAD)"
export DEVOURER_COMMIT="$(git -C "$repo" rev-parse HEAD:OpenHD/ohd_interface/lib/devourer)"
/usr/bin/python3 - "$output" <<'PY'
import hashlib,json,os,pathlib
p=pathlib.Path(__import__('sys').argv[1]); root=p/'root'
manifest=dict(schema=1,platform='luckfox-pico',architecture='armhf',
 video_backend='native-pico-mpi',gstreamer=False,
 sdk_sha256=os.environ['LUCKFOX_SDK_SHA256'],
 sdk_buildroot_commit=os.environ['LUCKFOX_SDK_BUILDROOT_COMMIT'],
 source_commits={n:os.environ[n.upper()+'_COMMIT'] for n in ('openhd','sysutils','devourer')},
 files={str(f.relative_to(root)):hashlib.sha256(f.read_bytes()).hexdigest()
        for f in sorted(root.rglob('*')) if f.is_file()})
(p/'applications.json').write_text(json.dumps(manifest,indent=2)+'\n')
PY
tar --numeric-owner --owner=0 --group=0 -C "$output/root" -czf "$output/applications.tar.gz" .
(cd "$output"; sha256sum applications.tar.gz applications.json > applications.sha256)
rm -rf "$output/root" "$output/openhd"
