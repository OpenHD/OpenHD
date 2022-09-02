These files are header-only and used in various other modules. I do not think they have any dependencies other than c++
standard libraries and (rarely) libboost.

NOTE: Code here is available to all the different OHD modules (e.g. ohd_telemetry,ohd_video, ohd_interface). The modules itself
are not allowed to depend on each other (e.g. ohd_telemetry does not depend on ohd_interface). This enforces a clear separation between them,
which makes testing much easier and allows independent contribution(s) to each of the modules.
In general, code here shouldn't have any platform-specific dependencies.

