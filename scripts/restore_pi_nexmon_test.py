#!/usr/bin/env python3
"""Restore the stock firmware saved by the September 2026 Pi 4 scout test.

Run locally on the test Pi as root, via Ethernet or its console. The test
module was loaded with insmod only; the installed module remains stock.
"""
import os
import shutil
import subprocess
import time
from pathlib import Path


def restore():
    if os.geteuid() != 0:
        raise SystemExit('Run with sudo on the test Pi.')
    backup = Path('/var/lib/openhd/nexmon-backup-20260906')
    original = backup / 'firmware.bin'
    if not original.is_file() or original.stat().st_size < 100000:
        raise SystemExit('Stock firmware backup missing or unexpectedly small.')
    uuid = (backup / 'hotspot-uuid').read_text().strip()
    # Restore on disk first, so a reboot also recovers if driver removal hangs.
    shutil.copyfile(original, '/lib/firmware/cypress/cyfmac43455-sdio.bin')
    os.sync()
    commands = [
        ['nmcli', 'device', 'set', 'wlan0', 'managed', 'no'],
        ['ip', 'link', 'set', 'ohdmon', 'down'],
        ['/usr/sbin/iw', 'dev', 'ohdmon', 'del'],
        ['/usr/sbin/modprobe', '-r', 'brcmfmac'],
    ]
    for command in commands:
        try:
            subprocess.run(command, timeout=15, check=False)
        except subprocess.TimeoutExpired:
            raise SystemExit('Stock firmware restored on disk; reboot to unload the stalled driver.')
    subprocess.run(['/usr/sbin/modprobe', 'brcmfmac'], check=True, timeout=15)
    time.sleep(2)
    subprocess.run(['nmcli', 'device', 'set', 'wlan0', 'managed', 'yes'], check=True, timeout=15)
    subprocess.run(['nmcli', 'connection', 'up', 'uuid', uuid], check=True, timeout=30)
    subprocess.run(['/usr/sbin/iw', 'dev'], check=True)


if __name__ == '__main__':
    restore()
