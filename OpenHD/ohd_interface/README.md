# Summary

This submodule is responsible for starting and configuring all the OpenHD
interfaces - aka all OpenHD links like wifibroadcast (communication between air and ground),
ground hotspot, ...

For now, it requires at least one wifi card connected to the system - this might change in the 
future when we add other hardware types for data communication between ground and air pi
like LTE cards.

Note that some modules handle HW connection(s) themselves, for example telemetry does the UART
connection to the FC (even though one could reason UART is a HW interface).

## Created WB links for openhd-telemetry and openhd-video:
1) Bidirectional link made up of 2 wifibroadcast instances for telemetry up / down, both on air and ground
2) 2 Unidirectional links for video down from air pi to ground pi (primary and secondary video stream)
   -> NOTE: Video only goes from air pi to ground pi, so we need 2 tx instances on air pi and 2 rx instances on ground
   pi

look at the code to find out the ports usw, they should come from openhd-constants.hpp

## Keys
The purpose of encryption is less about security and more about avoiding packet collisions.
R.n encryption is mandatory for wifibroadcast, but to make development easier, we just use a default seed
to create the tx and rx files. We are in the progress in changing that, The OpenHD image writer already allows
the user to set a bind phrase.

You can use [wifibroadcast repo](https://github.com/openhd/wifibroadcast) to generate a key pair. The keys should be placed in $SETTINGS_BASE_PATH$/interface
