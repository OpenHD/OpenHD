cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Discovery)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# include the general openhd stuff
include_directories(../ohd_common)

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
# Changed according to https://cliutils.gitlab.io/modern-cmake/chapters/packages/Boost.html
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS regex filesystem)
SET(OHD_SYSTEM_USED_LIBRARIES Boost::regex Boost::filesystem)

# find and include all the other libs
include(cmake/FindLibNL.cmake)
include_directories(${LibNL_INCLUDE_DIR})
SET(OHD_SYSTEM_USED_LIBRARIES ${OHD_SYSTEM_USED_LIBRARIES} ${LibNL_LIBRARIES})
#
include(cmake/FindLibUSB.cmake)
include_directories(${LIBUSB_1_INCLUDE_DIRS})
SET(OHD_SYSTEM_USED_LIBRARIES ${OHD_SYSTEM_USED_LIBRARIES} ${LIBUSB_1_LIBRARIES})
#
include(cmake/FindV4L2.cmake)
include_directories(${V4L2_INCLUDE_DIR})
SET(OHD_SYSTEM_USED_LIBRARIES ${OHD_SYSTEM_USED_LIBRARIES} ${V4L2_LIBRARY})

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
target_link_libraries(OHDDiscoveryLib ${OHD_SYSTEM_USED_LIBRARIES})

# The main service that is run on OpenHD
add_executable(test_discovery test/test_discovery.cpp)
target_link_libraries(test_discovery OHDDiscoveryLib)