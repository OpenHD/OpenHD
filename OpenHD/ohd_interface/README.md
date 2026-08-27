# Summary

This submodule is responsible for starting and configuring all the OpenHD
interfaces - aka all OpenHD links like wifibroadcast (communication between air and ground),
ground hotspot, ...

OpenHD routes traffic through a `MultiLink` facade. On standard Wi-Fi setups,
wifibroadcast and IP/Ethernet are active simultaneously: each encoded RTP
packet and telemetry packet is submitted to both transports. The ground facade
forwards the first received RTP packet and suppresses copies arriving through
the other transport. A transport can therefore disappear without changing the
camera or encoder pipeline.

Each transport has its own worker and bounded queues. Encoded buffers remain
shared (there is no video payload copy), and a blocked transport can only fill
its own queue. Video is capped at two pending frames per camera stream, so a
failed or slow route drops stale frames locally instead of adding continuously
growing latency or blocking the encoder and healthy routes. Telemetry queues
retain RC packets preferentially under congestion.

When any active WFB card disappears or injection reports `ENXIO`, WFB is
quarantined without replacing the `WBLink` object. A background supervisor
matches the replugged adapter by MAC address, restores its original interface
name and monitor mode, and retries every two seconds. The existing `WBTxRx` instance
then reopens its pcap/raw-socket handles in place, reapplies channel and power
settings, and rejoins `MultiLink`. Camera, encoder, telemetry, settings
callbacks, Ethernet, and other transports stay alive throughout recovery.

Telemetry follows the same rule in both directions. Wifibroadcast may still
honor its per-packet `n_injections` reliability hint, while Ethernet sends one
UDP datagram for the same logical packet. Duplicate packets received through
different links are removed before MAVLink parsing.

When no wifibroadcast, Artosyn, or Microhard link is detected, the same
IP/Ethernet transport remains available by itself. Air listens for discovery
requests on UDP port `49891`. Ground probes every active local IPv4 network
using both directed broadcasts and bounded unicast subnet scans; the air
response supplies its IP address and the video and telemetry UDP ports. Both
sides then configure the existing Ethernet video and telemetry forwarding path.
Discovery continues in the background so DHCP address changes and reconnections
are handled without restarting OpenHD.

Automatic Ethernet uses dedicated link ports `5910` for video and `5920` for
telemetry, avoiding OpenHD's local video/client ports. An explicit
`ethernet.txt` configuration may override them.

If a connected Ethernet interface has no usable DHCP address after a short
grace period, OpenHD configures a direct-link `/24`: air uses `192.168.8.1` and
ground uses `192.168.8.2`. These addresses are applied at runtime and allow a
plain Ethernet cable or an unmanaged switch to work without a DHCP server.

An explicit `ethernet.txt` configuration remains authoritative and bypasses
automatic discovery, but it no longer disables a detected wifibroadcast link.

Note that some modules handle HW connection(s) themselves, for example telemetry does the UART
connection to the FC (even though one could reason UART is a HW interface).

## Created WB links for openhd-telemetry and openhd-video:
1) Bidirectional link made up of 2 wifibroadcast instances for telemetry up / down, both on air and ground
2) 2 Unidirectional links for video down from air pi to ground pi (primary and secondary video stream)
   -> NOTE: Video only goes from air pi to ground pi, so we need 2 tx instances on air pi and 2 rx instances on ground
   pi

look at the code to find out the ports usw, they should come from openhd-constants.hpp

## Video Crypto (.so)
Video encryption is not available in the community build. Licensed builds may
load a proprietary video crypto library (.so) at runtime. For licensing, contact
license@openhdfpv.com.
