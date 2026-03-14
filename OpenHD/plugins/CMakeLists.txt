cmake_minimum_required(VERSION 3.16.3)
project(OHDPlugins)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(walksnail)

add_library(OHDPlugins::OHDPlugins ALIAS OHDPlugins)

target_include_directories(COHDMods PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
