# Ground details telemetry

The status worker samples every five seconds, separately from MAVLink output.
DATA96 type 0x47 carries a 76-byte OHDG version 1 snapshot. Existing clients
ignore this optional message. No link, hotspot or storage configuration is changed.

Byte 4 is version 1. Byte 5 flags valid system storage (1), reserved legacy log storage (2),
station count (4), hotspot IPv4 (8), and public IPv4 (16). Bytes 6-7 are reserved.
A little-endian uint32 station count is at 8. Little-endian uint64 byte counts
are at 12/20 (system used/total) and 28/36 (reserved legacy log storage, no longer measured).
NUL-padded IPv4 strings of 16 bytes are at 44 and 60. Unflagged fields are unknown.
The QOpenHD model replaces snapshots and expires them after 15 seconds.

Storage uses statvfs on the system filesystem: used is total minus free blocks.
Log storage is not collected or displayed.
Hotspot interface discovery uses the actual iw AP interface, station dump counts
associated clients, and getifaddrs supplies its IPv4. Failures are unknown, not zero.
Public IPv4 is the egress/NAT address from an HTTPS api.ipify.org request, bounded
to two seconds, cached for five minutes. Missing curl, no connectivity, an invalid
response or lookup failure produces unknown. IPv6 is not reported by this version.
The lookup contacts a third-party service; it is not a guess from a local address.

RAM remains the legacy OpenHD percentage plus MemTotal in KiB. QOpenHD converts
that unit for display; it does not synthesize a RAM capacity.
