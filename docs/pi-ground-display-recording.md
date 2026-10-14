# Raspberry Pi groundstation display recording investigation

Tested 2026-10-11 on the groundstation at 192.168.1.124. This is a working
standalone prototype, not an integrated QOpenHD recording feature.

## Result

**Investigation stopped at the user's request.** The overclock was rolled back
after an unexpected groundstation reboot. No recording service was installed.
The prototype does not establish reliable Pi 4 groundstation recording at 720p60.

The Pi 4 can hardware-encode its composed display while decoding live OpenHD
video. Capture the firmware compositor using `vc_dispmanx_snapshot`, read the
result into a GStreamer `appsrc`, then use `v4l2h264enc` and `matroskamux`.
This captures the full display including the primary video and transparent
QOpenHD OSD. A visible secondary video should be captured as part of that same
composition; an actual second camera was **not** tested.

The tested device uses `vc4-fkms-v3d`, Linux 6.1.29-v7l+, a 1920x1080 display,
128 MB GPU memory, and the external `fpv_video0.bin` decoder. This capture path
is specific to the firmware compositor. Do not assume it works with full KMS
or on Pi 5.

## Encoder initialization issue resolved

The initial `v4l2h264enc` tests failed even at 640x480 using `videotestsrc`.
The kernel reported `Failed enabling i/p port, ret -3`; the firmware reported
`enc_open: consistency failed` and `EINVAL`. This was not evidence of an
occupied encoder or insufficient GPU memory: `vcdbg reloc stats` reported
67 MB free, the largest free block was 65 MB, and allocation failures were zero.

Explicit output caps resolve the failure:

```text
v4l2h264enc
  ! video/x-h264,profile=high,level=(string)4.2
  ! h264parse
```

Isolation test: specifying only `profile=high` failed; specifying only
`level=4.2` succeeded. The exact incorrectly selected default level has not
been traced. For direct RGB input, set `colorimetry=sRGB` and
`interlace-mode=progressive`; unspecified raw-parser colorimetry initially
caused a separate negotiation failure.

Related upstream report of the same initialization symptom:
https://github.com/raspberrypi/firmware/issues/1701

## Measured results

Ten-second recordings with live primary video and OSD, H.264 hardware encoder,
10 Mbit/s target bitrate, bounded raw-frame buffering and monotonic timestamps:

| Capture | Encoded frames | Measured fps | Decoded file duration |
| --- | ---: | ---: | ---: |
| 1280x720 RGB888 | 300 | 30.00 | 10.001 s |
| 1920x1080 RGB888 | 164 | 16.38 | 9.984 s |
| 1920x1080 RGB565 | 217 | 21.68 | 9.992 s |

Every encoded frame decoded successfully through GStreamer `avdec_h264`, ending
in EOS. A decoded 1080p frame was visually inspected: full primary camera image
and OSD were present. RGB565 reduces source color precision and is optional.
These are short feasibility tests, not sustained flight/load benchmarks.

A SIGTERM test also finalized correctly: 148 frames, 4.933 seconds, all frames
decoded. The recorder used about 56% of one CPU core and 13 MB RSS at 720p in
that short test. QOpenHD retained PID 16841 and `NRestarts=0`; the Pi reported
`throttled=0x0`. No latency instrumentation was performed, so service survival
does not establish unchanged display latency or frame pacing.

Capture-only RGB888 tests sustained 30 fps at both resolutions. The prototype
does not yet sustain full-color 1080p30 through the complete capture/encode path.
Investigate buffer reuse, copies, and firmware RGB conversion before claiming
1080p30. Avoid the initial pipe/raw-parser prototype: it was much slower and
assigned fixed-rate timestamps even when capture ran behind.

### Follow-up: targeting 720p60

The display was confirmed as 1920x1080 at 60 Hz using `tvservice -s`.
Capture-only tests produced 180 frames in 3.000 seconds at 1280x720 for both
RGB888 and RGB565. Generated RGB input through the hardware encoder completed
300 frames in 3.289 seconds (RGB888) and 3.055 seconds (RGB565), including
pipeline startup; this demonstrates encoding throughput on generated frames,
not a complete screen-recording benchmark.

