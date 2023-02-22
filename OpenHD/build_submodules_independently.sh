# bin/bash

# For the github CIs
# Here we build all the submodules independently from each other to make sure
# no unwanted dependencies were introduced by accident during OpenHD development
cd ohd_common || exit
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug

cd ../ohd_interface || exit
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug

cd ../ohd_telemetry || exit
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug

cd ../ohd_video || exit
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug
