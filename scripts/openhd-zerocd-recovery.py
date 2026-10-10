#!/usr/bin/env python3
"""Opt-in Pi 4 recovery for a lone Realtek radio stuck in driver-CD mode."""
import json
from pathlib import Path
import subprocess
import time

SYSROOT = Path('/sys/bus/usb/devices')
MODEL = Path('/proc/device-tree/model')
STATE = Path('/run/openhd-zerocd-recovery/state.json')
CU_IDS = {'c812', 'c82c', 'c82e'}


def read(path):
    try:
        return path.read_text().strip().strip('\0').lower()
    except OSError:
        return ''


def lone_zerocd(root):
    """Pi 4 switches ALL external USB power: refuse to disturb other devices."""
    targets = []
    for device in root.iterdir():
        vendor, product = read(device / 'idVendor'), read(device / 'idProduct')
        if not vendor:
            continue  # Interface entries have no device descriptor.
        if device.name in {'usb1', 'usb2'} and vendor == '1d6b':
            continue
        if device.name == '1-1' and (vendor, product) == ('2109', '3431'):
            continue
        if (vendor, product) != ('0bda', '1a2b'):
            return None
        if device.name not in {'1-1.1', '1-1.2', '1-1.3', '1-1.4'}:
            return None
        targets.append(device)
    return targets[0] if len(targets) == 1 else None


def command(*args):
    subprocess.run(args, check=True, timeout=25)


def recover(target, run=command, sleep=time.sleep):
    # Stop competing mode-switch attempts before cutting VBUS. udev is allowed
    # to perform its normal eject on the fresh device when power returns.
    try:
        try:
            run('systemctl', 'stop', 'openhd.service', 'openhd-sys-utils.service',
                'usb_modeswitch@' + target.name + '.service')
            run('uhubctl', '-l', '2', '-a', 'off')
            sleep(10)
        finally:
            try:
                run('uhubctl', '-l', '2', '-a', 'on')
            finally:
                run('systemctl', 'start', 'openhd-sys-utils.service')
        # Wait for enumeration before starting OpenHD: a startup without a
        # WBLink otherwise needs another restart to pick up the late radio.
        for _ in range(20):
            sleep(1)
            if read(target / 'idVendor') == '0bda' and read(target / 'idProduct') in CU_IDS:
                run('systemctl', 'restart', 'openhd-sys-utils.service')
                print('Realtek CU recovered from ZeroCD without physical access.', flush=True)
                return True
        print('ZeroCD recovery did not enumerate a CU radio; retry is bounded.', flush=True)
        return False
    finally:
        run('systemctl', 'start', 'openhd.service')


def tick(root=SYSROOT, model=MODEL, state_file=STATE, now=None, recovery=recover):
    if not read(model).startswith('raspberry pi 4 model b'):
        return
    now = time.monotonic() if now is None else now
    try:
        state = json.loads(state_file.read_text())
    except (OSError, ValueError):
        state = {}
    target = lone_zerocd(root)
    if target is None:
        state.pop('pending', None)
    else:
        instance = target.name + ':' + read(target / 'devnum')
        pending = state.get('pending', {})
        if pending.get('instance') != instance:
            pending = {'instance': instance, 'since': now}
            print('Realtek ZeroCD detected; allowing normal mode switching first.', flush=True)
        state['pending'] = pending
        if (now - pending['since'] >= 90 and state.get('attempts', 0) < 3
                and now - state.get('last_attempt', -120) >= 120):
            state['attempts'] = state.get('attempts', 0) + 1
            state['last_attempt'] = now
            # Save the budget BEFORE touching USB, including failed attempts.
            state_file.parent.mkdir(parents=True, exist_ok=True)
            state_file.write_text(json.dumps(state))
            print('Cycling Pi 4 USB power for stalled ZeroCD (attempt %d/3).' % state['attempts'], flush=True)
            recovery(target)
            state.pop('pending', None)
    state_file.parent.mkdir(parents=True, exist_ok=True)
    state_file.write_text(json.dumps(state))


if __name__ == '__main__':
    tick()
