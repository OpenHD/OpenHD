"""Maintain QOpenHD's localhost TCP5760 connection over the X21 USB link."""
import argparse
import os
from pathlib import Path
import subprocess
import time
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--identity',required=True,type=Path)
parser.add_argument('--known-hosts',type=Path,default=Path.home()/'.ssh/known_hosts')
options=parser.parse_args()
args=['ssh','-N','-T','-b','192.168.7.18','-p','2222','-i',str(options.identity),
      '-o','BatchMode=yes','-o','StrictHostKeyChecking=yes',
      '-o','UserKnownHostsFile='+str(options.known_hosts),
      '-o','ExitOnForwardFailure=yes','-o','ServerAliveInterval=5','-o','ServerAliveCountMax=2',
      '-L','127.0.0.1:5760:127.0.0.1:5760','root@192.168.7.2']
while True:
    print('Connecting QOpenHD telemetry over USB',flush=True)
    from x21_usb_link import usb_link
    try:
        interface,source=usb_link()
        args[args.index('-b')+1]=source
        result=subprocess.run(args)
    except (subprocess.SubprocessError,OSError,ValueError) as exc:
        print(f'USB address unavailable: {exc}',flush=True)
        time.sleep(2)
        continue
    print(f'Telemetry tunnel exited {result.returncode}; reconnecting',flush=True)
    time.sleep(2)
