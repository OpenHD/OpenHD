cmake_minimum_required(VERSION 3.16.3)
project(OpenHD)

set(CMAKE_CXX_STANDARD 17)

link_libraries(stdc++fs)

add_subdirectory(ohd_common)
add_subdirectory(ohd_interface)
add_subdirectory(ohd_telemetry)
add_subdirectory(ohd_video)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# Here we build the single executable that makes up OpenHD.
# It depends on all the libraries we have just added above
include_directories(ohd_common)
add_executable(OpenHD main.cpp)
target_link_libraries(OHDInterfaceLib PUBLIC OHDCommonLib)
target_link_libraries(OHDTelemetryLib PUBLIC OHDCommonLib)
target_link_libraries(OHDVideoLib PUBLIC OHDCommonLib)
target_link_libraries(OpenHD OHDInterfaceLib OHDVideoLib OHDTelemetryLib OHDCommonLib)

# NOTE: We only install the OpenHD executable, the test_xxx execuatables are omitted
# For development, just clone and build OpenHD on the pi / platform itself
install(TARGETS OpenHD DESTINATION bin)