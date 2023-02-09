cmake_minimum_required(VERSION 3.16.3)
project(OHDCommon
        VERSION 2.0.0
        DESCRIPTION "OpenHD common headers"
        LANGUAGES CXX)

if (TARGET OHDCommonLib)
    message(STATUS "OHDCommonLib already built")
    return()
endif()

# We use spdlog pretty much everyhwere in OpenHD
add_subdirectory(lib/json)

add_subdirectory(lib/spdlog)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)

add_library(OHDCommonLib STATIC) # initialized below
add_library(OHDCommonLib::OHDCommonLib ALIAS OHDCommonLib)

# Public since we use it throughout OpenHD
#find_package(spdlog REQUIRED)
target_link_libraries(OHDCommonLib PUBLIC spdlog::spdlog)

# Public since we use it throughout OpenHD
#find_package(spdlog nlohmann_json)
target_link_libraries(OHDCommonLib PUBLIC  nlohmann_json::nlohmann_json)

#----------------------------------------------------------------------------------------------------------------------
# sources
#----------------------------------------------------------------------------------------------------------------------

set(sources
    "inc/mavlink_settings/ISettingsComponent.hpp"
    "inc/include_json.hpp"
    "inc/openhd-action-handler.hpp"
    "inc/openhd-common-tests.hpp"
    "inc/openhd-external-device.hpp"
    "inc/openhd-global-constants.hpp"
    "inc/openhd-led-codes.hpp"
    "inc/openhd-led-pi.hpp"
    "inc/openhd-link-statistics.hpp"
    "inc/openhd-platform.hpp"
    "inc/openhd-platform-discover.hpp"
    "inc/openhd-platform-json.hpp"
    "inc/openhd-profile.hpp"
    "inc/openhd-profile-json.hpp"
    "inc/openhd-settings-directories.hpp"
    "inc/openhd-settings-persistent.hpp"
    "inc/openhd-util.hpp"
    "inc/openhd-util-filesystem.hpp"
    "inc/openhd-spdlog.hpp"
    "inc/openhd-spdlog-tele-sink.h"
    "inc/openhd-udp-log.h"
    )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

#----------------------------------------------------------------------------------------------------------------------
# target
#----------------------------------------------------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

target_sources(OHDCommonLib PRIVATE ${sources})

target_include_directories(OHDCommonLib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/mavlink_settings>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/>")

set_target_properties(OHDCommonLib PROPERTIES
        SOVERSION ${PROJECT_VERSION_MAJOR}
        VERSION ${PROJECT_VERSION}
        LINKER_LANGUAGE CXX)