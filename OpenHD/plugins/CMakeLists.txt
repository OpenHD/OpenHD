cmake_minimum_required(VERSION 3.16.3)
project(OHDPlugins)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(OHDPlugins INTERFACE)

add_subdirectory(walksnail)

target_include_directories(OHDPlugins INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_library(OHDPlugins::OHDPlugins ALIAS OHDPlugins)
