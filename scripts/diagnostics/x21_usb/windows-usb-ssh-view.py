"""Direct TCP video over USB Ethernet; SSH keeps local telemetry forwarding active."""
import argparse
import shlex
import os
import socket
import struct
import time
import threading
import subprocess
from pathlib import Path
import paramiko

udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--identity', required=True, type=Path)
parser.add_argument('--known-hosts', type=Path, default=Path.home()/'.ssh/known_hosts')
parser.add_argument('--relay', default='/ohd/x21-diagnostics/usb-video-relay')
args = parser.parse_args()
key = args.identity

def exact(channel, count):
    data = bytearray()
    while len(data) < count:
        chunk = channel.recv(count - len(data))
        if not chunk:
            raise EOFError('video relay closed')
        data.extend(chunk)
    return data

while True:
    ssh = paramiko.SSHClient()
    ssh.load_host_keys(str(args.known_hosts))
    try:
        from x21_usb_link import usb_link
        interface,source=usb_link()
        wire = socket.socket()
        wire.settimeout(12)
        wire.setsockopt(socket.IPPROTO_IP, 31, struct.pack('!I', interface))
        wire.bind((source, 0))
        wire.connect(('192.168.7.2', 2222))
        ssh.connect('192.168.7.2', port=2222, username='root', sock=wire,
                    pkey=paramiko.ECDSAKey.from_private_key_file(str(key)),
                    timeout=12, allow_agent=False, look_for_keys=False)
        _, out, err = ssh.exec_command('pidof usb-video-relay >/dev/null || (nohup ' + shlex.quote(args.relay) + ' 192.168.7.2 >/tmp/usb-video-relay.log 2>&1 </dev/null &)', timeout=5)
        if out.channel.recv_exit_status():
            raise RuntimeError(err.read().decode())
        channel = socket.socket()
        channel.settimeout(10)
        channel.setsockopt(socket.IPPROTO_IP, 31, struct.pack('!I', interface))
        channel.bind((source, 0))
        channel.connect(('192.168.7.2', 15600))
        channel.settimeout(20)
        # Air enables UDP forwarding while a telemetry TCP client is connected.
        telemetry = ssh.get_transport().open_channel('direct-tcpip', ('127.0.0.1', 5760), ('127.0.0.1', 0), timeout=10)
        def drain_telemetry(telemetry=telemetry, video_channel=channel):
            try:
                while telemetry.recv(65536):
                    pass
            except (OSError, EOFError):
                pass
            finally:
                video_channel.close()
        threading.Thread(target=drain_telemetry, daemon=True).start()
        started = time.monotonic()
        total = packets = 0
        print('USB direct video connected: QOpenHD UDP5600; diagnostic UDP5602', flush=True)
        while True:
            size, = struct.unpack('!H', exact(channel, 2))
            packet = exact(channel, size)
            for port in (5600, 5602):
                udp.sendto(packet, ('127.0.0.1', port))
            packets += 1
            total += size
            elapsed = time.monotonic() - started
            if elapsed >= 5:
                print(f'{packets} packets, {total*8/elapsed/1e6:.2f} Mbit/s', flush=True)
                started = time.monotonic()
                total = packets = 0
    except (OSError, EOFError, RuntimeError, paramiko.SSHException, subprocess.SubprocessError) as exc:
        print(f'Waiting for USB video: {exc}', flush=True)
        time.sleep(2)
    finally:
        ssh.close()
