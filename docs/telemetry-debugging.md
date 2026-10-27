# Missing telemetry / RSSI debug image

The telemetry diagnostics below are enabled in source by default. No raw
recording switch is needed. They write bounded summaries every five seconds
to normal OpenHD logs and the QOpenHD journal. Payloads, locations, parameter
values and credentials are not logged. Counters are cumulative for the lifetime
of the endpoint; callback/GUI delay maxima cover the last reporting interval.

## Changes

UART output now uses a dedicated writer, a queue of at most 128 complete
MAVLink frames and nonblocking writes. Producers enqueue without waiting for
the serial line. The writer handles short writes and polls for backpressure;
a stalled frame times out after 500 ms and requests a reconnect. Pending frames
are discarded on disconnect/stop and tagged with a connection generation so
they cannot be replayed onto a replacement device. Stop cancels the writer.

RC overrides/manual control lead, then commands/acknowledgements/heartbeat/time
sync, normal telemetry, and parameter-value/file-transfer bulk traffic. Existing
OpenHD UART source priority settings remain the primary ordering when present.
Equal-priority frames retain order; every ninth selection serves the oldest
pending frame so bulk traffic can progress. Unsigned pending RC samples from
the same sender are replaced with the latest sample and expire after 250 ms.
Other queued frames expire after five seconds. A full queue rejects incoming
frames unless a lower-priority pending frame can be evicted. Signed frames
retain timestamp order within a sender. Queue acceptance is not physical or
remote delivery; failures/expiration/eviction have separate counters.

Each OpenHD endpoint now owns its receive parser. Previously, endpoint creation
monotonically allocated an index into MAVLink's finite global channel array.
After 16 lifetime endpoint creations on Linux this could access outside the
array. Parser state also resets when a UART reconnects.

FC detection requests parameter index zero first. One valid response is enough
for ArduPilot evidence. One full-list fallback remains for INAV versions that
only expose their empty response on LIST; repeated full-list probes are gone.
PX4 and Betaflight detection remain supported.

## Reading the logs

* `telemetry-source-v1`: local OpenHD statistics before encoding. Includes system,
  role, readiness, age of the last producer update, active card/type, adapter and
  antenna RSSI, received packet count and Devourer metadata. `age_ms=-1` means
  no producer update has ever arrived. An increasing age points at stalled link
  work/statistics rather than QOpenHD parsing.
* `telemetry-path-v1`: endpoint parsing and send-call results grouped as
  `sys/comp/msgid:rx,tx_batch_ok,tx_batch_failed,bad_crc,bad_signature`.
  TX columns count messages in successful/failed batches; a failed batch can
  still have partially accepted messages. They do not prove remote delivery.
  Includes the latest received card RSSI and maximum callback duration.
  Maps are capped at 128 distinct keys; additional keys increment `untracked`.
* `serial-tx-v1`: current/high-water queue size, completely written frames,
  incomplete/failed frames, rejected/evicted/cleared frames, expired frames,
  replaced RC samples and messages rejected while disconnected.
* `telemetry-receive-v1`: QOpenHD receive-thread accepted/CRC/signature counts
  grouped by source system/component/message. No payloads are logged.
* `telemetry-model-v1`: QOpenHD model values, Air/Ground card-message counts,
  alive flags and maximum receive-to-GUI dispatch delay. Runs even while no
  messages arrive. These two QOpenHD summaries bypass the normal
  `log_warnings_only` filter.

For a failure, export the usual OpenHD logs from **both units** after 30-60
seconds of reproduction, and save `journalctl -u qopenhd --no-pager` on Ground
(use the actual QOpenHD service name if different). Match `sys/comp/msgid`
between generation, forwarding, QOpenHD receipt and model state. The radio-card
message ID is 1212. Ground RSSI is local system 100; Air is system 101. FC
telemetry commonly originates at system 1 and follows a separate routing path.
The HUD's `N/A dBm` means its Ground RSSI property is <= -127. `loss 0%` alone
does not establish that current RSSI/card telemetry is present.

Optional `LOG_MAVLINK` recording on both units retains the detailed `.ohd`
records under the recording directory's `logs/` folder if the summaries are
insufficient. Do not leave full recording on unnecessarily.

## Regression window: 19-23 October 2026

* OpenHD `41fae4aa` (19 October) added active FC firmware probing, including
  full parameter-list requests. A full ArduPilot parameter dump adds substantial
  UART/RF/UI load. The new single-parameter-first probe removes that normal
  case. This is a plausible congestion contributor, not proof of the RSSI cause.
* QOpenHD `7c52c58ac` (19 October) added FC diagnostics and component-aware FC
  routing. The new component filter can explain missing messages originating
  from another FC component, but does not directly filter OpenHD system
  100/101 radio statistics. Diagnose source component counts before changing it.
* QOpenHD `116bb873d` (23 October) and `82c4f477a` (24 October) already address
  excessive FC inspector work/GUI stalls. Older images in this window may lack
  those fixes. GUI delay counters distinguish that from absent RSSI messages.
* The supplied `other.log` contains 648 Air RX telemetry queue-enqueue failures;
  `other (2).log` overlaps it and must not be added as independent failures.
  `openhd (3).log` contains roughly 338 ms tracker writes. Async serial output
  removes this blocking path, but does not establish RF/USB health.

These are source/log findings. Pi image build, boot, sustained RF/video and
physical HUD verification remain separate checks.

## Local checks

Configure `OpenHD/ohd_telemetry` with CMake and build/run `test_serial_async`,
`test_fc_firmware_detector`, `test_legacy_radio_settings` and
`test_statistics_contract`. The async test uses a Linux PTY and covers bounded
congestion, priority/fairness, expiration, signed ordering, stop/restart,
disconnect and 40 parser creations. It requires no FC or radio.

QOpenHD's `tests/run_hud_latency_test.py` against a Windows Qt release build
covers firmware identity before heartbeat, simultaneous Air/Ground RSSI,
CRC recovery, GUI-thread delivery, menu loading and retired-connection events.
