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
    "inc/openhd_settings_imp.hpp"
    "inc/include_json.hpp"
    "inc/openhd_action_handler.hpp"
    "inc/openhd_common_tests.hpp"
    "inc/openhd_external_device.hpp"
    "inc/openhd_global_constants.hpp"
    "inc/openhd_led_codes.hpp"
    "inc/openhd_led_pi.hpp"
    "inc/openhd_link_statistics.hpp"
    "inc/openhd_platform.hpp"
    "inc/openhd_platform_discover.hpp"
    "inc/openhd_platform_json.hpp"
    "inc/openhd_profile.hpp"
    "inc/openhd_profile_json.hpp"
    "inc/openhd_settings_directories.hpp"
    "inc/openhd_settings_persistent.hpp"
    "inc/openhd_util.hpp"
    "inc/openhd_util_filesystem.hpp"
    "inc/openhd_spdlog.hpp"
    "inc/openhd_spdlog_tele_sink.h"
    "inc/openhd_udp_log.h"
    )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

#----------------------------------------------------------------------------------------------------------------------
# target
#----------------------------------------------------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

target_sources(OHDCommonLib PRIVATE ${sources})

target_include_directories(OHDCommonLib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/>")

set_target_properties(OHDCommonLib PROPERTIES
        SOVERSION ${PROJECT_VERSION_MAJOR}
        VERSION ${PROJECT_VERSION}
        LINKER_LANGUAGE CXX)

add_executable(test_openhd_util test/test_openhd_util.cpp)
target_link_libraries(test_openhd_util OHDCommonLib)