cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-System)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# include the general openhd stuff
include_directories(../ohd_common)

include_directories(inc)
include_directories(lib/include)

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread regex filesystem)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

SET(OHD_SYSTEM_USED_LIBRARIES ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

# find and include  all the libs
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

# TODO make nice
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lsystemd")

SET(ALL_SYSTEM_SOURCE_FILES
        src/DCameras.cpp
        src/DEthernetCards.cpp
        src/nl.c
        src/DPlatform.cpp
        src/DProfile.cpp
        src/DWifiCards.cpp
        src/OHDSystem.cpp
        lib/bcm2835.c)

add_library(OHDSystemLib
        SHARED
        ${ALL_SYSTEM_SOURCE_FILES})
target_link_libraries(OHDSystemLib ${OHD_SYSTEM_USED_LIBRARIES})

# The main service that is run on OpenHD
add_executable(test_discovery test/test_discovery.cpp)
target_link_libraries(test_discovery OHDSystemLib)