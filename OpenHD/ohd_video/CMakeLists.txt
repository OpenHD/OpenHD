cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Video)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# include the general openhd stuff
include_directories(../ohd_common)

# local includes
include_directories(inc)

# include mavlink (header-only)
include_directories(../lib/c_library_v2_openhd)

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread regex filesystem)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

SET(OHD_VIDEO_USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

# FUCKING GSTREAMER !! begin
find_package(PkgConfig REQUIRED)
pkg_search_module(GST REQUIRED gstreamer-1.0>=1.4
        gstreamer-sdp-1.0>=1.4
        gstreamer-video-1.0>=1.4
        gstreamer-app-1.0>=1.4)
# FUCKING GSTREAMER !! end
find_package(fmt)
SET(OHD_VIDEO_USED_LIBS ${OHD_VIDEO_USED_LIBS} fmt::fmt)

# TODO make nice
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lsystemd")

SET(ALL_VIDEO_SOURCE_FILES
        src/cameramicroservice.cpp
        src/camerastream.cpp
        src/control.cpp
        src/flironestream.cpp
        src/gstreamerstream.cpp
        src/hotspot.cpp
        src/mavlinkcontrol.cpp
        src/seekstream.cpp
        src/OHDVideo.cpp)

#add_library(OHDVideoLib
#        SHARED
#        ${ALL_})
#target_link_libraries(OHDInterfaceLib ${INTERFACE_USED_BOOST_LIBRARIES})

# The main service that is run on OpenHD
add_executable(openhd_video src/main.cpp
        ${ALL_VIDEO_SOURCE_FILES})
target_link_libraries(openhd_video ${OHD_VIDEO_USED_LIBS})

target_include_directories(openhd_video PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(openhd_video ${GST_LIBRARIES})