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
# Changed according to https://cliutils.gitlab.io/modern-cmake/chapters/packages/Boost.html
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem)
SET(OHD_VIDEO_USED_LIBS Boost::filesystem)

# FUCKING GSTREAMER !! begin
find_package(PkgConfig REQUIRED)
pkg_search_module(GST REQUIRED gstreamer-1.0>=1.4
        gstreamer-sdp-1.0>=1.4
        gstreamer-video-1.0>=1.4
        gstreamer-app-1.0>=1.4)
# FUCKING GSTREAMER !! end
find_package(fmt REQUIRED)
SET(OHD_VIDEO_USED_LIBS ${OHD_VIDEO_USED_LIBS} fmt::fmt)

SET(ALL_VIDEO_SOURCE_FILES
        src/camerastream.cpp
        src/gstreamerstream.cpp
        src/libcamerastream.cpp
        src/OHDVideo.cpp
        src/DCameras.cpp)

add_library(OHDVideoLib
        STATIC
        ${ALL_VIDEO_SOURCE_FILES})
target_include_directories(OHDVideoLib PUBLIC inc/)
target_link_libraries(OHDVideoLib PUBLIC ${OHD_VIDEO_USED_LIBS})
# link and include gstreamer
target_include_directories(OHDVideoLib PUBLIC ${GST_INCLUDE_DIRS})
target_link_libraries(OHDVideoLib PUBLIC ${GST_LIBRARIES})
# find and include all the other libs
#
include(cmake/FindLibUSB.cmake)
target_include_directories(OHDVideoLib PRIVATE ${LIBUSB_1_INCLUDE_DIRS})
target_link_libraries(OHDVideoLib PRIVATE ${LIBUSB_1_LIBRARIES})
#
include(cmake/FindV4L2.cmake)
target_include_directories(OHDVideoLib PRIVATE ${V4L2_INCLUDE_DIR})
target_link_libraries(OHDVideoLib PRIVATE ${V4L2_LIBRARY})

# Executables for testing manually, note that some might need to run after discovery
add_executable(test_video test/test_video.cpp)
target_link_libraries(test_video OHDVideoLib)
add_executable(test_dummy_gstreamer test/test_dummy_gstreamer.cpp)
target_link_libraries(test_dummy_gstreamer OHDVideoLib)