import hashlib
import importlib.machinery
import importlib.util
from pathlib import Path
import random
import select
import socket
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock
import zipfile

from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat

script = Path(__file__).resolve().parents[1] / "scripts" / "openhd-wb-ota.py"
loader = importlib.machinery.SourceFileLoader("ota", str(script))
spec = importlib.util.spec_from_loader(loader.name, loader)
ota = importlib.util.module_from_spec(spec)
loader.exec_module(ota)


def local_socket():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("127.0.0.1", 0))
    sock.settimeout(0.05)
    return sock


class OtaTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.dir = Path(self.tmp.name)
        tree = self.dir / "deb"
        (tree / "DEBIAN").mkdir(parents=True)
        self.target = {"model": "OTA test board", "architecture":
                       subprocess.check_output(["dpkg", "--print-architecture"], text=True).strip()}
        identity = mock.patch.object(ota, "platform_identity", return_value=self.target)
        identity.start()
        self.addCleanup(identity.stop)
        (tree / "DEBIAN" / "control").write_text(
            "Package: openhd\nVersion: 1.0\nArchitecture: all\n"
            "Maintainer: OpenHD <dev@example.com>\nDescription: OTA test fixture\n"
        )
        (tree / "fixture").write_bytes(random.Random(4).randbytes(60000))
        self.deb = self.dir / "source.deb"
        subprocess.run(["dpkg-deb", "--build", str(tree), str(self.deb)],
                       check=True, stdout=subprocess.DEVNULL)

    def test_resume_through_loss_duplicate_and_reordering(self):
        source = self.dir / "update.zip"
        ota.pack(source, [self.deb], **self.target)
        content = source.read_bytes()
        digest = hashlib.sha256(content).digest()
        destination = self.dir / "verified.zip"
        partial = self.dir / f".{destination.name}.{digest.hex()}.part"
        prefix = ota.WINDOW * ota.CHUNK_SIZE
        with local_socket() as silent_peer, local_socket() as interrupted:
            interrupted.connect(silent_peer.getsockname())
            with partial.open("w+b") as file:
                file.write(content[:prefix] + b"interrupted-window")
                with self.assertRaises(TimeoutError):
                    ota.receive_windows(interrupted, file, digest, len(content), 0.2)
        self.assertEqual(partial.read_bytes(), content[:prefix])

        sockets = [local_socket() for _ in range(4)]
        for sock in sockets:
            self.addCleanup(sock.close)
        ground, air, ground_relay, air_relay = sockets
        ground.connect(ground_relay.getsockname())
        air.connect(air_relay.getsockname())
        randomizer = random.Random(28)
        stop = threading.Event()
        requests = []

        def relay():
            scheduled = []
            while not stop.is_set():
                ready, _, _ = select.select([ground_relay, air_relay], [], [], 0.006)
                for incoming in ready:
                    packet, _ = incoming.recvfrom(2048)
                    _, _, kind, number = ota.HEADER.unpack_from(packet)
                    if kind == ota.REQUEST:
                        requests.append(number)
                    outgoing, peer = (air_relay, air) if incoming is ground_relay else (ground_relay, ground)
                    if randomizer.random() < (0.4 if kind == ota.DATA else 0.25):
                        continue
                    now = time.monotonic()
                    scheduled.append((now + randomizer.uniform(0, 0.035), outgoing, peer, packet))
                    if randomizer.random() < 0.25:
                        scheduled.append((now + 0.037, outgoing, peer, packet))
                now = time.monotonic()
                for when, outgoing, peer, packet in sorted(scheduled, key=lambda entry: entry[0]):
                    if when <= now:
                        outgoing.sendto(packet, peer.getsockname())
                scheduled = [entry for entry in scheduled if entry[0] > now]

        errors = []

        def sender():
            try:
                with source.open("rb") as file:
                    ota.serve(ground, file, digest, len(content), 8, 400000)
            except Exception as error:
                errors.append(error)

        bridge = threading.Thread(target=relay)
        server = threading.Thread(target=sender)
        bridge.start()
        server.start()
        try:
            ota.receive(air, destination, digest, len(content), 8)
            server.join(10)
            self.assertFalse(server.is_alive())
        finally:
            stop.set()
            bridge.join()
            server.join()
        self.assertEqual(errors, [])
        self.assertEqual(min(requests), ota.WINDOW)
        self.assertEqual(destination.read_bytes(), content)
        self.assertFalse(partial.exists())

    def test_rejects_wrong_model_and_architecture(self):
        for key, wrong in (("model", "Different board"), ("architecture", "foreign")):
            with self.subTest(key=key):
                target = self.target | {key: wrong}
                bundle = self.dir / f"{key}.zip"
                ota.pack(bundle, [self.deb], **target)
                with zipfile.ZipFile(bundle) as archive, self.assertRaisesRegex(
                        ValueError, "does not match"):
                    ota.validate_bundle(archive)

    def test_install_passes_all_packages_to_native_backend(self):
        bundle = self.dir / "test.zip"
        ota.pack(bundle, [self.deb], **self.target)
        digest = hashlib.sha256(bundle.read_bytes()).digest()
        original_run = subprocess.run
        for executable in ("apt-get", "dpkg"):
            with self.subTest(executable=executable):
                commands = []

                def record(command, *args, **kwargs):
                    if command[0] == "dpkg-query":
                        return subprocess.CompletedProcess(command, 1, "", "")
                    if command[0] == executable:
                        self.assertTrue(Path(command[-1]).is_file())
                        commands.append(command)
                        return subprocess.CompletedProcess(command, 0)
                    return original_run(command, *args, **kwargs)

                with mock.patch.object(ota.subprocess, "run", side_effect=record), mock.patch.object(
                        ota.shutil, "which", return_value=executable if executable == "apt-get" else None):
                    ota.install(bundle, digest)
                self.assertEqual(len(commands), 1)
                if executable == "apt-get":
                    self.assertIn("--no-download", commands[0])
                else:
                    self.assertIn("--install", commands[0])

    def test_signed_offer_authenticates_digest_and_size(self):
        bundle = self.dir / "test.zip"
        key = self.dir / "key.pem"
        public = self.dir / "public.pem"
        proof = self.dir / "test.sig"
        ota.pack(bundle, [self.deb], **self.target)
        subprocess.run(["openssl", "genpkey", "-algorithm", "ED25519", "-out", str(key)], check=True)
        subprocess.run(["openssl", "pkey", "-in", str(key), "-pubout", "-out", str(public)], check=True)
        ota.sign(bundle, key, proof)
        data = bundle.read_bytes()
        digest = hashlib.sha256(data).digest()
        self.assertTrue(ota.verify_offer(public, digest, len(data), proof.read_bytes()))
        self.assertFalse(ota.verify_offer(public, digest, len(data) + 1, proof.read_bytes()))
        self.assertFalse(ota.verify_offer(public, bytes(32), len(data), proof.read_bytes()))

    def test_downgrade_refused_before_native_install(self):
        bundle = self.dir / "test.zip"
        ota.pack(bundle, [self.deb], **self.target)
        digest = hashlib.sha256(bundle.read_bytes()).digest()
        original_run = subprocess.run

        def record(command, *args, **kwargs):
            if command[0] == "dpkg-query":
                return subprocess.CompletedProcess(command, 0, "installed 2.0", "")
            self.assertNotIn("install", command)
            self.assertNotIn("--install", command)
            return original_run(command, *args, **kwargs)

        with mock.patch.object(ota.subprocess, "run", side_effect=record):
            with self.assertRaisesRegex(ValueError, "Refusing to downgrade"):
                ota.install(bundle, digest)

    def test_bad_digest_is_never_installed(self):
        bundle = self.dir / "test.zip"
        ota.pack(bundle, [self.deb], **self.target)
        with self.assertRaisesRegex(ValueError, "trusted digest"):
            ota.install(bundle, bytes(32))


    def test_listener_install_once_for_signed_offer(self):
        key = ota.Ed25519PrivateKey.generate()
        public = self.dir / "trusted.pem"
        public.write_bytes(key.public_key().public_bytes(Encoding.PEM,
                                                         PublicFormat.SubjectPublicKeyInfo))
        digest = hashlib.sha256(b"bundle").digest()
        size = 6
        header = ota.HEADER.pack(b"OHD1", digest, ota.OFFER, 0) + ota.OFFER_SIZE.pack(size)
        signed = header + key.sign(ota.signature_input(digest, size))
        sock = mock.Mock()
        sock.recv.side_effect = [b"short", header + bytes(64), signed, signed, StopIteration]
        with mock.patch.object(ota.time, "monotonic", side_effect=[1, 2, 3]), \
             mock.patch.object(ota, "receive") as receive, \
             mock.patch.object(ota, "install") as install:
            with self.assertRaises(StopIteration):
                ota.listen(sock, self.dir, public, 60, True)
        bundle = self.dir / (digest.hex() + ".zip")
        install.assert_called_once_with(bundle, digest)
        self.assertEqual(receive.call_count, 2)
        self.assertTrue((self.dir / (digest.hex() + ".installed")).is_file())

    def test_cli_dispatches_without_installing_on_test_host(self):
        bundle = self.dir / "test.zip"
        digest = hashlib.sha256(b"unsigned test input").digest()
        bundle.write_bytes(b"unsigned test input")
        proof = self.dir / "test.sig"
        proof.write_bytes(bytes(64))
        for mode, args, function, expected in (
            ("identify", [], "platform_identity", ()),
            ("pack", [str(bundle), str(self.deb), "--architecture", "amd64",
                      "--model", "fixture"], "pack", (bundle, [self.deb], "amd64", "fixture")),
            ("sign", [str(bundle), "private.pem", str(proof)], "sign",
             (bundle, Path("private.pem"), proof)),
            ("install", [str(bundle), "--sha256", digest.hex()], "install", (bundle, digest)),
            ("receive", [str(bundle), "--sha256", digest.hex(), "--size", "19"], "receive",
             (mock.ANY, bundle, digest, 19, 60)),
            ("listen", [str(self.dir), "--public-key", "public.pem", "--install"], "listen",
             (mock.ANY, self.dir, Path("public.pem"), 60, True)),
            ("send", [str(bundle), "--signature", str(proof)], "serve",
             (mock.ANY, mock.ANY, digest, 19, 60, 160000, proof.read_bytes())),
        ):
            with self.subTest(mode=mode), mock.patch.object(sys, "argv", ["openhd-wb-ota", mode, *args]), \
                 mock.patch.object(ota.socket, "socket"), \
                 mock.patch.object(ota, function, return_value=self.target) as action:
                ota.main()
                action.assert_called_once_with(*expected)


if __name__ == "__main__":
    unittest.main()
