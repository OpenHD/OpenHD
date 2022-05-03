cmake_minimum_required(VERSION 3.16.3)
project(OpenHD)

add_subdirectory(lib)

add_subdirectory(ohd_interface)
add_subdirectory(ohd_system)
#add_subdirectory(ohd_telemetry)
add_subdirectory(ohd_video)

#include_directories(ohd_common)
#include_directories(ohd_interface/inc)
#include_directories(ohd_system/inc)
#include_directories(ohd_video/inc)

#add_executable(OpenHD main.cpp)
#target_link_libraries(OpenHD OHDInterfaceLib OHDSystemLib OHDVideoLib)