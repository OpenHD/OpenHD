## Summary

This subdirectory contains all the code that is needed to build the OpenHD executable
that is then run on the air and ground platform to create OpenHD.
Note that this executable assumes that some modifications have been applied to the underlying linux OS,
like patched wifi drivers and directories to write files into.

## List of Assumptions about the linux system we are running on that are non-standard:
1) Wifi drivers are patched to support monitor mode for all WiFi cards OpenHD supports
2) The directory /tmp for writing temporary files exists
3) The directory XXX for writing persistent files exists TODO which one
4) UART: if the platform has an UART connector (like rpi GPIO), the UART is enabled and it is implemented
    in the "find UART linux filename for platform" method.


