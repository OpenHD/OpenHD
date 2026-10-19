# Camera worker isolation

GStreamer, native Rockchip MPP and native libcamera cameras each run in an exec'd
copy of the running OpenHD executable. The private `--gst-camera-worker`,
`--mpp-camera-worker` and `--libcamera-worker` entry points run before the dashboard, singleton
guard, telemetry, plugins and radio are initialized. The worker inherits a private
Unix socketpair, closes other inherited descriptors, and exits if its parent dies.
No public listening port or separately installed worker binary is required.

OpenHD keeps the camera parameter service, persistence, camera plugins and link
forwarding in the parent. Settings snapshots, bitrate/QP requests and arming state
are sent to the worker. GStreamer applies snapshots through its camera-thread mailbox.
Native backends stop before applying snapshots and recreate their stream inside
the worker. Bitrate adaptation stays live; native MPP QP and force-keyframe controls
also stay live. MPP ROI, recording configuration and debug setting changes currently
rebuild the native stream, rather than mutating the running owner's settings.
Workers never persist a competing copy of camera settings. A worker's
low-space recording disable is propagated back to the parent for persistence.

Encoded frames return with their RTP boundaries, creation timestamp and encryption,
intra-stream and IDR flags. IPC uses bounded, nonblocking packets. Congestion drops
incomplete frames rather than forwarding damaged partial frames. Camera statistics
and MAVLink log messages also return to the parent. Recording index allocation uses
a file lock so independently running cameras cannot allocate the same counter.

## Recovery

- Existing GStreamer operation timeouts terminate only the camera worker. Its
  supervisor reaps it and restarts that camera with the current parent settings.
- A 30-second progress watchdog catches hangs outside those timeout helpers,
  including a stuck setup, runtime property operation, or frozen worker process.
  Progress comes from the camera loop, not merely a responsive control thread.
  Native workers use completed encoded frames as progress, with an exemption
  while streaming is explicitly disabled. Failed MPP initialization/capture or
  encoding therefore also triggers recovery, rather than leaving a dead stream.
- A replacement is never started until the old worker has been reaped. If a
  kernel driver leaves a killed process in uninterruptible sleep, that camera
  remains unavailable while the rest of OpenHD continues running.
- Normal shutdown requests pipeline cleanup first, allowing six seconds before
  killing a stuck worker. The parent waits at most one additional second for
  reaping during shutdown. A hard-killed worker may leave an unfinished recording.
- Libcamera independently restarts its encoder after ten seconds without output.
  Bitrate commands use a nonblocking socket with SIGPIPE suppression. Graceful
  encoder shutdown gets three seconds, followed by SIGKILL and one second of
  bounded reaping. Its nested encoder receives a parent-death signal if the
  camera worker crashes. No blocking `waitpid` remains in that shutdown path.

## Regression check

On Linux with the normal OpenHD build dependencies, `perf` and `x264enc`:

```sh
cmake --build OpenHD/build --target test_gst_camera_process --parallel 2
timeout 120s OpenHD/build/ohd_video/test_gst_camera_process
```

The test uses temporary recording/configuration paths and in-memory camera
settings. It checks frame reconstruction and nonblocking congestion handling,
GStreamer helper timeout containment, two synthetic cameras, live settings,
capacity limits, auto recording, crash recovery, a frozen worker, and clean reaping.
It also uses a fake libcamera encoder that stalls once and ignores SIGINT to
exercise no-data recovery, forced shutdown, worker crash recovery and bounded stop.
`OPENHD_LIBCAMERA_APP` can select an absolute encoder executable for testing;
normal installations use `rpicam-vid` or `libcamera-vid` as before.
The existing Air/Ground build workflow runs this regression.

Validation snapshot, 2026-10-16: the full OpenHD Linux build and the regression
passed locally under Ubuntu 22.04 in WSL, with two synthetic H.264 camera streams.
Native libcamera recovery was exercised using the fake encoder: no-data restart,
forced encoder shutdown, worker crash recovery, nested-encoder termination and
bounded application shutdown passed. MPP-only supervisor and encoder sources
passed a host compiler syntax check against the available Rockchip SDK headers.
`scripts/test_rockchip_mpp_stream.sh` passed its buffer/latency checks and
H.264/H.265 decoding with preserved recording timestamp gaps. This does not
establish an ARM package build or native MPP hardware recovery.
Workflow YAML parsing and `git diff --check` also passed. This is local evidence;
no GitHub Actions run or device deployment is claimed.

## Boundaries

This isolates GStreamer camera pipelines, including their attached FleetControl
output, and native MPP/libcamera backends. Audio and native NXP retain their existing
execution model. Parent-owned camera controls and plugins also retain their existing model.
It does not guarantee recovery from a wedged kernel driver or protect the parent
from its own bugs. Real cameras, encoders, recording playback, RF/telemetry continuity
and performance at production resolutions require hardware validation.
In particular, a libcamera encoder stuck in uninterruptible kernel sleep may
outlive its killed worker; user-space supervision cannot guarantee device release.
