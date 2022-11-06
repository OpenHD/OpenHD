Any repositories / code that are non platform dependent and also have their own repositories under the
OpenHD namespace should be added here as git submodules.
Note: Drivers or similar should not end up here, as well as libraries that can simly be installed by apt-get
(well maintained libraries)

# Regarding mavlink:
This repository contains our custom mavlink fork as a submdoule. To add new messages, just modify the mavlink source xml,
then run the generate_custom_mavlink.sh script. Note that pymavlink is a submodule of mavlink itself, so you might have
to init the submodule itself when you cloned OpenHD without the --recurse-submodules option.

# Regarding wifibroadcast:
One can build and link against wifibroadcast itself, see the CmakeLists.txt here.