| 1280x720 capture at target 60 fps | Measured encoded fps | Test length |
| --- | ---: | ---: |
| Original allocation per frame, RGB888 | 35.35 | 10 s |
| Original allocation per frame, RGB565 | 45.47 | 10 s |
| Reusable buffers, RGB888 | 49.27 | 10 s |
| Reusable buffers, RGB565 | 57.80 | 10 s |
| Final bounded buffer pool, RGB565 | 57.13 | 30 s |

The checked-in prototype now reuses a buffer pool (minimum four buffers,
maximum eight), with nonblocking acquisition. The thirty-second test encoded
1714 frames and skipped 86 capture ticks. This is close to 60 fps but **does
not** establish sustained 720p60. Per-frame allocation/page churn materially
affected throughput. Exact remaining stage costs have not been measured.

Direct-buffer experiments were attempted on the device:

- Reading a DispmanX snapshot directly into V4L2 encoder MMAP input returned
  an error and hung during cleanup. The experimental process was killed;
  the working recorder and live display continued operating afterward.
- Passing the snapshot resource's mailbox memory handle directly to a
  zero-copy MMAL encoder failed with MMAL EIO and firmware memory-handle
  errors. A DispmanX handle is not established here as a usable registered
  MMAL/VCSM handle. No working shared-buffer implementation resulted.
- A native MMAL host-buffer fallback reached about 41 fps RGB888 and
  51 fps RGB565 in ten-second tests, below the existing V4L2 buffer-pool path.

The unsuccessful experimental implementations were removed. The retained
prototype uses the measured, working host-buffer path. Proper shared-memory
registration/import remains unresolved.

```sh
sudo env OHD_CAPTURE_RGB565=1 ./pi-display-record \
  /Video/osd-720p60-test.mkv 1280 720 60 30
```

RGB565 trades color precision for bandwidth. Real 60 Hz display output also
does not prove the incoming camera or OSD supplies 60 distinct updates/second.
The 30-second sample is `/Video/display-proof-720p60-pool-final.mkv`.

### Modest device overclock

With user authorization, the groundstation boot configuration was backed up to
`/Video/display-recording-investigation/config-before-overclock.txt`, then this
Pi-4-only block was inserted before OpenHD's dynamic configuration section:

```ini
# OpenHD recording clock experiment (Pi 4 only)
[pi4]
arm_freq=1700
h264_freq=550
isp_freq=550
[all]
```

