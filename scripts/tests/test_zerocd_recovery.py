import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('recovery', Path(__file__).parents[1] / 'openhd-zerocd-recovery.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class RecoveryTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.usb = self.root / 'usb'
        self.usb.mkdir()
        self.model = self.root / 'model'
        self.model.write_text('Raspberry Pi 4 Model B Rev 1.2\0')
        self.state = self.root / 'state.json'
        self.calls = []
        self.add('usb1', '1d6b', '0002')
        self.add('usb2', '1d6b', '0003')
        self.add('1-1', '2109', '3431')
        self.target = self.add('1-1.3', '0bda', '1a2b')

    def add(self, name, vendor, product):
        p = self.usb / name
        p.mkdir()
        for filename, value in [('idVendor', vendor), ('idProduct', product), ('devnum', '8')]:
            (p / filename).write_text(value)
        return p

    def tick(self, now):
        module.tick(self.usb, self.model, self.state, now, self.calls.append)

    def test_waits_for_normal_switch_then_recovers(self):
        for now in [0, 15, 89]:
            self.tick(now)
        self.assertEqual(self.calls, [])
        self.tick(90)
        self.assertEqual(self.calls, [self.target])

    def test_healthy_radio_never_resets(self):
        (self.target / 'idProduct').write_text('c812')
        self.tick(0)
        self.tick(1000)
        self.assertEqual(self.calls, [])

    def test_other_usb_device_prevents_ganged_reset(self):
        self.add('2-1', '1234', 'abcd')
        self.tick(0)
        self.tick(1000)
        self.assertEqual(self.calls, [])

    def test_other_pi_is_excluded(self):
        self.model.write_text('Raspberry Pi 5 Model B')
        self.tick(0)
        self.tick(1000)
        self.assertEqual(self.calls, [])

    def test_retry_budget_survives_reenumeration(self):
        for n, start in enumerate([0, 200, 400, 600]):
            (self.target / 'devnum').write_text(str(n + 8))
            self.tick(start)
            self.tick(start + 90)
        self.assertEqual(len(self.calls), 3)

    def test_power_on_and_services_restored_on_failure(self):
        commands = []
        def run(*args):
            commands.append(args)
            if args == ('uhubctl', '-l', '2', '-a', 'off'):
                raise RuntimeError('power control failed')
        with self.assertRaises(RuntimeError):
            module.recover(self.target, run, lambda _: None)
        self.assertIn(('uhubctl', '-l', '2', '-a', 'on'), commands)
        self.assertIn(('systemctl', 'start', 'openhd-sys-utils.service'), commands)
        self.assertEqual(commands[-1], ('systemctl', 'start', 'openhd.service'))


if __name__ == '__main__':
    unittest.main()
