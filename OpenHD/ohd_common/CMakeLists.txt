cmake_minimum_required(VERSION 3.16.3)
project(OHDCommon)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

if (TARGET OHDCommonLib)
    message(STATUS "OHDCommonLib already built")
    return()
endif()

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)

add_library(OHDCommonLib STATIC) # initialized below
add_library(OHDCommonLib::OHDCommonLib ALIAS OHDCommonLib)

find_package(spdlog REQUIRED)
target_link_libraries(OHDCommonLib PRIVATE spdlog::spdlog)

#----------------------------------------------------------------------------------------------------------------------
# sources
#----------------------------------------------------------------------------------------------------------------------

set(sources
    "mavlink_settings/ISettingsComponent.hpp"
    "json.hpp"
    "include_json.hpp"
    "openhd-action-handler.hpp"
    "openhd-common-tests.hpp"
    "openhd-external-device.hpp"
    "openhd-global-constants.hpp"
    "openhd-led-codes.hpp"
    "openhd-led-pi.hpp"
    "openhd-link-statistics.hpp"
     "openhd-platform.hpp"
    "openhd-platform-discover.hpp"
    "openhd-platform-json.hpp"
    "openhd-profile.hpp"
    "openhd-profile-json.hpp"
    "openhd-settings-directories.hpp"
    "openhd-settings-persistent.hpp"
    "openhd-util.hpp"
    "openhd-util-filesystem.hpp"
    "openhd-spdlog.hpp"
    "openhd-spdlog-tele-sink.h"
    "openhd-udp-log.h"
    )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

#----------------------------------------------------------------------------------------------------------------------
# target
#----------------------------------------------------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

target_sources(OHDCommonLib PRIVATE ${sources})

target_include_directories(OHDCommonLib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mavlink_settings>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>")
