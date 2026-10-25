# Pico native camera backend

Pico component builds set `OPENHD_PICO_MPI=ON`, `OPENHD_MPP_ONLY=ON` and
`ENABLE_GSTREAMER=OFF`. The RV1103/RV1106 implementation uses RKAIQ for sensor
and ISP control, then binds Rockit VI directly to hardware VENC. Encoded access
units use OpenHD's existing RTP helper and isolated camera-worker protocol.
There is no GStreamer dependency or `gst-perf` requirement.

The backend reports camera presence, encoding state and measured bitrate/FPS
through the existing worker status channel. Bitrate, QP limits, orientation and
IDR requests go directly to VENC. Shutdown unbinds VI/VENC before releasing the
encoder, VI channel, RKAIQ context and MPI system. A five-second frame timeout
restarts the native path. The parent process watchdog also covers vendor calls
that do not return. Standard recording saves the same encoded stream to MKV.

## Consti's sushi reference

Reviewed `OpenHD/rv1126_ohd_sushi` at
[`853fa1fc2d50`](https://github.com/OpenHD/rv1126_ohd_sushi/tree/853fa1fc2d50f0e4f9b5eea71d1ff2657c9a2765):

- [`mpi_stream_test.cpp`](https://github.com/OpenHD/rv1126_ohd_sushi/blob/853fa1fc2d50f0e4f9b5eea71d1ff2657c9a2765/external/mpp/test/mpi_stream_test.cpp)
  carries capture timestamps through encoding and copies encoded data before
  handing it to a consumer thread. The Pico backend similarly returns VENC's
  DMA output before RTP/recording work. It preserves Rockit's monotonic capture
  PTS in worker frames and recording timestamps, with bounds for invalid or
  foreign clock values.
- [`consti_stream.cpp`](https://github.com/OpenHD/rv1126_ohd_sushi/blob/853fa1fc2d50f0e4f9b5eea71d1ff2657c9a2765/external/rkmedia/examples/consti_stream.cpp)
  documents SPS/PPS sharing an access unit with an IDR. Pico's keyframe detection
  scans the complete Annex-B access unit and marks H.264/H.265 random-access
  frames for the existing transport and worker protocol.
- The experimental raw-UDP NAL delimiter workaround is unnecessary with our
  RTP packetizer. The sushi consumer queue has no capacity limit and polls
  continuously; that queue is not copied. VI/VENC buffer counts and worker IPC
  remain bounded. Its older RKMedia bitrate units and API layout must not be
  copied into the RV1106 Rockit API, whose CBR field uses kbit/s.
- ISP denoise/sharpen disabling in `sample_common_isp.c` is explicitly marked
  as an experiment. Pico retains its sensor IQ profile and automatic 3A rather
  than silently disabling image processing.

`test_rockchip_stream_utils` checks capture-clock conversion, invalid PTS,
SPS/PPS-prefixed H.264 IDR, H.265 random-access frames, truncated NALs and existing
buffer ownership/overload behavior. Hardware verification is separate from
these checks: camera frames, status, decoded video, and requested controls must
be checked on the connected board.
