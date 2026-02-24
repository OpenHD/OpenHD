#!/bin/sh
set -u

LOG_DIR="/Config/openhd/logs"
mkdir -p "${LOG_DIR}"

TS="$(date -u +'%Y%m%d_%H%M%S')"
PREFIX="${LOG_DIR}/last_crash_${TS}"

# Capture previous boot logs if available.
journalctl -k -b -1 --no-pager > "${PREFIX}_kernel.log" 2>&1 || true
journalctl -u openhd -b -1 --no-pager > "${PREFIX}_openhd.log" 2>&1 || true
