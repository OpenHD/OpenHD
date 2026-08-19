#!/bin/bash
set -e

SDK_ROOT=$(cd "$(dirname "$0")/.." && pwd)

usage() {
    echo "Usage: $0 [all|x86_64|x86|arm64|arm]"
}

clean_arch() {
    local arch="$1"
    local build_dir="${SDK_ROOT}/build/${arch}"
    local install_dir="${SDK_ROOT}/install/${arch}"

    echo "Cleaning ${arch} build outputs..."
    rm -rf "${build_dir}"
    rm -rf "${install_dir}"
}

target="${1:-all}"

case "${target}" in
    all)
        clean_arch "x86_64"
        clean_arch "arm64"
        ;;
    x86_64|x86)
        clean_arch "x86_64"
        ;;
    arm64|arm)
        clean_arch "arm64"
        ;;
    -h|--help)
        usage
        exit 0
        ;;
    *)
        usage
        exit 1
        ;;
esac

echo "Clean done."
