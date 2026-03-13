cmake_minimum_required(VERSION 3.16.3)
project(COHDMods)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(COHDMods STATIC
    serial/serial.cpp
    serial/serial.h
)
add_library(COHDMods::COHDMods ALIAS COHDMods)

target_include_directories(COHDMods PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
