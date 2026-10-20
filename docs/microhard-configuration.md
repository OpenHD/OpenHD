# Microhard link configuration (OpenHD 3.0)

Microhard transports video and bidirectional telemetry between two OpenHD
hosts. Configure the values through SysUtils on both hosts, then restart
OpenHD. Settings are read at link construction; this is not a live reload.

| SysUtils key | Meaning |
| --- | --- |
| `air_unit_ip` | IPv4 address of the Air **OpenHD host**. Ground sends telemetry here. |
| `ground_unit_ip` | IPv4 address of the Ground **OpenHD host**. Air sends video and telemetry here. |
| `microhard_ip_air` | Management IPv4 address of the modem attached to Air. |
| `microhard_ip_ground` | Management IPv4 address of the modem attached to Ground. |
| `microhard_ip_range` | IPv4 prefix used to find a modem in the local default routes when its management address is empty; defaults to `192.168.168`. Example: `10.20.30`. |
| `microhard_video_port` | Video UDP port, identical on both hosts; default `5910`. |
| `microhard_telemetry_port` | Telemetry UDP port, identical on both hosts; default `5920`. |
| `microhard_username` / `microhard_password` | Local modem login credentials. |

For example, hosts `192.168.168.10` (Air) and `192.168.168.20` (Ground)
can communicate through modems managed at `192.168.168.1` (Air) and
`192.168.168.2` (Ground). Set `air_unit_ip` to `.10` and `ground_unit_ip`
to `.20` on both hosts; the modem management addresses must not replace
these host addresses. Both hosts need IP connectivity through the modems.

UDP receivers bind to `0.0.0.0` on their configured ports. The link never
uses interface enumeration to guess the remote host. A missing/invalid peer
host address or invalid/colliding UDP ports is rejected before sockets or
monitoring workers start. An empty modem address uses gateway discovery;
if no matching gateway exists, data transport starts without modem monitoring.

Credentials from SysUtils are used literally, including spaces, quotes, and
semicolons. Do not add the quoting or terminators used by the legacy
`hardware.config` format. Line breaks and NUL bytes are rejected, and login
values are not written to logs.

The old implementation ignored these Microhard settings and hardcoded video
port `5001`, telemetry port `5000`, and the default credentials. Configure
both ends together when moving to the corrected implementation.
