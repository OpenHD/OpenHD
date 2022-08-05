## Summary

This subdirectory contains all the code that is needed to build the OpenHD executable
that is then run on the air and ground platform to create OpenHD.
Note that this executable assumes that some modifications have been applied to the underlying linux OS,
like patched wifi drivers and directories to write files into.

## List of Assumptions about the linux system we are running on that are non-standard:
1) Wifi drivers are patched to support monitor mode for all WiFi cards OpenHD supports
This sounds simple, but it is not. Also, stuff like disabling weird services that would interfer with monitor
mode or similar falls into this category.
2) The directory /tmp for writing temporary files exists
3) The directory /usr/local/share/openhd/ for writing persistent (setting) files exists / can be created 
4) The directory XXX for writing video recordings to exists TODO which one
5) UART: if the platform has an UART connector (like rpi GPIO), the UART is enabled and has a corresponding linux file handle


## Note about connected Hardware:
While it is nice for the user to Hotplug new Hardware, this is not always feasible. For example,
to find out if we should run as air or ground pi, we use the n of connected cameras and then never
change this configuration. Similar to interface, where we detect the connected wifi cards at startup.
This can be generally described by having a discovery step at startup - once the discovery step has
been performed, we cannot check for changes on this discovered hardware. However, hardware that needs
to be hot-pluggable (for example the FC) in general uses a different pattern - check in regular
intervals if the HW configuration has changed, then react to these changes

