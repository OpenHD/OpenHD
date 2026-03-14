cmake_minimum_required(VERSION 3.16.3)
project(OHDPluginWalksnail)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(OHDPluginWalksnail STATIC
    WalksnailBridge.h
    WalksnailBridge.cpp
    serial/serial.cpp
    serial/serial.h
)

target_include_directories(OHDPluginWalksnail PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
