#!/usr/bin/env python3
"""Check that failed/stale firmware probes cannot authorize a capture."""
import base64
import importlib.machinery
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

loader = importlib.machinery.SourceFileLoader('scout', str(Path(__file__).with_name('openhd-nexmon-scout')))
spec = importlib.util.spec_from_loader(loader.name, loader)
scout = importlib.util.module_from_spec(spec)
loader.exec_module(scout)


class ProbeTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        root = Path(self.temp.name)
        (root/'manifest.json').write_text(json.dumps(dict(
            extended_probe_ioctl=512, extended_probe_frequencies_mhz=[5825, 5845])))
        state = root/'state.json'
        state.write_text(json.dumps(dict(bundle=str(root))))
        self.patch = patch.object(scout, 'STATE', state)
        self.patch.start()
        self.addCleanup(self.patch.stop)

    def probe(self, magic=0x4f484450, echoed=0xd0a9, frequency=5845,
              setter=0, getter=0, current=0xd0a9, iw='channel 169 (5845 MHz)'):
        raw = struct.pack('<IIiiiiI', magic, echoed, 0, frequency, setter, getter, current)
        with patch.object(scout, 'radio', return_value='wlan0'), \
                patch.object(scout, 'run', side_effect=[base64.b64encode(raw).decode(), iw]):
            return scout.probe(5845)

    def test_matching_readbacks(self):
        self.assertTrue(self.probe()['capture_ready'])

    def test_rejected_setter_with_stale_matching_readback(self):
        self.assertFalse(self.probe(setter=-20)['capture_ready'])

    def test_getter_failure(self):
        self.assertFalse(self.probe(getter=-23)['capture_ready'])

    def test_old_channel(self):
        self.assertFalse(self.probe(current=0xd0a5)['capture_ready'])

    def test_missing_phy_channel(self):
        self.assertFalse(self.probe(frequency=0)['capture_ready'])

    def test_kernel_disagrees(self):
        self.assertFalse(self.probe(iw='channel 165 (5825 MHz)')['capture_ready'])

    def test_unpatched_firmware(self):
        with self.assertRaises(RuntimeError):
            self.probe(magic=0)

    def test_wrong_request_reply(self):
        with self.assertRaises(RuntimeError):
            self.probe(echoed=0xd0a5)

    def test_outside_initial_scope(self):
        with self.assertRaises(RuntimeError):
            scout.probe(6005)


if __name__ == '__main__':
    unittest.main()
