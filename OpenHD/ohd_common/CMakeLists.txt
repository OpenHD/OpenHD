cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Common
    VERSION 2.0.0
    DESCRIPTION "OpenHD common heades"
    LANGUAGES CXX)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

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
    "openhd-log.hpp"
    "openhd-platform.hpp"
    "openhd-platform-discover.hpp"
    "openhd-profile.hpp"
    "openhd-settings.hpp"
    "openhd-settings2.hpp"
    "openhd-util.hpp"
    "openhd-util-filesystem.hpp"
    "veye-helper.hpp"
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

set_target_properties(OHDCommonLib PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
    LINKER_LANGUAGE CXX)
