#!/bin/bash
set -e

SDK_ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR="${SDK_ROOT}/build/x86_64"
INSTALL_DIR="${SDK_ROOT}/install/x86_64"

# build type: Debug, Release, RelWithDebInfo, MinSizeRel
cmake -S "${SDK_ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DAPP_STATIC_LIB=OFF \
    -DUSING_8030USB=ON \
    -DUSING_8030UART=ON \
    -DUSING_8030SDIO=OFF \
    -DUSING_XDS_HDR=ON

num=$(grep -c processor /proc/cpuinfo)
cmake --build "${BUILD_DIR}" --target ar8030_client l4_cmd_dbg l4_tuntap l4_ota_upgrade l4_basic_info l4_pair_manager l4_link_monitor l4_link_config l4_config_file l4_minidb_config l4_uart_config l4_socket_transfer l4_project_event l4_daemon -j${num}
cmake --install "${BUILD_DIR}"
