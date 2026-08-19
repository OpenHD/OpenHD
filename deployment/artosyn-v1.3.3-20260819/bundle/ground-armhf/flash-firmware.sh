#!/bin/sh
set -eu

[ "$(id -u)" -eq 0 ] || { echo "Run with sudo"; exit 2; }
if [ "${1:-}" != "--confirm" ]; then
    echo "Usage: sudo $0 --confirm"
    echo "This flashes the connected GROUND Artosyn module with G-V1.3.3-U."
    exit 2
fi

IMAGE=/home/openhd/firmware/artosyn-v1.3.3/VT4-KT-2458-G-V1.3.3-U.img
EXPECTED=e32c45e4e87c35329e6561d16de6538470be93f0facb44770f1fbc90707ddd13
ACTUAL=$(sha256sum "$IMAGE" | awk '{print $1}')
[ "$ACTUAL" = "$EXPECTED" ] || { echo "Firmware checksum mismatch"; exit 3; }

systemctl stop openhd >/dev/null 2>&1 || true
pkill -x artosyn_daemon >/dev/null 2>&1 || true
nohup /usr/local/bin/artosyn_daemon -i 0 -p 50000 >>/var/log/openhd-artosyn-daemon.log 2>&1 &
sleep 5
/usr/local/bin/l4_ota_upgrade -f "$IMAGE"
sleep 10
pkill -x artosyn_daemon >/dev/null 2>&1 || true
nohup /usr/local/bin/artosyn_daemon -i 0 -p 50000 >>/var/log/openhd-artosyn-daemon.log 2>&1 &
sleep 8
/usr/local/bin/l4_basic_info -V
/usr/local/bin/l4_minidb_config -R ap -H
echo "Ground firmware flashed and persistent role set to AP (0). Allow the module to reboot before verification."
