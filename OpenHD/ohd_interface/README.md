# Summary

This submodule is responsible for starting and configuring all the OpenHD
interfaces - aka all OpenHD links like wifibroadcast (communication between air and ground),
ground hotspot, ...

When no wifibroadcast, Artosyn, or Microhard link is detected, OpenHD
automatically falls back to an IP/Ethernet link. Air listens for discovery
requests on UDP port `49891`. Ground probes every active local IPv4 network
using both directed broadcasts and bounded unicast subnet scans; the air
response supplies its IP address and the video and telemetry UDP ports. Both
sides then configure the existing Ethernet video and telemetry forwarding path.
Discovery continues in the background so DHCP address changes and reconnections
are handled without restarting OpenHD.

If a connected Ethernet interface has no usable DHCP address after a short
grace period, OpenHD configures a direct-link `/24`: air uses `192.168.8.1` and
ground uses `192.168.8.2`. These addresses are applied at runtime and allow a
plain Ethernet cable or an unmanaged switch to work without a DHCP server.

An explicit `ethernet.txt` configuration remains authoritative and bypasses
automatic discovery.

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
