#!/bin/sh
set -eu

if [ "${1:-}" != "--confirm" ]; then
    echo "Usage: $0 --confirm"
    echo "This flashes the connected AIR Artosyn module with S-V1.3.3-U."
    exit 2
fi

IMAGE=/ohd/storage/firmware/artosyn-v1.3.3/VT4-KT-2458-S-V1.3.3-U.img
EXPECTED=54f48b43519214d5c8cb1af2cc4380b033000e08e19ca630f5357ceb185831f2
ACTUAL=$(sha256sum "$IMAGE" | awk '{print $1}')
[ "$ACTUAL" = "$EXPECTED" ] || { echo "Firmware checksum mismatch"; exit 3; }

/ohd/etc/init.d/S99openhd stop >/dev/null 2>&1 || true
/ohd/etc/init.d/S97openhd-artosyn restart
sleep 5
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_ota_upgrade -f "$IMAGE"
sleep 10
/ohd/etc/init.d/S97openhd-artosyn restart
sleep 8
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_basic_info -V
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_minidb_config -R dev -H
echo "Air firmware flashed and persistent role set to DEV (1). Allow the module to reboot before verification."
