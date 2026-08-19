# OpenHD X21 / Artosyn recovery snapshot (2026-08-19)

This directory is the persistent recovery snapshot for the working air and
ground setup. It deliberately contains the final executables, the original
vendor firmware archive, extracted deployment files, live-equivalent settings,
the KUTIAN SDK source and its test tools, and the packet-validation utility.
It does not depend on the Windows temporary directory.

## Working hardware and roles

- Air: X21 (AArch64), reachable by ADB as `6aec128f5e94bef1`.
- Ground: Raspberry Pi (ARMHF), SSH `openhd@192.168.1.171`.
- Air Artosyn firmware: `KT-2458-S-V1.3.3-U`, observed runtime version
  `1.3.02-28`.
- Ground Artosyn firmware: `KT-2458-G-V1.3.3-U`, observed runtime version
  `1.3.02-28`.
- Camera: HDZero/RunCam Nano 90 through `openhd_camera`, 720x540 at 90 fps.
  There is no ISP/RKAIQ stage for this camera.

## Final executable hashes

```text
fb3bb3ff15ca411a2a777313ca3c9cf67fcda7898f2bb47f10f4692044b6ec99  executables/openhd-air-arm64
22f88ceed9759db98726736c9aacec857a75035fefa0e84300fecb08478b5350  executables/openhd-ground-armhf
3deec4fe6faf5466d091f460298b68ed9fd0dddf2c6523306326760a1efeed84  firmware/V1.3.3-original.zip
```

The binaries are also present in the architecture-specific directories under
`bundle/`. `bundle/README.md` contains the associated runtime and firmware
installation steps.

## Live installation and reboot persistence

Air live files:

```text
/ohd/usr/bin/openhd
/usr/local/share/openhd/interface/artosyn_link.json
/usr/local/share/openhd/video/RV1126B_CSI_0.json
/ohd/etc/init.d/S99openhd
```

Air recovery copies are stored under `/ohd/storage/l4-v1.3.3/`, including
`openhd-air-single-reader`. The init script starts OpenHD after an X21 reboot.

Ground live binary:

```text
/usr/local/bin/openhd
```

The ground recovery binary is
`/home/openhd/l4-v1.3.3/openhd-ground-single-reader`. Both `openhd.service` and
`qopenhd.service` were enabled and active at handoff, so both return after a Pi
reboot.

## Required settings

Both Artosyn endpoints use SDK datagram framing and the P401 firmware's single
bidirectional host-data port:

```json
"video_port": 3,
"telemetry_port": 3,
"use_datagram": 1
```

The final camera profile uses 720x540 H.264 at 90 fps. ROI and MPP intra
refresh are disabled. Mode-2 intra refresh generated invalid zero-sized ROI
regions in the RV1126 MPP backend, and mode 1 caused visible corruption in this
setup.

The camera was put into its native Nano 90 mode with:

```sh
i2cset -f -y 5 0x23 0x12 0x00 0x00 0x08 0x80 0x08 0x81 0x1d i
```

Although the driver rejects `S_PARM` for 90 fps, measured V4L2 timestamps were
stable at approximately 89.99-90.00 fps.

## Root causes fixed

1. P401 exposes logical host-data port 3, so video and telemetry need OpenHD's
   `P401` stream multiplexing on the same SDK socket.
2. Artosyn telemetry transmission was incorrectly air-only. Ground parameter
   requests were silently discarded, and air did not start the corresponding
   receive path.
3. The shared receive path was started correctly once, but subsequent 500 ms
   stats ticks fell through and started a second reader on the same SDK socket.
   The readers split P401 records between them, corrupting RTP and MAVLink.
4. The video sender formerly queued/aggregated frames. It now sends one bounded
   SDK chunk synchronously from the encoder callback, with no temporal frame
   queue.
5. Running the encoder at 88 percent of estimated radio capacity filled the
   baseband queue. The Artosyn recommendation now keeps substantially more
   headroom and caps this setup near 5.8 Mbit/s.
6. Card status formerly treated five seconds without reverse application data
   as a disconnected radio. It now uses the vendor link state.

## Final validation

- A ground parameter request produced 7-15 received packets/s on air, proving
  the uplink works. It returns to zero while idle, which is expected.
- Clean ground capture: 300 packets, 300 valid RTP packets, 0 invalid packets,
  and 0 sequence gaps across 299 transitions.
- QOpenHD had exactly one GStreamer/HelloVideo pipeline bound to UDP 5600.
- Air and ground services were active after the coordinated deployment.
- Both final cross-builds completed successfully; `git diff --check` passed.

## SDK provenance

The preserved SDK source is from:

```text
https://github.com/KUTIAN-VT/L4_Linux_SDK.git
commit 3562e48cb8948fe404bda50b1d768a0824d4ab89
```

The SDK `.git` object database and compiler build caches are intentionally not
copied. All source, examples, headers, libraries, and test-tool inputs needed
for rebuilding are in `sdk-source/`.

## Recovery order

1. Power and link the two Artosyn modules.
2. Start or restart ground OpenHD first.
3. Start air OpenHD after the ground SDK socket is stable. Restarting ground
   underneath an open air SDK socket can leave the old air socket writing into
   a dead session.
4. Start QOpenHD last and verify only one `gst-launch-1.0` and one
   `fpv_video0.bin` process exist.
5. Use `capture_udp5600.py` with QOpenHD stopped for a clean 300-packet RTP
   sequence test.

