import base64
from pathlib import Path
import runpy
import unittest

setup = runpy.run_path(str(Path(__file__).with_name('openhd-fleet-setup')))
validate = setup['validate_profile']
key = base64.b64encode(bytes(range(32))).decode()
PROFILE = f'''# OpenHD-Profile-Version=1
# OpenHD-Device-ID=craft-test
# OpenHD-Role=air
# OpenHD-Interface=openhd-lte
# OpenHD-FleetControl-Address=10.77.10.1
# OpenHD-Video-Port=5600
# OpenHD-Video2-Port=5601
# OpenHD-Telemetry-Port=14550
[Interface]
PrivateKey = {key}
Address = 10.77.10.2/24
[Peer]
PublicKey = {key}
AllowedIPs = 10.77.10.0/24
Endpoint = example.com:51820
PersistentKeepalive = 25
'''


class ProfileTest(unittest.TestCase):
    def test_air_and_ground_profiles(self):
        self.assertEqual(validate(PROFILE, 'air')['Device-ID'], 'craft-test')
        self.assertEqual(validate(PROFILE.replace('Role=air', 'Role=ground'), 'ground')['Role'], 'ground')
        self.assertEqual(validate(PROFILE.replace('\n', '\r\n'), 'air')['Role'], 'air')

    def test_rejects_wrong_device_and_cross_account_routes(self):
        with self.assertRaises(ValueError):
            validate(PROFILE, 'ground')
        for text in (PROFILE.replace('10.77.10.0/24', '0.0.0.0/0'),
                     PROFILE.replace('10.77.10.2/24', '10.77.11.2/24')):
            with self.assertRaises(ValueError):
                validate(text, 'air')

    def test_rejects_hooks_duplicate_keys_and_malformed_ports(self):
        for text in (PROFILE + 'PostUp = touch /tmp/never\n',
                     PROFILE + f'PublicKey = {key}\n',
                     PROFILE + '# OpenHD-Device-ID=other\n',
                     PROFILE.replace('Video-Port=5600', 'Video-Port=99999'),
                     PROFILE + '[Peer]\n', PROFILE + 'A' * 32768):
            with self.assertRaises(ValueError):
                validate(text, 'air')


if __name__ == '__main__':
    unittest.main()
