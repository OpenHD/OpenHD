# ORQA direct Hantro capture and encoding

ORQA cameras use the direct VC8000E API when its SDK is available. Other NXP
cameras keep the existing V4L2 mem2mem encoder. Build with
`OPENHD_ENABLE_ORQA_HANTRO=OFF` to select the previous ORQA path explicitly.
The ORQA cross-build script checks for the Hantro SDK and verifies the resulting
runtime dependency, so an ORQA release cannot silently omit this backend.

The camera captures packed YUYV into three EWL allocations shared with the
encoder, using USERPTR or DMA-BUF import. The hardware preprocessor accepts
packed YUYV directly. There is no CPU conversion, raw-frame memcpy, V4L2 encoder
daemon, or application queue of unencoded pictures. If more than one captured
picture is ready, the encoder processes the newest and returns older buffers.
Capture never regains ownership of a buffer while encoding still reads it.

The encoder uses one core, no lookahead and an IPPP reference sequence without
B pictures. Keyframe interval, slices, QP limits, rotation, codec and bitrate
come from the existing camera settings. Live bitrate/QP updates and explicit
keyframe requests run on the capture thread. Output remains Annex B and uses
OpenHD's existing RTP packetizer and transport. This is full-frame encoding;
it does not claim sensor-line processing or early slice delivery.

The shared RTP packetizer now sends codec configuration before the first
picture, instead of waiting for the periodic one-second resend timer. Its
multi-NAL splitter also bounds each search to the remaining bytes. A protected
page regression checks the splitter and verifies SPS/PPS precede the first
H.264 picture.

A live bitrate change starts from the previous picture's measured QP, with an
initial adjustment limited to four QP steps. Picture rate control then continues
adapting. This avoids a large bitrate cut spending another GOP at the old
quantizer. A two-second capture stall releases and reopens the direct backend;
the old capture references are released before another encoder is started.

The NXP C ABI represents `bool` as a 32-bit enum, while the SDK's C++ header
path would use one-byte C++ `bool`. The implementation confines a matching
32-bit type to the vendor headers and checks the configuration layout at
compile time. Vendor headers and libraries are dependencies, not copied into
the repository.

## Hardware verification

The standalone target `test_orqa_hantro` exercises capture, encoding, a live
8-to-3 Mbit/s rate change, QP updates and a forced keyframe. Stop the existing
camera owner before running it; restore that owner afterwards. For example:

```sh
systemctl stop openhd
trap 'systemctl start openhd' EXIT
./test_orqa_hantro --seconds 16 --output /tmp/camera.h264
./test_orqa_hantro --h265 --seconds 16 --output /tmp/camera.h265
```

Decode each elementary stream with an independent decoder, for example
`ffmpeg -v error -i camera.h264 -f null -`. `test_nxp_v4l2` additionally exercises
the OpenHD adapter, RTP callbacks and telemetry. It uses existing camera
settings, so preserve those settings before running it.

Every ten seconds the running adapter reports frames, discarded stale captures,
encode duration, and capture-timestamp-to-encoded-output duration. Those
durations exclude exposure, sensor readout before the capture timestamp,
transport, decoding and display scanning. They are not glass-to-glass latency.

Air recording is not implemented by this native NXP stream, as in the previous
V4L2 backend. Unsupported capture layouts or SDK errors are reported explicitly;
the direct backend does not silently substitute a CPU-copy pipeline.

## Verified on 19 October 2026

Device: `192.168.1.132`, ORQA DTK Standalone Plus, kernel
`6.6.52-orqa-g2857f74b2fba-dirty`, VC8000E library `1.10.1-r0`.
Both standalone encoding and the full OpenHD adapter ran on the physical board.
The full application was cross-built with GCC 11.4 against a sysroot copied
from this board; no system libraries were replaced. The source snapshot
included the existing worktree changes at HEAD
`cdd33271810ef1e932c60a2b495cce4c3d0062a9`.

The existing adapter regression passed with these measured RTP byte rates:

| Codec | 12 Mbit/s target | After change to 3 Mbit/s | Largest callback gap |
| --- | ---: | ---: | ---: |
| H.264 | 12.26 Mbit/s | 2.48 Mbit/s | 30.9 ms |
| H.265 | 14.03 Mbit/s | 3.18 Mbit/s | 35.9 ms |

These are rate-controlled hardware streams, not a strict wire-rate ceiling.
The regression also checked continuous output during the transition, RTP
callbacks, keyframe indication and camera telemetry at 960x720/60.

A separate loopback round trip captured 12,227 H.264 RTP packets and 13,131
H.265 packets with no sequence gaps. After depacketizing, FFmpeg with `-xerror`
decoded 619 pictures per codec without errors, including startup. This checks
OpenHD packetization and UDP output, not a physical ground link.

The installed application reported 2,364 encoded frames after approximately
40 seconds, no discarded stale captures, a 1,495 microsecond average encode
duration (1,675 microsecond maximum), and a 1,533 microsecond average from the
capture timestamp to encoded output. This proves the direct encoder is active;
it does not measure exposure-to-display latency or an improvement against a
measured baseline. The board reports no functional OpenHD video link, as it did
before this change.

The installed binary SHA256 is
`ac308af41ba5d05d5c065c486db445eb5910bcebb975e0ce7743696f58e1cb71`.
The original binary is preserved at
`/usr/local/bin/openhd.before-hantro-20261019.2qUolF` (SHA256
`599cea3dd77477aae60b1efb66edcec561d6730b772e854e41495a45a2678aa5`).
Rollback, on the board:

```sh
systemctl stop openhd
cp -a /usr/local/bin/openhd.before-hantro-20261019.2qUolF /usr/local/bin/openhd
systemctl start openhd
```

Local binaries and validation logs are in `out/orqa-low-latency/`. Test camera
settings were restored before restarting the normal service. Hardware validation
did not include a reboot, receiver/display timing, RF testing or a CI run.
