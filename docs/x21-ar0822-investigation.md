# X21 AR0822/AP1302 integration and bench findings

The custom four-lane Arducam module supplies processed UYVY through its AP1302
ISP and MCU. OpenHD identifies the Pivariety subdevice rather than guessing an
IMX model. Camera type 152 uses the native CIF-to-MPP path and exposes the
detected primary camera as read-only to clients.

## Changes shipped

- Close inherited worker descriptors before `exec`, preserving DMA heaps opened
  by vendor-library constructors in the new process.
- Configure the Pivariety format chain and interval before capture; refresh CIF
  buffer layout after cropping and import its native vertical stride into MPP.
- Keep the X21 encoder awake during Pivariety capture and restore its previous
  runtime-power policy when the capture loop exits normally.
- Skip debug-noise buffer synchronization when noise injection is disabled.
- Read aggregate CPU usage from `/proc/stat` deltas instead of launching `top`.
- Bundle `librtlsdr` with the X21 runtime dependencies.

The companion `openhd_x21b_kernel` and `openhd_x21b_base` repositories contain
the dedicated `openhd,x21-ar0822-ap1302` profile, guarded probe-time MCU update,
embedded firmware and mode API. Generic Pivariety modules are not automatically
flashed. The embedded image selects manual exposure and target 600; it contains
neither the rejected startup experiment nor the higher-clock experiments.

## Hardware results, 2026-10-22

| Request | Measured result | Scope |
| --- | --- | --- |
| 1080p60 | About 60 fps | CIF counters and Windows RTP delivery |
| 720p120 | About 83.3 fps | MAX_FPS readback 120; measured CIF counter deltas |
| 1080p120 | Not enabled | Driver clamps 1080p requests to 60 |

The final 1080p check advanced 1825 frames in 30.42 seconds, with seven zero ISP
error/CSI error snapshots and no worker restart in that run. MPP reported 8120
DMA inputs, no copy fallback, no capture sequence gaps and no invalid frames.
A Windows 8-second RTP sample had no sequence gaps or invalid packets. Aggregate
CPU use was 15.61% over 10.72 seconds, including USB/debug activity; camera-worker
use was 34.7% of one core and main OpenHD use 9.42% of one core.

These are bounded bench measurements, not long-term or RF validation. Automatic
exposure and stable 120 fps output remain unresolved. MIPI target numbers are not
physical lane-clock measurements. Higher targets and sensor timing experiments
did not establish stable 120 fps end-to-end output.

## ISP failure and rejected experiment

The ISP console, read from the 512-byte basic-register buffer at 0x0a2c, reported:

```text
ERROR #001 @special/faced.c:236
Phase assigned: 0x3 done: 0x0 cnt: 1
```

During that state the picture continued near 60 fps while configuration writes
failed to read back. A clean boot cleared the error and restored writes. This
identifies the reported failure site; it does not establish a general meaning
for every numeric error 001 or prove the cause of every freeze.

Clearing context 2010 bit 4 at runtime left 720p output unchanged: 418 frames in
5.02 seconds both before and after, with ISP and CSI errors zero. Its interpretation
as a face-processing enable is inferred from older Aptina configuration data.
A separate MCU experiment that read this register midway through ISP boot left
the MCU busy despite passing loader emulation. It was rejected and the original
target 600 image was restored successfully, followed by a clean reboot.

## Saved firmware analysis

Two complete 131072-byte external EEPROM reads matched SHA256
`2e480794169fb2e44ce67ba2f767384ca5a14f1f0f2f7ed49bead9feb39b2653`.
The 84732-byte ISP payload begins at 0x2000. Its length matches the MCU loader's
ten 8192-byte blocks plus 0xafc bytes; payload SHA256 is
`3878d4e4359b915571c524c358468cba4075bd415b0b56954d6c864e3e69f5ff`.
The external EEPROM was never written.

`scripts/diagnostics/ap1302_decode_bootdata.py` is a read-only structural parser.
It decodes 16 records and 383 final register words in this payload and can compare
reference binaries. The generic onsemi AR0822 colour-reference table has 14
differing final words; FPS, exposure and FRC defaults agree. Executable blocks
remain compressed/opaque, and the trailer checksum is unresolved. The parser
does not produce a loadable or flashable firmware image. Private EEPROM dumps,
vendor files, credentials, captures and rejected firmware remain local.

## Validation boundaries

CPU accounting, worker descriptor inheritance, camera detection and MCU update
fault checks have host tests. OpenHD and the integrated kernel were cross-built
and exercised on the board. The deployed kernel9 was built from SDK kernel
commit 32e6c42 with these driver changes; the native kernel repository branch has
a different base, so deployment does not prove a full build of that branch.

For Windows debugging, the local relay carries framed TCP over ADB and recreates
forwards after reboot. This image's ADB sync upload stalled; shell transport and
temporary key-authenticated SSH over USB Ethernet were used for recovery.
