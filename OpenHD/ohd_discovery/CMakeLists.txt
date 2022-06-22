cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Discovery)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# include the general openhd stuff
include_directories(../ohd_common)

SET(ALL_SYSTEM_SOURCE_FILES
        #src/DEthernetCards.cpp
        src/DPlatform.cpp
        src/DProfile.cpp
        src/OHDDiscovery.cpp)

add_library(OHDDiscoveryLib
        STATIC
        ${ALL_SYSTEM_SOURCE_FILES})
target_include_directories(OHDDiscoveryLib PUBLIC inc/)
target_include_directories(OHDDiscoveryLib PUBLIC lib/include)

# find and link all the external libraries

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
# Changed according to https://cliutils.gitlab.io/modern-cmake/chapters/packages/Boost.html
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(OHDDiscoveryLib PRIVATE Boost::filesystem)


# Test the discovery step on its own
add_executable(test_discovery test/test_discovery.cpp)
target_link_libraries(test_discovery OHDDiscoveryLib)