# MAVLink definition ownership

OpenHD/mavlink `message_definitions/v1.0/openhd.xml` owns the wire contract.
OpenHD/mavlink-headers contains generated headers, client descriptors and
`source.json` with immutable XML/generator revisions. OpenHD and QOpenHD pin
the headers as submodules. FleetControl, WebUI and Glide keep generated client
descriptors, synchronized by `scripts/sync_openhd_wire.py` in the XML repository.
Do not add independent CRC tables, custom message structs or payload offsets.
The eight internal statistics types and their telemetry conversions are also
generated from XML. `ohd_common` uses plain generated fields without MAVLink
transport headers; application-only state stays in its local wrappers.

OpenHD commands use `MAV_CMD_OPENHD_*` constants generated from the MAV_CMD
enum. Their numeric values are unchanged; the duplicate OPENHD_CMD enum and
unused dummy commands were removed. Settings remain standard PARAM_EXT plus
MAVLink FTP metadata. ImageWriter retains offline initial JSON configuration;
SysUtils retains its local privileged interface. Neither consumes these wire
definitions directly.

## Upgrade order

Install the new OpenHD receiver before the new QOpenHD sender. Message 1231 had
a manually entered CRC 177; canonical generation produces 178. OpenHD's
`legacy_radio_settings.h` verifies old unsigned CRC-177 frames for this exact
message, then normalizes them for forwarding. Other bad CRCs remain rejected.
Old OpenHD receivers do not understand the new CRC. All other published message
CRCs, lengths and field offsets are unchanged. The compatibility test covers
all 322 messages inherited by this dialect.

The upstream-only XML patch is on OpenHD/mavlink `upstream/openhd-dialect`.
It generated successfully against upstream all.xml. The deployed IDs still
require allocation approval; this branch is for review, not an accepted
upstream standard. See [upstream preparation](https://github.com/OpenHD/mavlink/blob/openhd/docs/openhd-upstream.md).
