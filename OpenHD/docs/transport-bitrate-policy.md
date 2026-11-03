# Camera bitrate by transport

All capacity providers use the common video budget router and camera allocator.
Only routed video links participate. When identical RTP streams are replicated,
the lowest available budget wins; independent link capacities are never added.
Removed links stop constraining the encoders. A late update from a removed link
is ignored.

WiFiBroadcast supplies its existing MCS/FEC-derived payload budget. It reserves
10% of that budget for headroom, in addition to fixed/essential reservations.
Camera targets are held between MCS/FEC, camera-share, camera-availability or
encoder-limit changes. Live telemetry/audio traffic updates the measurements
without continually retuning the encoders; reservations refresh on the next
configuration change. Artosyn
supplies its existing modem estimate. Ethernet uses negotiated port speed and
peer probes without substituting an encoder ceiling for reported link capacity.
The shared allocator limits the adjustable encoder pool to **50,000 kbit/s total**
after reserving fixed and essential traffic. Per-camera capability limits still
apply, and unused link capacity stays available for other traffic.
When a camera reaches its maximum, its unused share is reassigned to the other
adjustable camera. The BCM2835 CSI encoder is limited to its advertised
25,000 kbit/s driver maximum. The software thermal encoder can use the remaining
pool, without the previous generic 20,000 kbit/s restriction.
LTE and Microhard start conservatively and
use end-to-end probe RTT, probe loss and local send errors to back off and slowly
recover toward the same ceiling. Current TX throughput is not treated as spare
capacity. These estimates are congestion signals, not a WAN speed test.

The common allocator reserves essential traffic, fixed camera traffic, audio
and overhead before splitting the remaining pool using `V_PRIMARY_PERC`. Encoder
hardware limits still apply. `V_AUTO_BITRATE` controls automatic allocation for
all transports. Turning it off restores saved per-camera bitrates. Existing
WiFiBroadcast manual/automatic choice is migrated on first use, and its legacy
`VARIABLE_BITRATE` control remains an alias. Automatic updates do not overwrite
the saved manual bitrate.

LTE now carries the original primary and secondary RTP streams. It does not
create the previous 854x480 secondary encoder. The FleetControl permission lease
is required for every uploaded stream; denial or expiry stops its upload.
Matching lease replies also provide probe timing feedback. This does not change
the WireGuard or permission protocol.

A Ground LTE gateway forwards both original streams and reports a short-lived
budget to Air using MAVLink DATA96 type `0x52`: `OHBR`, version 1, three reserved
bytes, then a little-endian uint32 kbit/s budget. Air accepts this only from the
Ground onboard-computer component, with a 64..50,000 range. Reports expire after
three seconds, so a stopped relay cannot permanently lower the local video
budget. This uses the same trust boundary as existing Ground camera controls.

OHLU allocation (`BWA1`) now applies to every video transport. Ethernet emits
allocation and metrics as separate DATA96 packets so both fit the existing
payload. `ETH1` optionally appends negotiated duplex (0 half, 1 full, UINT32_MAX
unknown) after its existing eight fields. Older receivers ignore the extension.
QOpenHD combines the valid complementary trailers; truncated packets and legacy
packets do not retain unsupported fields.

The Ethernet dashboard plots a bounded 30-second TX/RX history, with gaps for
missing measurements. It shows actual port speed/duplex and probe RTT/jitter,
probe loss and local send errors. The Total Link Usage popup is available for
every transport.

Unit tests cover capacity reductions, congestion backoff, gradual recovery,
route switching, replicated links, relay budget expiry, permission denial,
authenticated RTP forwarding, and old/new telemetry decoding. Live Ethernet
validation does not establish performance on a cellular or Microhard radio link.
