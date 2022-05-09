cmake_minimum_required(VERSION 3.16.3)
project(OpenHD)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(ohd_interface)
add_subdirectory(ohd_system)
add_subdirectory(ohd_telemetry)
add_subdirectory(ohd_video)

# Here we build the single executable that makes up OpenHD.
# It depends on all the libraries we have just added above
include_directories(ohd_common)
add_executable(OpenHD main.cpp)
target_link_libraries(OpenHD OHDInterfaceLib OHDSystemLib OHDVideoLib OHDTelemetryLib)