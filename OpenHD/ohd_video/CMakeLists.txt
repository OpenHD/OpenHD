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
find_package(PkgConfig)
pkg_search_module(GLIB REQUIRED glib-2.0)
#include(cmake/FindGstreamer.cmake)
#include_directories(${GSTREAMER_INCLUDE_DIR})
#SET(USED_LIBS ${USED_LIBS} ${GSTREAMER_LIBRARY})
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)
SET(OHD_VIDEO_USED_LIBS ${OHD_VIDEO_USED_LIBS} ${GST_APP_LIBRARIES} ${GST_VIDEO_LIBRARIES})
include_directories(${GSTREAMER_INCLUDE_DIR})
link_directories(${GSTLIBS_LIBRARY_DIRS})
include_directories(
        /usr/lib/x86_64-linux-gnu/glib-2.0/include
        /usr/include/glib-2.0
        /usr/local/include/gstreamer-1.0
        /usr/local/include/libsoup-2.4
        /usr/local/include/json-glib-1.0)
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
        src/seekstream.cpp)

# The main service that is run on OpenHD
add_executable(openhd_video src/main.cpp
        ${ALL_VIDEO_SOURCE_FILES})
target_link_libraries(openhd_video ${OHD_VIDEO_USED_LIBS})