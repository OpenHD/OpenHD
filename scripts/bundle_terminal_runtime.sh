#!/usr/bin/env bash
# Bundle only terminal libraries required by this binary (including libtinfo).
set -euo pipefail
sysroot="$1"
stage="$2"
binary="$3"
pending=("${binary}")
copied=0
while ((${#pending[@]})); do
  current="${pending[0]}"
  pending=("${pending[@]:1}")
  while IFS= read -r soname; do
    [[ -n "${soname}" && ! -f "${stage}/usr/lib/${soname}" ]] || continue
    library="$(find "${sysroot}" -name "${soname}" -print -quit)"
    if [[ -z "${library}" || ! -f "${library}" ]]; then
      echo "Missing terminal runtime library: ${soname}" >&2
      exit 1
    fi
    mkdir -p "${stage}/usr/lib"
    cp -L "${library}" "${stage}/usr/lib/${soname}"
    pending+=("${stage}/usr/lib/${soname}")
    copied=1
  done < <(readelf -d "${current}" | awk -F'[][]' '/NEEDED.*lib(ncursesw?|curses|tinfo).*\.so/ {print $2}')
done
if ((copied)); then
  for directory in usr/share/terminfo lib/terminfo etc/terminfo; do
    if [[ -d "${sysroot}/${directory}" ]]; then
      mkdir -p "${stage}/usr/share/terminfo"
      cp -a "${sysroot}/${directory}/." "${stage}/usr/share/terminfo/"
    fi
  done
fi
