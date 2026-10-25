"""Bundle the complete ARM runtime closure from the same verified cross SDK."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


def resolve(root, path):
    for _ in range(40):
        if not path.is_symlink():
            return path
        link = path.readlink()
        path = root / str(link).lstrip('/') if link.is_absolute() else path.parent / link
        path = Path(os.path.normpath(path))
        if not path.is_relative_to(root):
            raise ValueError('SDK symlink escapes sysroot')
    raise ValueError('SDK symlink loop')


def bundle(stage, sdk):
    pending = [stage / 'usr/bin' / n for n in ('openhd', 'openhd_sys_utils')]
    done = set()
    while pending:
        binary = pending.pop()
        header = binary.read_bytes()[:20]
        if header[:6] != b'\x7fELF\x01\x01' or header[18:20] != b'\x28\x00':
            raise ValueError('Expected ARM32 ELF: ' + str(binary))
        dynamic = subprocess.check_output(['readelf', '-d', str(binary)], text=True)
        for name in re.findall(r'\(NEEDED\).*?\[(.*?)\]', dynamic):
            if name in done:
                continue
            if '/' in name:
                raise ValueError('Invalid runtime SONAME')
            done.add(name)
            source = next((sdk / d / name for d in ('usr/lib', 'lib')
                           if (sdk / d / name).is_symlink() or (sdk / d / name).exists()), None)
            if source is None:
                raise ValueError('Missing SDK runtime library: ' + name)
            destination = stage / 'usr/lib' / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            if destination.is_symlink():
                destination.unlink()
            shutil.copyfile(resolve(sdk, source), destination)
            destination.chmod(0o755)
            pending.append(destination)


if __name__ == '__main__':
    bundle(Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve())
