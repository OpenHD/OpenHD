#!/usr/bin/env bash
# Backward compatibility wrapper for Makefile build system
set -euo pipefail
usage="Usage: build_lyra_ground_component.sh <sdk-or-sysroot-dir> <output-dir>"
sdk_dir="${1:?${usage}}"
out_dir="${2:?${usage}}"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec make -C "${repo_root}" lyra SDK="${sdk_dir}" OUT="${out_dir}"
