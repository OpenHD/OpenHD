# Camera backend and recording diagnostics

With persistent logging enabled, `camera.log` includes camera-worker diagnostics.
Workers forward a bounded queue over their existing private IPC channel; the
parent alone owns file rotation. Encoder threads do not write logs to storage or
send diagnostic packets themselves. Queue/socket congestion drops diagnostics
and is reported as `worker diagnostic messages dropped` when delivery recovers.
Existing MAVLink warning forwarding remains separate.

Search for `video-backend`, `video-encoder`, `video-diag`, and recording started /
stopped / failed messages. Backend selection alone does not confirm encoder
startup. Initialization and encoded-output reports provide that evidence.

Reports are emitted at startup and when the observed configuration or health
state changes. A short measurement window supplies bitrate/FPS snapshots after
changes; unchanged operation does not produce periodic status or latency logs.
Short state changes between measurement windows can be absent from the summary;
recording start/stop/failure events are still logged separately by native MPP.

* `native-mpp` / `rockchip-mpp`: direct MPP API. A separate recording context is
  created only when requested. `record_encoder_initialized` confirms successful
  encoder/file setup; `record_has_output` confirms successful muxer writes.
  `dual_encoder_active` requires both contexts to produce frames in the observed
  window. It does not mean a second camera is active.
* `gstreamer`: the instantiated bitrate-control element and factory are logged
  when identifiable (for example an MPP plugin can still run inside GStreamer).
  Recording uses the same encoded tee, so `dual_encoder_active=false`.
  A configured recording branch does not verify disk writes.
* `native-rkmpi` / `rkmpi-venc`: Pico native MPI implementation. Recording shares
  encoded output with transmission, so it also reports no separate encoder.

Native MPP reports requested/target/measured live bitrate and target/measured
recording bitrate in kbit/s, frame rates/counts, keyframe counts/GOP in frames,
QP bounds, recording mode, arming state, recording-worker availability and drops.
Measured rates are elementary video payload, before RTP/FEC and container
overhead. Recording counts include successful writes to the muxer, not durable
storage confirmation. A stalled/failed channel can have a target but no output.
The configured target is not an encoder property readback.

For shared-output implementations, `record_encoded_kbit_s` describes the shared
encoded stream while a recording branch/file is present. It is not an
independently measured recording or disk-throughput rate. The log labels this
scope explicitly. Recording modes: 0 off, 1 on, 2 automatic while armed.

Native MPP also emits latency percentiles and input/drop totals on observed
state changes to help distinguish capture gaps, encoder delays and recording
write delays. No log report by itself proves the cause of periodic stutter.
