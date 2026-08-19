# Artosyn/OpenHD v1.3.3 deployment bundle

Prepared 2026-08-19 from:

- OpenHD commit `f30df520b35aaa3c385afe0ae154dcfdb9277475`
- KUTIAN L4 Linux SDK commit `3562e48cb8948fe404bda50b1d768a0824d4ab89`
- firmware archive `V1.3.3(1).zip`

No device staging uses `/tmp`. Air data is staged under `/ohd/storage`; ground data is staged under `/home/openhd`.

## Correct module assignment

| Side | CPU | Firmware | Required role |
| --- | --- | --- | --- |
| X21 drone/air | ARM64 | `KT-2458-S-V1.3.3-U` | `DEV (1)` |
| Ground station | ARMHF | `KT-2458-G-V1.3.3-U` | `AP (0)` |

`U` is the USB data-path variant. Do not use `E` for this OpenHD arrangement. Do not use the `R` image unless KUTIAN identifies the attached hardware as an R variant.

The old live state was reversed (`air=AP`, `ground=DEV`). The v1.3.3 JSON configs and the SDK manual confirm that `S=DEV/air` and `G=AP/ground`.

## Tomorrow's sequence

Run from PowerShell in this directory after both boards are online:

```powershell
.\stage-tomorrow.ps1
```

Install the matching runtime while leaving OpenHD stopped:

```powershell
& 'C:\Users\Raphael\AppData\Local\Android\Sdk\platform-tools\adb.exe' shell "/ohd/storage/l4-v1.3.3/air-arm64/install-runtime.sh"
& 'C:\Program Files\PuTTY\plink.exe' -batch -ssh -pw openhd openhd@192.168.1.171 "sudo /home/openhd/l4-v1.3.3/ground-armhf/install-runtime.sh"
```

Flash one side at a time. Keep power and USB stable until `upgrade done` appears:

```powershell
& 'C:\Users\Raphael\AppData\Local\Android\Sdk\platform-tools\adb.exe' shell "/ohd/storage/l4-v1.3.3/air-arm64/flash-firmware.sh --confirm"
& 'C:\Program Files\PuTTY\plink.exe' -batch -ssh -pw openhd openhd@192.168.1.171 "sudo /home/openhd/l4-v1.3.3/ground-armhf/flash-firmware.sh --confirm"
```

After both module reboots, restart their daemons and verify versions/roles:

```sh
# Air
/ohd/etc/init.d/S97openhd-artosyn restart
sleep 8
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_basic_info
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_minidb_config -A

# Ground (sudo)
pkill -x artosyn_daemon || true
nohup /usr/local/bin/artosyn_daemon -i 0 -p 50000 >>/var/log/openhd-artosyn-daemon.log 2>&1 &
sleep 8
/usr/local/bin/l4_basic_info
/usr/local/bin/l4_minidb_config -A
```

Expected results: firmware `V1.3.3`, air `DEV (1)`, ground `AP (0)`.

## Pairing

Start the AP/ground command first, then the DEV/air command immediately. The new tool waits for the asynchronous result and has a 100-second default timeout.

```sh
# Ground/AP
sudo /usr/local/bin/l4_pair_manager -P -s 0

# Air/DEV
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_pair_manager -P
```

Then verify:

```sh
# Ground/AP
sudo /usr/local/bin/l4_pair_manager -m -s 0
sudo /usr/local/bin/l4_link_monitor -S

# Air/DEV
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_pair_manager -m
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_link_monitor -S
```

Do not continue until both sides show the reciprocal MAC and connected link state.

## Raw socket smoke test before OpenHD

This isolates the radio SDK from video and QOpenHD. Start the receiver on ground:

```sh
sudo /usr/local/bin/l4_socket_transfer -s 0 -P 3 --recv
```

Then send from air:

```sh
LD_LIBRARY_PATH=/ohd/usr/lib /ohd/usr/bin/l4_socket_transfer -s 0 -P 3 --text "openhd-artosyn-smoke"
```

The text must appear on ground. Repeat in the opposite direction if needed. If a stale socket prevents opening, use the SDK's forced-close recovery through `l4_tuntap -k`, or restart both daemons before retrying. Do not start OpenHD until this test passes.

## Start OpenHD and validate video

```sh
# Ground
sudo systemctl start openhd

# Air
/ohd/etc/init.d/S99openhd start
```

Firmware v1.3.3 exposes logical socket port 3 on this hardware. Confirm that OpenHD opens one shared stream socket (not datagram mode) on port 3; video and telemetry are multiplexed over it. Then check UDP 5600 reception and QOpenHD video. If raw socket transfer passes but OpenHD fails, the remaining fault is in the OpenHD adapter; if raw transfer fails, stay at the firmware/role/pairing layer.

## Key checksums

```text
fb3bb3ff15ca411a2a777313ca3c9cf67fcda7898f2bb47f10f4692044b6ec99  air-arm64/bin/openhd
02d3a01debaa38176aa420e451fcb03e780ffd4f9a0eff651c768524a76b3224  air-arm64/lib/libusb-1.0.so.0.3.0
9cb93c2dec2864843a3352e42182d3e495897ddae69286bdb1c6b2bc96bd8c3b  air-arm64/bin/l4_daemon
f944638019c49b90e289054b47bce1c06f2401fc5fc0552e78d239f94749f611  air-arm64/lib/libar8030_client.so
54f48b43519214d5c8cb1af2cc4380b033000e08e19ca630f5357ceb185831f2  air-arm64/firmware/VT4-KT-2458-S-V1.3.3-U.img
22f88ceed9759db98726736c9aacec857a75035fefa0e84300fecb08478b5350  ground-armhf/bin/openhd
9d86940253d3ab759da7c80c0677cc9042a98ca2cfcf1fa60f79514234f3f9eb  ground-armhf/bin/l4_daemon
9e916bca42bbf8725abd1a5ccbb2c4dd31f42acec090fe779a150e0dec77f177  ground-armhf/lib/libar8030_client.so
e32c45e4e87c35329e6561d16de6538470be93f0facb44770f1fbc90707ddd13  ground-armhf/firmware/VT4-KT-2458-G-V1.3.3-U.img
```

Backups are created before runtime replacement at `/ohd/storage/backups/artosyn-pre-v1.3.3` on air and `/home/openhd/openhd-backup-pre-v1.3.3` on ground.
