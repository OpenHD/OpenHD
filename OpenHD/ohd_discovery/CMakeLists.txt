cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Discovery)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# include the general openhd stuff
include_directories(../ohd_common)

SET(ALL_SYSTEM_SOURCE_FILES
        src/DCameras.cpp
        #src/DEthernetCards.cpp
        src/DPlatform.cpp
        src/DProfile.cpp
        src/DWifiCards.cpp
        src/OHDDiscovery.cpp
        lib/nl.c
        lib/bcm2835.c)

add_library(OHDDiscoveryLib
        SHARED
        ${ALL_SYSTEM_SOURCE_FILES})
target_include_directories(OHDDiscoveryLib PUBLIC inc/)
target_include_directories(OHDDiscoveryLib PUBLIC lib/include)

# find and link all the external libraries

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
# Changed according to https://cliutils.gitlab.io/modern-cmake/chapters/packages/Boost.html
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS regex filesystem)
target_link_libraries(OHDDiscoveryLib PUBLIC Boost::regex Boost::filesystem)

# find and include all the other libs
include(cmake/FindLibNL.cmake)
target_include_directories(OHDDiscoveryLib PUBLIC ${LibNL_INCLUDE_DIR})
target_link_libraries(OHDDiscoveryLib PUBLIC ${LibNL_LIBRARIES})
#
include(cmake/FindLibUSB.cmake)
target_include_directories(OHDDiscoveryLib PUBLIC ${LIBUSB_1_INCLUDE_DIRS})
target_link_libraries(OHDDiscoveryLib PUBLIC ${LIBUSB_1_LIBRARIES})
#
include(cmake/FindV4L2.cmake)
target_include_directories(OHDDiscoveryLib PUBLIC ${V4L2_INCLUDE_DIR})
target_link_libraries(OHDDiscoveryLib PUBLIC ${V4L2_LIBRARY})

# Test the discovery step on its own
add_executable(test_discovery test/test_discovery.cpp)
target_link_libraries(test_discovery OHDDiscoveryLib)