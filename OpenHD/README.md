## Summary

This subdirectory contains all the code that is needed to build the OpenHD executable
that is then run on the air and ground platform to create OpenHD.
Note that this executable assumes that some modifications have been applied to the underlying linux OS,
like patched wifi drivers and installed linux utility programs.

## List of Assumptions about the linux system we are running on that are non-standard:
(This might be incomplete)
1) Wifi drivers are patched to support monitor mode for all WiFi cards OpenHD supports
(Or at least the wifi card you then connect to the system and that is used by OpenHD for wifibroadcast)
This sounds simple, but it is not. Also, stuff like disabling weird services that would interfer with monitor
mode or similar falls into this category.
2) The directory /tmp for writing temporary files exists
3) The directory "SETTINGS_BASE_PATH" for writing persistent (setting) files exists / can be created by OpenHD and is read/writeable
(Make sure it is read / writeable on embedded devices)
4) The directory /home/openhd/Videos for storing video recording(s) on the air unit can be created by OpenHD
5) UART: if the platform has an UART connector (like rpi GPIO), the UART is enabled and has a corresponding linux file handle


## Note about connected Hardware:
While it is nice for the user to Hotplug new Hardware, this is not always feasible. For example,to allow easier debugging, we discover connected
cameras on startup if running as air and if no camera is found, emulate the primary camera in SW.
Similar to interface, where we detect the connected wifi cards at startup.
This can be generally described by having a discovery step at startup - once the discovery step has
been performed, we cannot check for changes on this discovered hardware. However, hardware that needs
to be hot-pluggable (for example the FC) in general uses a different pattern - check in regular
intervals if the HW configuration has changed, then react to these changes

