#!/usr/bin/env bash
set -euo pipefail

architecture="${ARCHITECTURE:-$(dpkg --print-architecture)}"
version="${VERSION:-0.1.0}"
root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build="${root}/build-package"
stage="${build}/stage"

rm -rf "${build}"
cmake -S "${root}" -B "${build}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DTOPOTEK_BUILD_TESTS=OFF \
  -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "${build}" --parallel
DESTDIR="${stage}" cmake --install "${build}"

mkdir -p "${stage}/DEBIAN"
cat >"${stage}/DEBIAN/control" <<EOF
Package: openhd-plugin-topotek
Version: ${version}
Section: video
Priority: optional
Architecture: ${architecture}
Depends: openhd (>= 2.7)
Maintainer: OpenHD <contact@openhd.tech>
Description: Topotek Ethernet gimbal-camera plugin for OpenHD
 Controls Topotek SIP/HI-family cameras over the vendor UDP protocol.
EOF

output="openhd-plugin-topotek_${version}_${architecture}.deb"
dpkg-deb --build --root-owner-group "${stage}" "${root}/${output}"
echo "Built ${root}/${output}"
