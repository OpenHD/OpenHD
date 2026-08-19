#!/bin/sh
set -eu

[ "$(id -u)" -eq 0 ] || { echo "Run with sudo"; exit 2; }
BASE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BACKUP=/home/openhd/openhd-backup-pre-v1.3.3

systemctl stop openhd >/dev/null 2>&1 || true
pkill -x artosyn_daemon >/dev/null 2>&1 || true
mkdir -p "$BACKUP" /usr/local/bin /usr/local/lib /home/openhd/firmware/artosyn-v1.3.3

for name in openhd artosyn_daemon tuntap_bb; do
    if [ -f "/usr/local/bin/$name" ] && [ ! -f "$BACKUP/$name" ]; then
        cp -p "/usr/local/bin/$name" "$BACKUP/$name"
    fi
done
if [ -f /usr/local/lib/libar8030_client.so ] && [ ! -f "$BACKUP/libar8030_client.so" ]; then
    cp -p /usr/local/lib/libar8030_client.so "$BACKUP/libar8030_client.so"
fi

install -m 755 "$BASE/lib/libar8030_client.so" /usr/local/lib/libar8030_client.so
for tool in "$BASE"/bin/l4_*; do
    install -m 755 "$tool" "/usr/local/bin/$(basename "$tool")"
done
install -m 755 "$BASE/bin/l4_daemon" /usr/local/bin/artosyn_daemon
install -m 755 "$BASE/bin/l4_tuntap" /usr/local/bin/tuntap_bb
install -m 755 "$BASE/bin/openhd" /usr/local/bin/openhd
install -m 644 "$BASE"/firmware/* /home/openhd/firmware/artosyn-v1.3.3/
ldconfig

nohup /usr/local/bin/artosyn_daemon -i 0 -p 50000 >>/var/log/openhd-artosyn-daemon.log 2>&1 &
sleep 5
/usr/local/bin/l4_basic_info -V
echo "Ground runtime installed. OpenHD remains stopped until firmware and socket tests pass."
