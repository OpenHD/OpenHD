cmake_minimum_required(VERSION 3.16.3)
project(OHDPluginWalksnail)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(OHDPluginWalksnail STATIC
    WalksnailGround.h
    WalksnailGround.cpp
    WalksnailUtils.h
    serial/serial.h
    serial/serial.cpp
)

target_include_directories(OHDPluginWalksnail PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(OHDPluginWalksnail PRIVATE OHDTelemetryLib)

target_link_libraries(OHDPlugins INTERFACE OHDPluginWalksnail)