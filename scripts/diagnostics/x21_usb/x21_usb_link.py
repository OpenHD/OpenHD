"""Resolve the active X21 USB NCM adapter after Windows renumbers interfaces."""
import json
import os
import subprocess

def usb_link():
    override=os.environ.get('X21_USB_INTERFACE_INDEX','auto')
    if override != 'auto':
        index=int(override)
        select=f'Get-NetAdapter -InterfaceIndex {index}'
    else:
        select="Get-NetAdapter | Where-Object { $_.InterfaceDescription -like 'UsbNcm*' -and $_.Status -eq 'Up' }"
    command=select+" | ForEach-Object { $adapter=$_; Get-NetIPAddress -InterfaceIndex $adapter.ifIndex -AddressFamily IPv4 | Where-Object IPAddress -like '192.168.7.*' | ForEach-Object { [PSCustomObject]@{index=$adapter.ifIndex;source=$_.IPAddress} } } | ConvertTo-Json -Compress"
    raw=subprocess.check_output(['powershell','-NoProfile','-Command',command],text=True,timeout=12).strip()
    links=json.loads(raw) if raw else []
    if isinstance(links,dict):links=[links]
    if len(links)!=1:raise OSError(f'Expected one active X21 USB address, found {len(links)}')
    return int(links[0]['index']),links[0]['source']