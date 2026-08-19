#!/bin/sh
set -eu

BASE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BACKUP=/ohd/storage/backups/artosyn-pre-v1.3.3

/ohd/etc/init.d/S99openhd stop >/dev/null 2>&1 || true
/ohd/etc/init.d/S97openhd-artosyn stop >/dev/null 2>&1 || true
mkdir -p "$BACKUP" /ohd/usr/bin /ohd/usr/lib /ohd/storage/firmware/artosyn-v1.3.3

for name in openhd artosyn_daemon tuntap_bb; do
    if [ -f "/ohd/usr/bin/$name" ] && [ ! -f "$BACKUP/$name" ]; then
        cp -p "/ohd/usr/bin/$name" "$BACKUP/$name"
    fi
done
for name in libar8030_client.so libusb-1.0.so.0.3.0; do
    if [ -f "/ohd/usr/lib/$name" ] && [ ! -f "$BACKUP/$name" ]; then
        cp -p "/ohd/usr/lib/$name" "$BACKUP/$name"
    fi
done
if [ -f /ohd/etc/init.d/S99openhd ] && [ ! -f "$BACKUP/S99openhd" ]; then
    cp -p /ohd/etc/init.d/S99openhd "$BACKUP/S99openhd"
fi

cp -f "$BASE/lib/libar8030_client.so" /ohd/usr/lib/libar8030_client.so
chmod 755 /ohd/usr/lib/libar8030_client.so
cp -f "$BASE/lib/libusb-1.0.so.0.3.0" /ohd/usr/lib/libusb-1.0.so.0.3.0
ln -sf libusb-1.0.so.0.3.0 /ohd/usr/lib/libusb-1.0.so.0
chmod 755 /ohd/usr/lib/libusb-1.0.so.0.3.0
for tool in "$BASE"/bin/l4_*; do
    cp -f "$tool" /ohd/usr/bin/
    chmod 755 "/ohd/usr/bin/$(basename "$tool")"
done
cp -f "$BASE/bin/l4_daemon" /ohd/usr/bin/artosyn_daemon
cp -f "$BASE/bin/l4_tuntap" /ohd/usr/bin/tuntap_bb
cp -f "$BASE/bin/openhd" /ohd/usr/bin/openhd
chmod 755 /ohd/usr/bin/artosyn_daemon /ohd/usr/bin/tuntap_bb /ohd/usr/bin/openhd
cp -f "$BASE"/firmware/* /ohd/storage/firmware/artosyn-v1.3.3/

# The stock init script relies on PATH and omits the runtime library path.
# Make startup deterministic while preserving the rest of the board script.
sed -i 's|^COMMAND=.*|COMMAND="env LD_LIBRARY_PATH=/ohd/usr/lib:/usr/lib /ohd/usr/bin/openhd -a"|' /ohd/etc/init.d/S99openhd
chmod 755 /ohd/etc/init.d/S99openhd

/ohd/etc/init.d/S97openhd-artosyn start
sleep 5
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_basic_info -V
echo "Air runtime installed. OpenHD remains stopped until firmware and socket tests pass."
