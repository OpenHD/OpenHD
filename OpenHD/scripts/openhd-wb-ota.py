#!/usr/bin/env python3
"""Transfer a verified update bundle over the OpenHD WiFiBroadcast OTA stream."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import socket
import struct
import subprocess
import sys
import tempfile
import time
import zipfile

from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives.asymmetric.ed25519 import (
    Ed25519PrivateKey, Ed25519PublicKey,
)
from cryptography.hazmat.primitives.serialization import (
    load_pem_private_key, load_pem_public_key,
)

BRIDGE_PORT = 47600
CLIENT_PORT = 47601
CHUNK_SIZE = 896
WINDOW = 16
RETRY_INTERVAL = 0.35
MAX_SIZE = 8 * 1024**3
HEADER = struct.Struct("!4s32sBI")
MASK = struct.Struct("!H")
OFFER_SIZE = struct.Struct("!Q")
REQUEST, DATA, COMPLETE, RECEIPT, OFFER = range(1, 6)


def sha256(file):
    digest = hashlib.sha256()
    for block in iter(lambda: file.read(1024 * 1024), b""):
        digest.update(block)
    return digest.digest()


def send_packet(sock, digest, kind, number=0, body=b""):
    sock.send(HEADER.pack(b"OHD1", digest, kind, number) + body)


def receive_packet(sock, digest):
    try:
        packet = sock.recv(2048)
    except (socket.timeout, ConnectionResetError, ConnectionRefusedError):
        return None
    if not HEADER.size <= len(packet) <= HEADER.size + CHUNK_SIZE:
        return None
    magic, transfer, kind, number = HEADER.unpack_from(packet)
    if magic != b"OHD1" or transfer != digest:
        return None
    return kind, number, packet[HEADER.size:]


def chunk_count(size):
    return (size + CHUNK_SIZE - 1) // CHUNK_SIZE


def serve(sock, source, digest, size, timeout, rate, signature=None):
    count = chunk_count(size)
    progressed = -1
    offer_at = 0
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if signature and time.monotonic() >= offer_at:
            send_packet(sock, digest, OFFER, body=OFFER_SIZE.pack(size) + signature)
            offer_at = time.monotonic() + 1
        packet = receive_packet(sock, digest)
        if packet is None:
            continue
        kind, base, body = packet
        if kind == COMPLETE and base == count and not body:
            for _ in range(4):
                send_packet(sock, digest, RECEIPT, count)
                time.sleep(0.1)
            return
        if kind != REQUEST or len(body) != MASK.size or base >= count or base % WINDOW:
            continue
        missing, = MASK.unpack(body)
        if not 0 < missing < (1 << min(WINDOW, count - base)):
            continue
        if base > progressed:
            progressed = base
            deadline = time.monotonic() + timeout
        for index in range(min(WINDOW, count - base)):
            if missing & (1 << index):
                offset = (base + index) * CHUNK_SIZE
                source.seek(offset)
                expected = min(CHUNK_SIZE, size - offset)
                data = source.read(expected)
                if len(data) != expected:
                    raise ValueError("Source bundle changed during transfer")
                send_packet(sock, digest, DATA, base + index, data)
                time.sleep((HEADER.size + len(data)) * 8 / rate)
    raise TimeoutError("No OTA progress before timeout. Restart to resume.")


def receive_windows(sock, file, digest, size, timeout):
    # Commit one complete window at a time. Discard an interrupted tail.
    window_size = CHUNK_SIZE * WINDOW
    length = file.seek(0, os.SEEK_END)
    if length != size:
        length = min(length, size) // window_size * window_size
        file.truncate(length)
    file.seek(length)
    count = chunk_count(size)
    for base in range(chunk_count(length), count, WINDOW):
        width = min(WINDOW, count - base)
        chunks = [None] * width
        missing = (1 << width) - 1
        deadline = time.monotonic() + timeout
        request_at = 0
        while missing:
            now = time.monotonic()
            if now >= deadline:
                raise TimeoutError("No OTA progress before timeout. Restart to resume.")
            if now >= request_at:
                send_packet(sock, digest, REQUEST, base, MASK.pack(missing))
                request_at = now + RETRY_INTERVAL
            packet = receive_packet(sock, digest)
            if packet is None:
                continue
            kind, number, data = packet
            index = number - base
            if kind != DATA or not 0 <= index < width or chunks[index] is not None:
                continue
            if len(data) != min(CHUNK_SIZE, size - number * CHUNK_SIZE):
                continue
            chunks[index] = data
            missing &= ~(1 << index)
            request_at = now + RETRY_INTERVAL
            deadline = time.monotonic() + timeout
        file.write(b"".join(chunks))
        file.flush()
        os.fsync(file.fileno())
        if base % (WINDOW * 64) == 0 or base + width == count:
            print(f"Received {file.tell():,} / {size:,} bytes", flush=True)


def confirm(sock, digest, count):
    deadline = time.monotonic() + 6
    while time.monotonic() < deadline:
        send_packet(sock, digest, COMPLETE, count)
        request_at = time.monotonic() + RETRY_INTERVAL
        while time.monotonic() < request_at:
            if receive_packet(sock, digest) == (RECEIPT, count, b""):
                return
    print("Bundle verified. Ground did not acknowledge completion.", file=sys.stderr)


def receive(sock, output, digest, size, timeout):
    partial = output.with_name(f".{output.name}.{digest.hex()}.part")
    if output.exists():
        with output.open("rb") as file:
            if os.fstat(file.fileno()).st_size != size or sha256(file) != digest:
                raise FileExistsError(f"Destination contains a different file: {output}")
            with zipfile.ZipFile(file) as archive:
                validate_bundle(archive)
    else:
        fd = os.open(partial, os.O_RDWR | os.O_CREAT | getattr(os, "O_NOFOLLOW", 0), 0o600)
        with os.fdopen(fd, "r+b") as file:
            receive_windows(sock, file, digest, size, timeout)
            file.seek(0)
            verified = sha256(file) == digest
            if verified:
                with zipfile.ZipFile(file) as archive:
                    validate_bundle(archive)
        if not verified:
            partial.unlink()
            raise ValueError("SHA-256 mismatch. Discarded the unverified bundle.")
        # Publish atomically without overwriting an existing destination.
        os.link(partial, output)
        partial.unlink()
        if os.name == "posix":
            directory = os.open(output.parent, os.O_RDONLY | os.O_DIRECTORY)
            try:
                os.fsync(directory)
            finally:
                os.close(directory)
    confirm(sock, digest, chunk_count(size))


def platform_identity():
    architecture = subprocess.check_output(["dpkg", "--print-architecture"], text=True).strip()
    for path in ("/proc/device-tree/model", "/sys/firmware/devicetree/base/model",
                 "/sys/class/dmi/id/product_name"):
        try:
            model = Path(path).read_bytes().rstrip(b"\0\n ").decode("utf-8")
        except OSError:
            continue
        if model:
            return {"architecture": architecture, "model": model}
    raise ValueError("Could not identify the local hardware model")


def component(name):
    if name in {"openhd", "openhd-arm64", "openhd-armhf", "openhd-amd64", "openhd-x20"}:
        return "openhd"
    if name in {"qopenhd", "qopenhd-native-rk3566", "qopenhd-native-rk3588"}:
        return "qopenhd"
    if name in {"openhd-sys-utils", "open-hd-web-ui"}:
        return name
    raise ValueError(f"Unsupported OTA package: {name}")


def package_info(path, target_arch):
    info = {}
    for field in ("Package", "Architecture", "Version"):
        info[field] = subprocess.check_output(
            ["dpkg-deb", "--field", str(path.resolve()), field], text=True
        ).strip()
    if info["Architecture"] not in {target_arch, "all"}:
        raise ValueError(f"{info['Package']} has incompatible architecture {info['Architecture']}")
    return component(info["Package"]), info


def pack(destination, packages, architecture, model):
    metadata = []
    components = set()
    for path in packages:
        name, info = package_info(path, architecture)
        if name in components:
            raise ValueError(f"Duplicate component: {name}")
        components.add(name)
        metadata.append(info)
    with destination.open("xb") as file:
        try:
            with zipfile.ZipFile(file, "w") as archive:
                manifest = {"version": 1, "target": {"architecture": architecture,
                            "model": model}, "packages": metadata}
                archive.writestr("manifest.json", json.dumps(manifest))
                for index, path in enumerate(packages):
                    archive.write(path, f"{index}.deb")
            file.flush()
            os.fsync(file.fileno())
        except BaseException:
            file.close()
            destination.unlink()
            raise
    with destination.open("rb") as file:
        print(f"SHA-256 {sha256(file).hex()}   Size {file.tell()} bytes")


def validate_bundle(archive):
    if archive.getinfo("manifest.json").file_size > 8192:
        raise ValueError("OTA manifest too large")
    manifest = json.loads(archive.read("manifest.json"))
    if not isinstance(manifest, dict) or manifest.get("version") != 1:
        raise ValueError("Unsupported OTA manifest")
    local = platform_identity()
    if manifest.get("target") != local:
        raise ValueError(f"OTA target {manifest.get('target')} does not match {local}")
    packages = manifest.get("packages")
    if not isinstance(packages, list) or not 1 <= len(packages) <= 4:
        raise ValueError("Expected one to four OTA packages")
    members = [f"{index}.deb" for index in range(len(packages))]
    if sorted(archive.namelist()) != sorted(["manifest.json", *members]):
        raise ValueError("Unexpected OTA bundle entries")
    components = set()
    total_size = 0
    for member, info in zip(members, packages):
        if not isinstance(info, dict) or not all(
            isinstance(info.get(field), str) and info[field]
            for field in ("Package", "Architecture", "Version")
        ):
            raise ValueError("Invalid package metadata")
        name = component(info["Package"])
        if name in components or info["Architecture"] not in {local["architecture"], "all"}:
            raise ValueError(f"Duplicate or incompatible package: {info['Package']}")
        components.add(name)
        total_size += archive.getinfo(member).file_size
    if total_size > MAX_SIZE:
        raise ValueError("Unpacked bundle exceeds size limit")
    return local["architecture"], members, packages


def install(bundle, digest):
    with bundle.open("rb") as input_file, tempfile.TemporaryDirectory(prefix="openhd-ota-") as tmp:
        if sha256(input_file) != digest:
            raise ValueError("Bundle SHA-256 does not match the trusted digest")
        with zipfile.ZipFile(input_file) as archive:
            architecture, members, expected = validate_bundle(archive)
            packages = []
            for member, info in zip(members, expected):
                path = Path(tmp, member)
                with archive.open(member) as source, path.open("xb") as destination:
                    shutil.copyfileobj(source, destination)
                if package_info(path, architecture)[1] != info:
                    raise ValueError(f"Package metadata mismatch: {info['Package']}")
                installed = subprocess.run(
                    ["dpkg-query", "-W", "-f=${db:Status-Status} ${Version}", info["Package"]],
                    text=True, capture_output=True, check=False,
                )
                if installed.returncode == 0 and installed.stdout.startswith("installed "):
                    version = installed.stdout.split(maxsplit=1)[1]
                    if subprocess.run(["dpkg", "--compare-versions", info["Version"], "lt", version],
                                      check=False).returncode == 0:
                        raise ValueError(f"Refusing to downgrade {info['Package']} from {version}")
                # APT's sandbox user must be able to read the archive.
                path.chmod(0o644)
                packages.append(str(path))
        Path(tmp).chmod(0o755)
        if shutil.which("apt-get"):
            subprocess.run(["apt-get", "-y", "--no-download", "--no-remove",
                            "--no-install-recommends", "install", *packages], check=True)
        else:
            subprocess.run(["dpkg", "--install", *packages], check=True)



def signature_input(digest, size):
    return b"OpenHD WiFiBroadcast package bundle v1\0" + digest + OFFER_SIZE.pack(size)


def sign(bundle, private_key, destination):
    key = load_pem_private_key(private_key.read_bytes(), password=None)
    if not isinstance(key, Ed25519PrivateKey):
        raise ValueError("OTA requires an Ed25519 signing key")
    with bundle.open("rb") as file:
        size = os.fstat(file.fileno()).st_size
        signature = key.sign(signature_input(sha256(file), size))
    with destination.open("xb") as file:
        file.write(signature)


def verify_offer(public_key, digest, size, signature):
    key = load_pem_public_key(public_key.read_bytes())
    if not isinstance(key, Ed25519PublicKey):
        return False
    try:
        key.verify(signature, signature_input(digest, size))
    except InvalidSignature:
        return False
    return True


def listen(sock, directory, public_key, timeout, apply):
    next_offer = 0
    while True:
        try:
            packet = sock.recv(2048)
        except (socket.timeout, ConnectionResetError, ConnectionRefusedError):
            continue
        if len(packet) != HEADER.size + OFFER_SIZE.size + 64:
            continue
        magic, digest, kind, number = HEADER.unpack_from(packet)
        size, = OFFER_SIZE.unpack_from(packet, HEADER.size)
        if magic != b"OHD1" or kind != OFFER or number or not 0 < size <= MAX_SIZE:
            continue
        now = time.monotonic()
        if now < next_offer:
            continue
        next_offer = now + 0.5
        signature = packet[HEADER.size + OFFER_SIZE.size:]
        if not verify_offer(public_key, digest, size, signature):
            continue
        bundle = directory / f"{digest.hex()}.zip"
        marker = directory / f"{digest.hex()}.installed"
        try:
            receive(sock, bundle, digest, size, timeout)
            print(f"Verified bundle staged at {bundle}", flush=True)
            if apply and not marker.exists():
                install(bundle, digest)
                marker.touch()
        except (OSError, ValueError, KeyError, zipfile.BadZipFile, subprocess.CalledProcessError) as error:
            print(f"OTA failed for {digest.hex()}: {error}", file=sys.stderr, flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--idle-timeout", type=float, default=60)
    modes = parser.add_subparsers(dest="mode", required=True)
    sender = modes.add_parser("send")
    sender.add_argument("bundle", type=Path)
    sender.add_argument("--signature", type=Path)
    sender.add_argument("--rate-kbps", type=int, choices=range(64, 501), default=160,
                        metavar="64..500")
    receiver = modes.add_parser("receive")
    receiver.add_argument("destination", type=Path)
    receiver.add_argument("--sha256", required=True)
    receiver.add_argument("--size", required=True, type=int)
    modes.add_parser("identify")
    signer = modes.add_parser("sign")
    signer.add_argument("bundle", type=Path)
    signer.add_argument("private_key", type=Path)
    signer.add_argument("signature", type=Path)
    listener = modes.add_parser("listen")
    listener.add_argument("directory", type=Path)
    listener.add_argument("--public-key", required=True, type=Path)
    listener.add_argument("--install", action="store_true")
    builder = modes.add_parser("pack")
    builder.add_argument("destination", type=Path)
    builder.add_argument("packages", nargs="+", type=Path)
    builder.add_argument("--architecture", required=True)
    builder.add_argument("--model", required=True)
    installer = modes.add_parser("install")
    installer.add_argument("bundle", type=Path)
    installer.add_argument("--sha256", required=True)
    args = parser.parse_args()
    if not 1 <= args.idle_timeout <= 600:
        parser.error("idle timeout must be between 1 and 600 seconds")
    try:
        if args.mode == "identify":
            print(json.dumps(platform_identity()))
            return
        if args.mode == "sign":
            sign(args.bundle, args.private_key, args.signature)
            return
        if args.mode == "pack":
            pack(args.destination, args.packages, args.architecture, args.model)
            return
        if args.mode == "install":
            digest = bytes.fromhex(args.sha256)
            if len(digest) != 32:
                parser.error("provide a SHA-256 digest")
            install(args.bundle, digest)
            return
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.bind(("127.0.0.1", CLIENT_PORT))
            sock.connect(("127.0.0.1", BRIDGE_PORT))
            sock.settimeout(0.1)
            if args.mode == "listen":
                args.directory.mkdir(mode=0o700, parents=True, exist_ok=True)
                listen(sock, args.directory, args.public_key, args.idle_timeout, args.install)
            elif args.mode == "send":
                with args.bundle.open("rb") as file:
                    size = os.fstat(file.fileno()).st_size
                    if not 0 < size <= MAX_SIZE:
                        parser.error("bundle size must be between 1 byte and 8 GiB")
                    digest = sha256(file)
                    print(f"SHA-256 {digest.hex()}   Size {size} bytes", flush=True)
                    signature = args.signature.read_bytes() if args.signature else None
                    if signature is not None and len(signature) != 64:
                        raise ValueError("Invalid Ed25519 signature length")
                    serve(sock, file, digest, size, args.idle_timeout, args.rate_kbps * 1000, signature)
                print("Air verified the bundle.")
            else:
                digest = bytes.fromhex(args.sha256)
                if len(digest) != 32 or not 0 < args.size <= MAX_SIZE:
                    parser.error("provide a SHA-256 digest and a size between 1 byte and 8 GiB")
                receive(sock, args.destination, digest, args.size, args.idle_timeout)
                print(f"Verified bundle staged at {args.destination}")
    except (OSError, ValueError, KeyError, zipfile.BadZipFile, subprocess.CalledProcessError) as error:
        parser.exit(1, f"OTA transfer failed: {error}\n")


if __name__ == "__main__":
    main()
