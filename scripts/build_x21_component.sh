#!/usr/bin/env bash
# Backward compatibility wrapper for Makefile build system
set -euo pipefail
usage="Usage: build_x21_component.sh <sdk-dir> <output-dir>"
sdk_dir="${1:?${usage}}"
out_dir="${2:?${usage}}"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec make -C "${repo_root}" x21 SDK="${sdk_dir}" OUT="${out_dir}"
