#!/usr/bin/env python3
"""Record firmware tuning diagnostics and radiotap PCAPs on the test Pi."""
import argparse
import json
import os
from pathlib import Path
import socket
import struct
import subprocess
import sys
import time


def capture(path, frequency, seconds):
    packets = matching = bad_fcs = malformed = 0
    with socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(3)) as sock, \
            path.open('xb') as output:
        sock.bind(('ohdscout', 0))
        sock.settimeout(0.1)
        # Classic little-endian PCAP, LINKTYPE_IEEE802_11_RADIOTAP.
        output.write(struct.pack('<IHHIIII', 0xa1b2c3d4, 2, 4, 0, 0, 65535, 127))
        end = time.monotonic() + seconds
        while time.monotonic() < end:
            try:
                packet = sock.recv(65535)
            except socket.timeout:
                continue
            stamp = time.time_ns()
            output.write(struct.pack('<IIII', stamp//1000000000,
                                     stamp//1000 % 1000000, len(packet), len(packet)))
            output.write(packet)
            packets += 1
            if len(packet) < 24 or packet[:8] != b'\x00\x00\x18\x00\x6f\x00\x00\x00':
                malformed += 1
                continue
            if packet[16] & 0x40:
                bad_fcs += 1
            elif len(packet) >= 38 and struct.unpack_from('<H', packet, 18)[0] == frequency:
                matching += 1
    return dict(packets=packets, matching_frequency_good_fcs_packets=matching,
                bad_fcs_packets=bad_fcs, malformed_radiotap_packets=malformed)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bundle', type=Path, required=True)
    parser.add_argument('--helper', type=Path,
                        default=Path('/usr/local/libexec/openhd-nexmon-scout'))
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--phase', choices=['off', 'on'], required=True,
                        help='Whether the known test transmitter is sending')
    parser.add_argument('--frequencies', type=int, nargs='+', default=[5825, 5845, 5865])
    parser.add_argument('--seconds', type=float, default=10)
    args = parser.parse_args()
    if os.geteuid() != 0:
        parser.error('Run as root on the Pi, using Ethernet for access')
    if not 1 <= args.seconds <= 30:
        parser.error('--seconds must be between 1 and 30')
    if not 1 <= len(args.frequencies) <= 6 or any(
            f not in (5825, 5845, 5865, 5885, 5905, 5925) for f in args.frequencies):
        parser.error('Choose up to six supported probe frequencies')
    args.output.mkdir(parents=True, exist_ok=False)
    command = [sys.executable, str(args.helper.resolve())]
    def helper(*arguments):
        return subprocess.run(command + list(arguments), check=True, text=True,
                              stdout=subprocess.PIPE, timeout=45).stdout
    # start performs its own rollback on failure. Do not restore someone else's
    # lease if start refuses because a survey is already active.
    helper('start', '--bundle', str(args.bundle.resolve()))
    try:
        with (args.output/'results.jsonl').open('x') as results:
            for index, frequency in enumerate(args.frequencies):
                record = dict(phase=args.phase, hardware_verified=False)
                record.update(json.loads(helper('probe', str(frequency))))
                if record['capture_ready']:
                    path = args.output/f'{index}-{frequency}.pcap'
                    record.update(capture(path, frequency, args.seconds))
                    record['pcap'] = path.name
                print(json.dumps(record), flush=True)
                results.write(json.dumps(record) + '\n')
                results.flush()
    finally:
        helper('restore')


if __name__ == '__main__':
    main()
