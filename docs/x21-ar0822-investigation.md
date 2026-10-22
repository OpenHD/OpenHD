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
| 720p120, original mode | About 83.3 fps | MAX_FPS readback 120; measured CIF counter deltas |
| 720p120, staged linear timing | About 119.9 fps in a successful bench run | CIF counter deltas, Windows RTP and complete H.264 decoding; repeatability remains under test |
| 1080p100 | Experimental only | Short capture near 100 fps; no accepted sustained encoder/output test |
| 1080p120 | Not enabled | Driver still clamps 1080p requests to 60 |

The final 1080p check advanced 1825 frames in 30.42 seconds, with seven zero ISP
error/CSI error snapshots and no worker restart in that run. MPP reported 8120
DMA inputs, no copy fallback, no capture sequence gaps and no invalid frames.
A Windows 8-second RTP sample had no sequence gaps or invalid packets. Aggregate
CPU use was 15.61% over 10.72 seconds, including USB/debug activity; camera-worker
use was 34.7% of one core and main OpenHD use 9.42% of one core.

These CPU measurements apply to the 60 fps run. These are bounded bench
measurements, not long-term or RF validation. Automatic exposure remains
unresolved. MIPI target numbers are not physical lane-clock measurements.

## Staged 720p120 timing experiment

The original 83 fps ceiling can be exceeded without increasing the MCU's target
600. The successful sequence selects linear RAW12 sensor readout, acquires it at
frame length 1152 / line length 1216, then changes blanking to frame length 1164 /
line length 1180. AP1302 continues to supply UYVY over four lanes. It retains HINF
spoofing and the full binned field of view; no digital zoom or PLL write is used.
Exposure is fixed at 4 ms and the flicker/FRC bits are cleared with unrelated
bits preserved. The direct-sensor [Kurokesu driver](https://github.com/Kurokesu/ar0822-rpi-driver)
provides register references, but its mode table cannot replace the module's ISP
firmware or establish the measured output rate.

One ten-minute userspace-profile run advanced 72640 CIF frames over 608.53 seconds,
including initial transients. Steady Windows delivery measured 119.9 fps, with
1200 RTP frame timestamps, no sequence gaps and no invalid packets. A trimmed
capture decoded 1120 complete 1280x720 frames with no corrupt frames. ISP error
and sampled receiver error counters were zero; sticky ISP warning 0x0302 remained.
Other attempts stalled, so this successful run does not establish repeatable
startup or reliable mode switching.

The companion kernel adds a guarded, volatile timing transaction with identity
checks, bounded DMA waits, sensor-write readback and best-effort sensor restart
on failure. Nine host fault tests cover the transaction. Integration testing
found that immediate stream-on application was unreliable. The deferred startup
integration now applies automatically after a 15-second preview settling period;
`x21_linear_720p120=0` opts out. Reliability remains experimental. No new MCU image
or external EEPROM write is involved in this timing change.

Later startup tests also found an ISP output of 1920x1080 behind a cached 720p
selection. Neither a ten-second readiness wait nor an atomic output update while
stalled resolved this. The atomic/stall geometry experiment was rejected and
removed from the timing helper. The automatic driver path has not passed
sustained capture.

A separate MCU revision 0102 fixes its busy-status pre-dispatch gap by including
both queued and active commands. The ISR patch passed all four pending/active
instruction-emulation cases, and the guarded automatic upgrade from revision
0101 completed on hardware. Its new marker was verified. Startup still failed
afterward, so this protocol repair is not evidence that the video issue is fixed.
Kernel19, including the atomic/stall experiment, was installed and its flash
readback verified. It failed on hardware. Repeating the minimal userspace timing
sequence reached about 119 fps but stopped after roughly six seconds. Restoring
the exact revision0101 MCU and kernel9 did not eliminate startup error15, so the
new MCU busy hook and driver integration alone do not explain all failures.

In a failed 60-fps state, the AP1302 ICP input counter continued at about 60 fps
while its HINF output and CIF capture counters stopped. A raw V4L2 capture also
stalled with OpenHD and the encoder absent. Four sensor-input MIPI lane status
registers showed neither error nor abort flags, and the X21 CSI counters remained
zero. These observations locate the stall inside the ISP path but do not prove
the root cause. Enabling SYS_START restart-error did not prevent a later stall.

The Windows RTP probe now reports the requested and observed duration, trailing
idle time, maximum frame gap and continuity result. It exits unsuccessfully for
a short burst followed by a stall, even if the burst's active FPS looks correct.
USB helpers now rediscover the NCM interface; Windows renumbered it from59 to10.

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
and exercised on the board. The test kernels were built from SDK kernel
commit 32e6c42 with these driver changes; the native kernel repository branch has
a different base, so deployment does not prove a full build of that branch.

For Windows debugging, video uses framed TCP directly over USB Ethernet and is
recreated as localhost UDP for QOpenHD. Encrypted video tunneling did not sustain
the higher-rate test. Telemetry uses a separate localhost TCP5760 tunnel through
temporary key-authenticated SSH. Helpers refresh the Windows USB source address
after reboot. A working picture alone does not establish a telemetry connection:
camera status, bitrate and configured FPS were separately verified as valid
OpenHD MAVLink messages. Configured FPS is not a measurement of delivered frames.

## Context-disabled mode integration

Revision0103 clears context2010 before HINF enable and retains manual AE and
the revision0102 pending-command busy fix. The context-bit interpretation is
inferred rather than a documented face-control API. Clearing AE's face-ROI bit
or selecting manual AWB separately did not prevent freezes. Explicit4 ms manual
exposure before output extended one run to about100 seconds but also froze.

The context-disabled diagnostic completed a180-second60-fps baseline and then
captured about120 fps for roughly9.5 minutes. It subsequently failed with ISP
error15, file00071975, line1230. Input counters kept advancing at about120 fps
while HINF/CIF stopped. A30-second steady USB sample measured119.7 fps with zero
RTP packet loss; the user confirmed continuous motion and a complete image.
The late freeze remains unresolved and this mode is experimental.

Kernel20 was cross-built from SDK32e6c42 with the integration, packaged, flashed
and read back successfully. Its probe automatically upgraded diagnostic01e1 to
revision0103, and its driver automatically reached720p120 without a userspace
register sequence. The selected format and telemetry can therefore report120.
This does not establish a full build of the separate native kernel branch.

Reusable Windows USB adapter discovery, video relay, telemetry tunnel and RTP
measurement tools are in `scripts/diagnostics/x21_usb/`. The relay bypasses the
image's208 KiB rmem_max cap and drains stale datagrams on viewer reconnection.
Private keys, host keys, temporary SSH server configuration, vendor archives,
rejected firmware and video captures remain local.

Onsemi specifies120 dB for three-exposure eHDR at4K30 and lists binned1080p120
as linear: https://www.onsemi.com/download/public-data-sheet/pdf/ar0822-pd-d.pdf.
1080p100 and full three-exposure HDR are separate investigations; neither is
currently presented as a validated mode.
The automatic kernel20 run delivered 2399 complete RTP frame timestamps in
20 seconds (119.9 fps), with zero sequence gaps or timestamp discontinuities.
Native camera telemetry reported 1280x720 and configured FPS 120. This short
sample does not resolve the earlier late ISP freeze.
