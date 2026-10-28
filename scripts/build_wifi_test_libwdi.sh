#!/usr/bin/env bash
set -euo pipefail
# Optional Windows GUI dependency only; mirror libwdi's upstream MinGW build.
prefix="$(pwd)/out/libwdi-prefix"
wdk="$(cygpath -m "$(pwd)/out/wdk-redist/Windows Kits/8.0")"
cd out/libwdi-source
./bootstrap.sh
./configure --build=x86_64-w64-mingw32 --host=x86_64-w64-mingw32 \
  --disable-32bit --enable-64bit --enable-shared --disable-static \
  --disable-examples-build --disable-debug --with-wdfver=1011 \
  --with-wdkdir="$wdk" --prefix="$prefix"
make -j 4
make install
