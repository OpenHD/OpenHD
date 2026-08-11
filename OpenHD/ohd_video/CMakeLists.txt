cmake_minimum_required(VERSION 3.16.3)
project(OHDVideoLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If we build this module standalone for testing, always support air -
# otherwise, testing kinda doesn't make sense. The option might already be set to off
# by the OpenHD main executable CMakeLists.txt file - in which case this does nothing
# Same for managed cameras support
option(ENABLE_AIR "Enable air support" ON)
option(ENABLE_USB_CAMERAS "USB cameras support" ON)
option(ENABLE_LIBCAMERA "libcamera support" ON)
option(OPENHD_MPP_ONLY "Build the RV1126 MPP video path without GStreamer" OFF)
# The MPP-only image intentionally has no USB/GStreamer camera stack. USB
# discovery also links libv4l2, which is not part of the minimal runtime image.
if(OPENHD_MPP_ONLY)
    set(ENABLE_USB_CAMERAS OFF CACHE BOOL "USB cameras support" FORCE)
endif()
find_package(PkgConfig REQUIRED)

find_program(PYTHON_EXECUTABLE NAMES python3 python)
if(NOT PYTHON_EXECUTABLE)
    message(FATAL_ERROR "python3 or python is required to generate camera registry")
endif()

set(CAMERA_REGISTRY_JSON ${CMAKE_CURRENT_SOURCE_DIR}/misc/camera_registry.json)
set(GENERATED_CAMERA_REGISTRY_HEADER ${CMAKE_CURRENT_BINARY_DIR}/camera_registry_generated.h)

add_custom_command(
    OUTPUT ${GENERATED_CAMERA_REGISTRY_HEADER}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_camera_registry.py
            --input ${CAMERA_REGISTRY_JSON}
            --output ${GENERATED_CAMERA_REGISTRY_HEADER}
    DEPENDS ${CAMERA_REGISTRY_JSON} ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_camera_registry.py
    COMMENT "Generating camera registry from JSON"
    VERBATIM
)

add_custom_target(generate_camera_registry DEPENDS ${GENERATED_CAMERA_REGISTRY_HEADER})

add_library(OHDVideoLib STATIC
        inc/rpi_hdmi_to_csi_v4l2_helper.h
        src/openhd_rtp.cpp
        inc/openhd_rtp.h) # initialized below
add_library(OHDVideoLib::OHDVideoLib ALIAS OHDVideoLib)

# Only needed when building this submodule manually
add_subdirectory(../ohd_common commonlib EXCLUDE_FROM_ALL)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

set(sources
    src/ohd_video_ground.cpp
    #src/gst_recorder.cpp
    #src/gst_recording_demuxer.cpp
)

list(APPEND sources
        librtp/src/rtp-h264-pack.c
        librtp/src/rtp-h265-pack.c
        librtp/src/rtp-packet.c
        librtp/src/rtp-payload.c
        librtp/src/rtp-h264-bitstream.c
)

if(ENABLE_AIR)
    # Only if air support is enabled, link all the air-only related libraries and
    # build the part(s) required for air
    list(APPEND sources
            src/camerastream.cpp
            src/camera_discovery.cpp
            src/ip_camera_network.cpp
            src/ohd_video_air.cpp
            src/rtp_eof_helper.cpp
            src/camera_holder.cpp
            src/ohd_video_air_generic_settings.cpp
            src/validate_settings.cpp
            src/usb_thermal_cam_helper.cpp
    )

    if(NOT OPENHD_MPP_ONLY)
        list(APPEND sources src/gstreamerstream.cpp src/gstaudiostream.cpp)
        pkg_search_module(GST REQUIRED
                gstreamer-1.0>=1.4
                gstreamer-app-1.0>=1.4)
        pkg_search_module(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.4)
        pkg_search_module(gstreamer-app REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.4)
        target_link_libraries(OHDVideoLib PRIVATE PkgConfig::gstreamer PkgConfig::gstreamer-app)
    else()
        target_compile_definitions(OHDVideoLib PUBLIC OPENHD_MPP_ONLY)
    endif()

    # Native RV1126(B) encoder. Set ROCKCHIP_MPP_INCLUDE_DIR explicitly when
    # cross compiling against a runtime-only sysroot (which often contains the
    # library but not the development headers).
    find_path(ROCKCHIP_MPP_INCLUDE_DIR rk_mpi.h
              PATH_SUFFIXES rockchip mpp)
    find_library(ROCKCHIP_MPP_LIBRARY NAMES rockchip_mpp)
    if(ROCKCHIP_MPP_INCLUDE_DIR AND ROCKCHIP_MPP_LIBRARY)
        message(STATUS "Native Rockchip MPP encoder enabled")
        list(APPEND sources src/rockchip_mpp_stream.cpp)
        target_include_directories(OHDVideoLib PRIVATE ${ROCKCHIP_MPP_INCLUDE_DIR})
        target_link_libraries(OHDVideoLib PRIVATE ${ROCKCHIP_MPP_LIBRARY})
        target_compile_definitions(OHDVideoLib PUBLIC OPENHD_ROCKCHIP_MPP_PRESENT)
    else()
        if(OPENHD_MPP_ONLY)
            message(FATAL_ERROR "OPENHD_MPP_ONLY requires Rockchip MPP")
        endif()
        message(STATUS "Native Rockchip MPP encoder unavailable; RV1126 uses GStreamer")
    endif()
    # From libcamera-apps !! begin
    find_package(PkgConfig REQUIRED)
    if(ENABLE_LIBCAMERA)
        pkg_check_modules(LIBCAMERA QUIET libcamera)
        if(LIBCAMERA_FOUND)
            message(STATUS "libcamera library found:")
            message(STATUS "    version: ${LIBCAMERA_VERSION}")
            message(STATUS "    libraries: ${LIBCAMERA_LINK_LIBRARIES}")
            message(STATUS "    include path: ${LIBCAMERA_INCLUDE_DIRS}")
        else()
            message(STATUS "LIBCAMERA features will be unavailable!")
        endif()
    else()
        message(STATUS "libcamera support disabled")
    endif()
    # From libcamera-apps !! end
    if(ENABLE_LIBCAMERA AND LIBCAMERA_FOUND)
        target_include_directories(OHDVideoLib PRIVATE ${LIBCAMERA_INCLUDE_DIRS})
        target_link_libraries(OHDVideoLib PRIVATE ${LIBCAMERA_LINK_LIBRARIES})
        target_compile_definitions(OHDVideoLib PUBLIC OPENHD_LIBCAMERA_PRESENT)
    endif()
endif ()

if (ENABLE_AIR AND ENABLE_USB_CAMERAS AND NOT OPENHD_MPP_ONLY)
    # FUCKING GSTREAMER !! begin
    pkg_search_module(GST REQUIRED
            gstreamer-sdp-1.0>=1.4
            gstreamer-video-1.0>=1.4)
    # https://stackoverflow.com/questions/64078308/how-to-write-a-cmake-file-which-successfully-links-gstreamer-basic-tutorial-5
    pkg_search_module(gstreamer-sdp REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.4)
    pkg_search_module(gstreamer-video REQUIRED IMPORTED_TARGET gstreamer-video-1.0>=1.4)
    target_link_libraries(OHDVideoLib PUBLIC
            PkgConfig::gstreamer-sdp
            PkgConfig::gstreamer-video)
    # FUCKING GSTREAMER !! end

    include(cmake/FindLibUSB.cmake)
    if(LIBUSB_1_FOUND)
        target_include_directories(OHDVideoLib PRIVATE ${LIBUSB_1_INCLUDE_DIRS})
        target_link_libraries(OHDVideoLib PRIVATE ${LIBUSB_1_LIBRARIES})
        target_compile_definitions(OHDVideoLib PUBLIC OPENHD_LIBUSB_PRESENT)
    else()
        message("LIBUSB not found, thermal cameras not available")
    endif ()

    include(cmake/FindV4L2.cmake)
    target_include_directories(OHDVideoLib PRIVATE ${V4L2_INCLUDE_DIR})
    target_link_libraries(OHDVideoLib PRIVATE ${V4L2_LIBRARY})
endif ()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})
target_sources(OHDVideoLib PRIVATE ${sources})
target_include_directories(OHDVideoLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_include_directories(OHDVideoLib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(OHDVideoLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/librtp/inc)

target_link_libraries(OHDVideoLib PUBLIC OHDCommonLib)

# disable compiler warning since we don't link against GCC before 4.4
target_compile_options(OHDVideoLib PRIVATE -Wno-packed-bitfield-compat)

find_package(Threads REQUIRED)
target_link_libraries(OHDVideoLib PUBLIC Threads::Threads)

add_dependencies(OHDVideoLib generate_camera_registry)

# Executables for testing manually, note that some might need to run after discovery
add_executable(test_video test/test_video.cpp)
target_link_libraries(test_video OHDVideoLib)
add_executable(test_audio test/test_audio.cpp)
target_link_libraries(test_audio OHDVideoLib)
add_executable(openhd_lan_test test/openhd_lan_test.cpp)
target_link_libraries(openhd_lan_test OHDVideoLib)
