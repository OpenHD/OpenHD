cmake_minimum_required(VERSION 3.16.3)
project(OpenHD)

add_subdirectory(ohd_interface)
add_subdirectory(ohd_system)
add_subdirectory(ohd_telemetry)
add_subdirectory(ohd_video)

# TODO: make it a single executable

#add_executable(OpenHD main.cpp)
#target_link_libraries(OpenHD OHDInterfaceLib OHDSystemLib OHDVideoLib)
#target_include_directories(OpenHD PRIVATE ohd_common)
#target_include_directories(OpenHD PRIVATE ohd_interface/inc)
#target_include_directories(OpenHD PRIVATE ohd_system/inc)
#target_include_directories(OpenHD PRIVATE ohd_video/inc)
#target_include_directories(OpenHD PRIVATE ohd_telemetry/src)