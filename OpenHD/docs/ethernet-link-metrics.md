# Ethernet link metrics

Ethernet publishes an optional extension of the existing DATA96/OHLU v1 usage
message. No video/telemetry datagram format or Ethernet opt-in/routing policy changes.
Old QOpenHD builds still read the six usage categories. New QOpenHD builds display
unknown metrics as N/A when connected to older OpenHD builds.

## Measurements

The message is emitted by Air: TX is Air to Ground; RX is Ground to Air.

- TX/RX throughput: successful OpenHD UDP payload bytes per elapsed steady-clock
  interval, sampled every 500 ms. Includes the Ethernet video multiplexing header;
  excludes UDP/IP/Ethernet framing and unrelated interface traffic. Rates are measured
  offered/received traffic, not an active bandwidth benchmark or proof of delivery.
- TX/RX packets per second: actual successful sends/received datagrams.
- Port speed: negotiated speed read from `/sys/class/net/<route-interface>/speed`,
  after resolving the local source address of a route to the configured peer. A
  missing carrier, loopback, or unsupported speed leaves capacity unknown. This is
  local port speed, not an end-to-end path-capacity measurement.
- RTT: one small UDP echo from Air to the configured Ground peer on port 49892,
  normally once per second, with a 1500 ms timeout. The responder accepts only the
  configured peer. Separate sockets and a bounded thread keep probes out of video
  queues. Firewall rules must allow this port for diagnostics, but transport works
  without it. No raw sockets or ICMP privileges are needed.
- RTT jitter: EWMA of successive absolute RTT differences, weight 1/4 for new samples.
- Probe loss: failed deadlines in the last 60 completed probes. Unknown until the
  peer has replied at least once; it is **not** video or telemetry packet loss.
  RTT/jitter expire after three seconds without a reply. Changing peer resets the window.
- Send errors: cumulative failed UDP send calls during the EthernetLink lifetime.
  Failed datagrams are excluded from TX throughput and category accounting.

Ethernet metrics publish even with WiFiBroadcast active. The existing legacy
WiFi-shaped compatibility snapshot is still left to WiFiBroadcast in that case.
Additional useful values are the per-category traffic breakdown and TX utilization
against port speed (already available in Link Usage). Measuring video packet loss
would require sequence tracking per stream; a general MAVLink ping is unsuitable
for Ethernet RTT because it may take another active transport.

## Wire layout

The 46-byte OHLU v1 header and six category tuples remain unchanged. At offset 46,
`ETH1` identifies eight little-endian uint32 fields at offsets 50 through 78:
`tx_bps`, `rx_bps`, `tx_pps`, `rx_pps`, `rtt_us`, `jitter_us`,
`probe_loss_permille`, `send_errors`. Payload length is 82. UINT32_MAX means unknown
for extension fields; capacity zero retains its existing unknown meaning.

## Validation

`test_ethernet_metrics` tests rate scaling/saturation, the rolling probe window,
registry lifetime, an actual localhost UDP responder, and a responder outage.
QOpenHD's `tests/run_ethernet_observations_test.py` checks its production model's
parsing, old payloads, unknown sentinels, truncation, and stale-sample expiry.
`tests/ethernet_link` checks the actual card with live and unavailable values on Qt 5/6.
These tests do not establish performance or loss on a physical Ethernet pair.
