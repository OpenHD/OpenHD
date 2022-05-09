cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Video)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# include the general openhd stuff
include_directories(../ohd_common)

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


SET(ALL_VIDEO_SOURCE_FILES
        src/camerastream.cpp
        src/flironestream.cpp
        src/gstreamerstream.cpp
        src/libcamerastream.cpp
        src/seekstream.cpp
        src/OHDVideo.cpp)

add_library(OHDVideoLib
        SHARED
        ${ALL_VIDEO_SOURCE_FILES})
target_include_directories(OHDVideoLib PUBLIC inc/)
target_link_libraries(OHDVideoLib ${OHD_VIDEO_USED_LIBS})
target_include_directories(OHDVideoLib PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(OHDVideoLib ${GST_LIBRARIES})

# Executable for testing manually, note that this one needs generated jsons
add_executable(openhd_video test/main.cpp)
target_link_libraries(openhd_video OHDVideoLib)

#target_include_directories(openhd_video PRIVATE ${GST_INCLUDE_DIRS})
#target_link_libraries(openhd_video ${GST_LIBRARIES})