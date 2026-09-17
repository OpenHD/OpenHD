#!/usr/bin/env python3
"""Build an isolated Pi 4 extended-channel probe bundle on a Linux host."""
import argparse
import difflib
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tarfile

ROOT = Path(__file__).resolve().parents[1]
NEXMON = ROOT / 'OpenHD/ohd_interface/nexmon'
BASE = NEXMON / 'pi4-6.1.29-v7l'
FREQUENCIES = [5825, 5845, 5865, 5885, 5905, 5925]


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise RuntimeError('Pinned source anchor missing or ambiguous: ' + old)
    return text.replace(old, new, 1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--kernel-build', type=Path, required=True)
    parser.add_argument('--cross-compile', required=True,
                        help='Absolute GCC 13.3 ARM toolchain prefix')
    parser.add_argument('--firmware-toolchain', type=Path, required=True,
                        help='Nexmon GCC 5.4 toolchain directory')
    parser.add_argument('--modpost', type=Path,
                        help='Compatible host modpost executable or wrapper')
    args = parser.parse_args()
    output = args.output.resolve()
    if output.exists():
        parser.error('Output must be a new directory; existing builds are preserved')
    output.mkdir(parents=True)
    source = output / 'source'
    source.mkdir()
    with tarfile.open(BASE / 'nexmon-source.tar.gz') as archive:
        for entry in archive.getmembers():
            target = (source / entry.name).resolve()
            if not target.is_relative_to(source) or not (entry.isfile() or entry.isdir()):
                raise RuntimeError('Unexpected archive member: ' + entry.name)
        archive.extractall(source)
    # The original bundled archive omitted this upstream shared header.
    common = NEXMON / 'extended/structs.common.h'
    if hashlib.sha256(common.read_bytes()).hexdigest() != \
            '57aedee35fc717f02da0bf18a1fad9c4c5fe38682503b8f6907a2af4b9e9a134':
        raise RuntimeError('Pinned upstream shared-header checksum mismatch')
    shutil.copyfile(common, source / 'firmwares/bcm43455c0/structs.common.h')
    # This archive was made from a Windows checkout; shell shebangs need LF.
    for path in source.rglob('*'):
        if path.is_file() and (path.suffix in ('.c', '.h', '.mk', '.sh', '.awk') or
                               path.name == 'Makefile'):
            path.write_bytes(path.read_bytes().replace(b'\r\n', b'\n'))
    (source / 'DISABLE_STATISTICS').touch()
    driver = source / 'patches/driver/brcmfmac_6.1.y-nexmon'
    cfg = driver / 'cfg80211.c'
    original = cfg.read_text()
    modified = replace_once(original, 'CHAN5G(161), CHAN5G(165)',
                            'CHAN5G(161), CHAN5G(165),\n\tCHAN5G(169), CHAN5G(173), '
                            'CHAN5G(177), CHAN5G(181), CHAN5G(185)')
    # Keep firmware/regulatory disabled flags. Direct probe ioctls attempt
    # reception; adding an entry is not a claim of supported operation.
    cfg.write_text(modified)
    firmware = source / 'patches/bcm43455c0/7_45_206/nexmon'
    shutil.copyfile(NEXMON / 'extended/openhd_extended_probe.h',
                    firmware / 'src/openhd_extended_probe.h')
    ioctl = firmware / 'src/ioctl.c'
    original_ioctl = ioctl.read_text()
    modified_ioctl = replace_once(original_ioctl, '#define NULL 0',
                                  '#define NULL 0\n#include "openhd_extended_probe.h"')
    modified_ioctl = replace_once(modified_ioctl, '    switch(cmd) {',
                                  '    switch(cmd) {\n        case 512:\n'
                                  '            return openhd_extended_probe(wlc, arg, len, wlc_if);')
    ioctl.write_text(modified_ioctl)
    patches = ''.join(difflib.unified_diff(original.splitlines(True), modified.splitlines(True),
                     fromfile='a/patches/driver/brcmfmac_6.1.y-nexmon/cfg80211.c',
                     tofile='b/patches/driver/brcmfmac_6.1.y-nexmon/cfg80211.c'))
    patches += ''.join(difflib.unified_diff(original_ioctl.splitlines(True), modified_ioctl.splitlines(True),
                     fromfile='a/patches/bcm43455c0/7_45_206/nexmon/src/ioctl.c',
                     tofile='b/patches/bcm43455c0/7_45_206/nexmon/src/ioctl.c'))
    (output / 'openhd-extended.patch').write_text(patches)
    # setup_env.sh expects this toolchain at its traditional relative path.
    (source / 'buildtools/gcc-arm-none-eabi-5_4-2016q2-linux-x86').symlink_to(
        args.firmware_toolchain.resolve(), target_is_directory=True)
    compressor = output / 'compress_ucode.py'
    compressor.write_text('import sys, zlib\n'
                          'sys.stdout.buffer.write(zlib.compress(sys.stdin.buffer.read()))\n')
    env = dict(os.environ, NEXMON_ROOT=str(source), ARCH='arm',
               CROSS_COMPILE=args.cross_compile, NEXMON_SETUP_ENV='1', Q='@',
               CC=str(args.firmware_toolchain.resolve() / 'bin/arm-none-eabi-'),
               CCPLUGIN=str(source / 'buildtools/gcc-nexmon-plugin/nexmon.so'),
               ZLIBFLATE='python3 ' + shlex.quote(str(compressor)))
    subprocess.run(['make', '-C', str(source / 'buildtools/flash_patch_extractor')],
                   env=env, check=True)
    subprocess.run(['make', '-C', str(source / 'firmwares/bcm43455c0/7_45_206')],
                   env=env, check=True)
    subprocess.run(['make', '-C', str(firmware), 'brcmfmac43455-sdio.bin',
                    'GIT_VERSION=ohd-extended-v1'], env=env, check=True)
    command = ['make', '-C', str(args.kernel_build.resolve()), 'ARCH=arm',
               'CROSS_COMPILE=' + args.cross_compile, 'M=' + str(driver), '-j4']
    if args.modpost:
        command.append('MODPOST=' + str(args.modpost.resolve()))
    subprocess.run(command, env=env, check=True)
    bundle = output / 'bundle'
    bundle.mkdir()
    for name in ('nexutil', 'LICENSE.nexmon', 'kernel-Module.symvers.gz'):
        shutil.copy2(BASE / name, bundle / name)
    shutil.copy2(driver / 'brcmfmac.ko', bundle / 'brcmfmac.ko')
    shutil.copy2(firmware / 'brcmfmac43455-sdio.bin', bundle / 'firmware.bin')
    manifest = json.loads((BASE / 'manifest.json').read_text())
    manifest.update(firmware_build='ohd-extended-v1', extended_probe_ioctl=512,
                    extended_probe_frequencies_mhz=FREQUENCIES,
                    hardware_verified=False)
    (bundle / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    shutil.copy2(output / 'openhd-extended.patch', bundle / 'openhd-extended.patch')
    shutil.copy2(NEXMON / 'extended/README.md', bundle / 'README.md')
    # Include complete modified source, without binaries/build logs or the
    # externally provided compiler. Retain original per-file license notices.
    with tarfile.open(BASE / 'nexmon-source.tar.gz') as upstream, \
            tarfile.open(bundle / 'nexmon-source.tar.gz', 'w:gz') as archive:
        for entry in upstream.getmembers():
            archive.add(source / entry.name, arcname=entry.name, recursive=False)
        archive.add(firmware / 'src/openhd_extended_probe.h',
                    arcname='patches/bcm43455c0/7_45_206/nexmon/src/openhd_extended_probe.h')
        archive.add(source / 'firmwares/bcm43455c0/structs.common.h',
                    arcname='firmwares/bcm43455c0/structs.common.h')
    (bundle / 'SHA256SUMS').write_text(''.join(
        hashlib.sha256(p.read_bytes()).hexdigest() + '  ' + p.name + '\n'
        for p in sorted(bundle.iterdir()) if p.is_file() and p.name != 'SHA256SUMS'))
    for name in ('openhd-nexmon-scout', 'test_nexmon_extended.py'):
        shutil.copy2(ROOT / 'scripts' / name, output / name)
    shutil.copy2(NEXMON / 'extended/README.md', output / 'README.md')
    with tarfile.open(output / 'nexmon-extended-pi4-v1.tar.gz', 'w:gz') as archive:
        for name in ('bundle', 'openhd-nexmon-scout', 'test_nexmon_extended.py', 'README.md'):
            archive.add(output / name, arcname=name)
    print('Experimental bundle:', bundle)


if __name__ == '__main__':
    main()
