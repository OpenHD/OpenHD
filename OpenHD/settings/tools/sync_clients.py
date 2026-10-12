#!/usr/bin/env python3
"""Copy generated fallback/provisioning artifacts; runtime clients use MAVLink."""
import argparse
import pathlib
import generate

ROOT=pathlib.Path(__file__).resolve().parents[1]

def artifacts(args):
    result={}
    if args.qopenhd:
        base=pathlib.Path(args.qopenhd)/'app/telemetry/settings'
        result[base/'catalog.json']=ROOT/'generated/catalog.json'
        result[base/'metadata_ftp.h']=ROOT/'include/metadata_ftp.h'
    if args.imagewriter:
        result[pathlib.Path(args.imagewriter)/'doc/openhd_settings_map.json']=ROOT/'generated/imagewriter-settings.json'
    if args.glide:
        base=pathlib.Path(args.glide)/'src/common/settings'
        result[base/'metadata_ftp.h']=ROOT/'include/metadata_ftp.h'
        result[base/'json.hpp']=ROOT.parent/'ohd_common/lib/json/single_include/nlohmann/json.hpp'
    if args.webui:
        result[pathlib.Path(args.webui)/'src/openhdwebui.client/src/app/settings/settings-catalog.ts']=ROOT/'generated/webui-metadata.ts'
    if args.fleetcontrol:
        result[pathlib.Path(args.fleetcontrol)/'src/platforms.ts']=ROOT/'generated/platforms.ts'
        result[pathlib.Path(args.fleetcontrol)/'shared/platforms.json']=ROOT/'generated/platforms.json'
        result[pathlib.Path(args.fleetcontrol)/'shared/imagewriter-settings.json']=ROOT/'generated/imagewriter-settings.json'
    return result

def main():
    p=argparse.ArgumentParser()
    for name in ('qopenhd','imagewriter','glide','webui','fleetcontrol'): p.add_argument('--'+name)
    p.add_argument('--check',action='store_true')
    args=p.parse_args()
    for target,source in artifacts(args).items():
        data=source.read_bytes()
        if args.check:
            if not target.exists() or target.read_bytes()!=data: raise SystemExit(f'Out of date: {target}')
        else:
            target.parent.mkdir(parents=True,exist_ok=True)
            target.write_bytes(data)
        print(target)

if __name__=='__main__': main()
