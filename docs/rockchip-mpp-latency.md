# Native Rockchip MPP latency and recording

## Reference evidence from Consti's RV1126 work

The saved [720p120 capture/ISP log](https://github.com/OpenHD/rv1126_ohd_sushi/blob/master/a_consti/proc_and_isp/imx415_csi0_720p_120fps.txt)
requests 120 fps and reports an 8 ms capture interval (`fps:125`), with zero CSI
errors. The advertised 134 fps sensor mode is not a measured output rate. A
[later timestamp investigation](https://github.com/OpenHD/rv1126_ohd_sushi/blob/master/a_consti/with_timestamp_fix_from_pdf/with_sof_changed.txt)
also shows 1280x720 NV12 output at an 8 ms driver-reported interval.
These coarse interval snapshots support approximately 120 fps capture/ISP
operation; they do not establish a precise sustained encoded or RF frame rate.

His [1080p90 log](https://github.com/OpenHD/rv1126_ohd_sushi/blob/master/a_consti/proc_and_isp/imx415_csi0_1080p_90.txt)
shows 1920x1080 capture/ISP at 11 ms intervals and zero CSI errors, but the final
NV12 output is scaled to 1280x720. The results are from RV1126 with IMX415 and
must be measured again for the RV1126B, camera and pipeline used on X21B.

## Input ownership

The OpenHD native MPP backend uses V4L2 DMA buffers directly when single-plane
NV12 has the exact encoder stride and UV offset. Failed exports/imports and
incompatible padding use the existing row-copy path. Debug noise always uses a
copy, so capture buffers are never modified. Negotiated dimensions must match
the encoder; unsupported multi-plane layouts are rejected.

Capture buffers handed to transmission remain dequeued until all encoder output
partitions complete. A bounded two-slot transmit queue retains at most two of
the four capture buffers; rejected raw transmit frames return their buffer
immediately. Capture continues feeding recording when transmission is delayed.
Encoder failure ends the stream and destroys the encoder before returning or
releasing its input. Input/output waits use 250 ms MPP timeouts, with a two-second
partition deadline. File-system calls can still block the recording worker.

## Timing logs

Every five seconds the `mpp_stream` logger reports p50, p95 and maximum timings
in milliseconds for the latest at most 512 observations per stage:

- `capture-buffer-age`: kernel timestamp to V4L2 dequeue. Reported only for a
  nonzero timestamp explicitly marked `V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC`.
  The timestamp source is logged as start of exposure or end of frame.
- `transmit-copy`: fallback NV12 copy and debug modification. Absent on DMA input.
- `transmit-encode-and-output`: frame submission through the final encoded
  partition and synchronous RTP/output callback work.
- `output-callback`: synchronous handoff to the link; it does not measure RF delivery.
- `dequeue-to-output`: application capture dequeue to encoded output handoff.
- `capture-timestamp-to-output`: capture-buffer age plus dequeue-to-output.
- `recording-copy`, `recording-queue`, `recording-encode-and-enqueue`,
  `recording-writer-queue`, `recording-write`, `recording-dequeue-to-write`:
  isolated recording stages, including time waiting for the SD writer.

Totals distinguish DMA/copy frames, capture sequence gaps, invalid capture frames,
and dropped raw recording inputs. Sequence wrap is handled; backward sequence
resets are not interpreted as billions of missing frames. These are sender-side
measurements, not glass-to-glass latency. The video frame creation time passed
to the existing link timing statistics now uses application dequeue time.

## Recording ownership and overload

A recording worker owns its encoder and ROI metadata. A separate writer owns
the Matroska file and recording counter. Three preallocated MPP input buffers
bound copying, queued and encoding work. Capture submits recording input before
handing transmission to its own worker; transmit rate-control, encoding and
output callbacks never run on the capture thread. Capture never waits for a
recording or transmission slot.
When full, it drops the new raw recording frame before encoding; it never drops
an already encoded reference frame. Matroska timestamps preserve elapsed time
across those gaps and complete Annex-B access units are split into their NALUs.

The file writer buffers up to 32 MiB of complete encoded access units, including
the in-flight write, to absorb short SD stalls without retaining raw encoder
inputs. Counter/open/write failures are contained by the writer. A full byte
budget or writer failure ends that recording session; it never drops encoded
reference frames from a continuing file. Accepted data drains on normal close.
Stopping/disarming discards pending raw work. Encoder failure retires its context
before recycling input and retries with a fresh file and codec headers after one
second. Transmission and recording retain independent MPP contexts and rate
settings, but still share the SoC's encoder hardware and memory bandwidth.

Recording defaults to QP 4–51 so rate control can meet its 40 Mbit/s target even
with a complex/noisy scene; the recording maximum bitrate hint equals its target.
Existing persisted QP settings are preserved. A restrictive maximum QP can still
make the requested rate unattainable. Encoder rate control is not a per-packet
traffic shaper.

## Copy-versus-DMA benchmark on X21B

Use the same camera mode, codec, bitrate, GOP, ROI, exposure, scene, transport,
and thermal conditions for both runs. Disable debug noise. Warm up each run,
then record at least 60 seconds of timing logs with recording off and again with
recording on. Separately collect CPU load, temperature and actual encoded FPS.

Run the native backend with `OPENHD_MPP_FORCE_COPY=1` in its process environment
for the baseline; unset it for automatic DMA input. Keep the backend selector
on native MPP for both runs. Verify the input totals actually increment `DMA`
in the automatic run: a layout that falls back to copies is not a DMA benchmark.
Changing the environment requires restarting the process. For a systemd-managed
process, apply the environment to that service's existing launch configuration.

Compare per-window timing distributions, sequence gaps, recording drops, CPU,
and FPS. Per-window p95 values must not be pooled as if they were raw samples.
Perform an independent visual/end-to-end check for color, padding, frame
corruption and decode recovery. No measured DMA speedup is claimed by host tests.

## Host regression checks

```sh
scripts/test_rockchip_mpp_stream.sh
```

Requires a C++17 compiler, Python 3, FFmpeg with libx264/libx265, and ffprobe.
The checks cover padded/truncated layouts, sequence wrap/reset, bounded latency
samples, slot ownership under a stalled consumer, stop wakeups, and H.264/H.265
Matroska decoding with explicit timing gaps. ARM64/legacy-MPP compilation is a
separate check; neither proves DMA coherency or sustained throughput on X21B.
