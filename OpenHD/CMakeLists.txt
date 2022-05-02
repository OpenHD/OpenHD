cmake_minimum_required(VERSION 3.16.3)
project(OpenHD)

add_subdirectory(lib)

add_subdirectory(ohd_interface)
add_subdirectory(ohd_system)
#add_subdirectory(ohd_telemetry)
add_subdirectory(ohd_video)

add_executable(OpenHD main.cpp)
#target_link_libraries(OpenHD)