cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_CXX_STANDARD 17)

# find and build wifibroacast
include(wifibroadcast/WBLib.cmake)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)
# include mavlink (header-only)
include_directories(c_library_v2_openhd)