The device rebooted successfully. CPU, H.264 and ISP clocks reached the requested
1700/550/550 MHz under load. Core frequency remained 500 MHz; no voltage override
or forced turbo was added. These settings are device-local, not image defaults.
Clock option reference: [Raspberry Pi configuration documentation](https://www.raspberrypi.com/documentation/computers/config_txt.html#overclocking-options).

An isolated 30-second RGB888 recording with the normal `ondemand` governor
encoded **1760 frames in 30.005 seconds (58.66 fps)**, skipping 40 capture ticks.
All 1760 frames decoded successfully; file duration was 30.006 seconds.
Temperature was 46.7–48.2 C and `get_throttled` remained `0x0`. CPU and accelerator
clocks still varied under DVFS. Sample: `/Video/display-proof-oc-720p60-rgb.mkv`.

Subsequent RGB565 and performance-governor recordings overlapped with other
benchmarks/verification. Their throughput is not a valid isolated comparison.
After the user powered the Pi back on, an isolated 60-second test with the CPU
governor temporarily set to `performance` encoded 3544 RGB888 frames in 60.002 s
(59.06 fps), skipping 56 capture ticks. All frames decoded; duration was 60.003 s.
The governor was restored to `ondemand` afterward. Clocks stayed at 1700/550/550
MHz in sampled readings; temperature reached 40.4 C and sampled throttling flags
were zero during capture. Sample:
`/Video/display-proof-oc-performance-alone-720p60.mkv`.

This post-boot test had **no incoming camera data**: the running `fpv_video0.bin`
decoder's input byte count did not advance. It therefore measures OSD/display
recording without live decoding load. Sustained full-color 720p60 remains unproven.

The subsequent isolated RGB565/performance-governor run encoded 3567 frames in
60.000 s (59.45 fps), skipping 33 capture ticks. It also had no camera input.
All 3567 frames decoded successfully; file duration was 60.000 s.
Sample: `/Video/display-proof-oc-performance-alone-rgb565-720p60.mkv`.
Sampled temperatures reached 45.7 C. The CPU governor was restored after capture.

After that capture, the kernel logged a brief undervoltage event at boot-relative
109.593 s and recovery at 111.673 s. Subsequent `get_throttled` readings were
`0x50000`: historical undervoltage and throttling, with current-condition bits
clear. Do not interpret this as thermal throttling or a clean power test.
Flag definitions: [Raspberry Pi vcgencmd documentation](https://www.raspberrypi.com/documentation/computers/os.html#get_throttled).

### Actual live-video follow-up and rollback

The air unit at 192.168.1.42 was reachable with a running 1920x1080@30 camera,
but RF video was not reaching the ground decoder. Restarting the air service
did not restore reception. For the recording experiment, its SysUtils
`nw_manual_forwarding_ips` setting was temporarily set to `192.168.1.124`.
This forwarded the existing camera stream to the ground receiver over Ethernet;
decoder input increased from 10,842,710 to 55,741,525 bytes during capture.
It was not an RF-link validation or a 60 fps camera-source test.

With this live decoding load, full-color 1280x720 capture at target 60 fps,
overclocked and with the performance governor, encoded **2484 frames in
60.012 seconds (41.39 fps)** and skipped 1116 capture ticks. Temperature reached
52.1 C. Sample: `/Video/display-proof-live-oc-rgb-720p60.mkv`.

The Pi unexpectedly rebooted during subsequent file verification. The user
confirmed they had not restarted it. Complete decode verification and recorded
frame inspection did not finish, so the file is not reported as fully verified.
An additional undervoltage event had also been logged during the earlier RGB565
test (boot-relative 161.595–167.835 s). The cause of the restart is unresolved;
neither overclock stability nor adequate power delivery was established.

The original `/boot/config.txt` was restored from the saved backup, `ondemand`
was restored, and a reboot was issued to apply the original clocks. The temporary
air forwarding setting was subsequently cleared and the air service restarted
to remove it. The user requested that testing stop; no further benchmarks were
performed. The live RGB565 benchmark was not run.

### Subsequent restoration of normal video

After testing stopped, the user reported missing video and requested recovery
without physically reconnecting the radio. The air radio had enumerated as
`0bda:1a2b` (driver CDROM mode); OpenHD reported no broadcast card or functional
link. A normal eject attempt failed. The existing
`/usr/local/sbin/openhd-zerocd-recovery.py` helper successfully recovered it as
`0bda:c812` by stopping competing services and cycling USB power with
`uhubctl -l 2`, then restarting services. A matching ground-side USB power-cycle
also completed, but neither recovery alone restored video.

Both radios were configured for 5745 MHz. The air TX power setting was 0%;
changing it to 20% alone did not restore reception. Retuning both units to
5180 MHz then restored incoming video: the ground decoder input increased
from 4,177,818 to 6,650,369 bytes over three seconds. Working settings were
left at 5180 MHz on both units and 20% air TX power. This does not prove
whether the original problem was channel-dependent or cleared by retuning.
QOpenHD crashed once with SIGSEGV when video returned and systemd restarted it.
The restarted HUD loaded and its external decoder subsequently received
9,343,565 to 11,774,166 bytes over three seconds. A separate earlier observation
window crossed decoder replacement, so it is not evidence of uninterrupted
decoder uptime. Visible display confirmation was requested from the user.
Subsequent SysUtils logs identified decoder exits on additional decode-port
format-change events. The current replacement decoder nevertheless consumed
8,156,531 bytes over a ten-second observation. A twelve-second parser-only
observation of the dedicated localhost 5800 forwarding output negotiated
1920x1080@30, H.264 High level 4.2, without another caps change. This does not
establish long-term display stability or explain those decoder port events.
The temporary Ethernet video forwarding remains disabled; recording tests
were not resumed.

To restore the saved boot configuration and apply it:

```sh
sudo cp /Video/display-recording-investigation/config-before-overclock.txt /boot/config.txt
sudo reboot
```

## Reproduce

Source: `scripts/pi_display_record.c`. On the Pi with existing development
headers and GStreamer plugins:

```sh
gcc -O2 -Wall -Wextra \
  -I/usr/include/interface/vcos/pthreads \
  -I/usr/include/interface/vmcs_host/linux \
  pi_display_record.c -lbcm_host \
  $(pkg-config --cflags --libs gstreamer-app-1.0) \
  -o pi-display-record

# Ten seconds, or use duration 0 and stop with Ctrl-C/SIGTERM.
sudo ./pi-display-record /Video/osd-test.mkv 1280 720 30 10
sudo ./pi-display-record /Video/osd-full-resolution.mkv 1920 1080 30 10

# Optional reduced color precision.
sudo env OHD_CAPTURE_RGB565=1 ./pi-display-record \
  /Video/osd-rgb565.mkv 1920 1080 30 10
```

The compiled prototype was also saved persistently at
`/Video/display-recording-investigation/bin/pi-display-record`, with the source
at `/Video/display-recording-investigation/pi_display_record.c`.
Verified samples remain at:

- `/Video/display-proof-direct-720p30-final.mkv`
- `/Video/display-proof-direct-1080p30-final.mkv`
- `/Video/display-proof-direct-1080p30-rgb565.mkv`
- `/Video/display-proof-stop.mkv`

Local evidence is under ignored `out/display-recording/`, including
`720p30.mkv`, `display.png` (initial snapshot), and `final-frame.png`
(decoded hardware recording). Other `display-proof-*` files on the device are
intermediate experiments; do not treat their nominal 30 fps headers as measured
30 fps recording performance. Boot clocks were changed as documented above;
no service definitions were changed.

## What "full video and video2" requires

Two outputs serve different purposes:

1. **What the pilot sees:** record the composed display. This includes OSD,
   primary video, and secondary video at its displayed size, including menus
   when they are open. It cannot preserve pixels hidden behind a PiP or menu.
2. **Both original full-resolution feeds:** separately record the incoming
   encoded primary and secondary streams without transcoding, alongside the
   display recording. These streams do not acquire OSD automatically.

The device currently has `dev_qopenhd_n_cameras=1`; actual video2 recording
remains unverified. Do not infer this setting from
`dev_secondary_video_input_mode`, which is an Android test-input option.
OpenHD's ground video forwarding uses ports 5600 and 5601. Add recording tees
inside the existing receive/forward path or use dedicated forwarding ports;
do not compete with the display receiver for the same UDP socket.

QOpenHD offers a secondary `qmlglsink` path. Its AVCodec secondary fallback
needs care on the Pi: `ExternalDecodeService.hpp` selects the primary stream
unless streams are switched, and `OpenHD-SysUtils/src/sysutil_video.cpp` owns
one `g_video_pid` and a fixed primary-port pipeline. That external-service
fallback is not an independent two-video-plane implementation. Confirm the
installed build's secondary backend and test two actual streams before claiming
complete dual-camera support.

For integration, let SysUtils own a separate optional recording process and
expose start/stop/state to QOpenHD. Preserve real elapsed timestamps and bound
raw input buffering; recording overload must not block the live display path.
Add storage/free-space handling and test unexpected termination, disk-full,
long runs, actual video2, and live latency before enabling automatic recording.
