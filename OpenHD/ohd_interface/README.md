This module is responsible for starting and configuring all the OpenHD
interfaces - aka all OpenHD links like wifibroadcast (between aur and ground), ground hotspot, ...

It has to be started after the discovery step.

## For now, this module creates the following wifibroadcast links, since they are needed by openhd-telemetry and openhd-video:
1) Bidirectional link made up of 2 wifibroadcast instances for telemetry up / down, both on air and ground
2) 2 Unidirectional links for video down from air pi to ground pi (primary and secondary video stream)
   -> NOTE: Video only goes from air pi to ground pi, so we need 2 tx instances on air pi and 2 rx instances on ground
   pi

look at the code to find out the ports usw, they should come from openhd-constants.hpp