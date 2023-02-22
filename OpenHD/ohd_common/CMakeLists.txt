cmake_minimum_required(VERSION 3.18.3)
project(OHDCommon
        VERSION 2.0.0
        DESCRIPTION "OpenHD common headers"
        LANGUAGES CXX)

if (TARGET OHDCommonLib)
    message(STATUS "OHDCommonLib already built")
    return()
endif()

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)

add_library(OHDCommonLib STATIC) # initialized below
add_library(OHDCommonLib::OHDCommonLib ALIAS OHDCommonLib)

# We use spdlog and json pretty much everywhere in OpenHD. Build it here, then make publicly available
add_subdirectory(lib/spdlog)
add_subdirectory(lib/json)

# Public since we use it throughout OpenHD
#find_package(spdlog REQUIRED)
target_link_libraries(OHDCommonLib PUBLIC spdlog::spdlog)

# Public since we use it throughout OpenHD
#find_package(spdlog nlohmann_json)
target_link_libraries(OHDCommonLib PUBLIC  nlohmann_json::nlohmann_json)

# Find and include boost. We only use boost for the filesystem stuff (since std::filesystem is a bitch in regards to compatibility).
# Please try and avoid using boost throughout the project !
# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Changed according to https://cliutils.gitlab.io/modern-cmake/chapters/packages/Boost.html
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(OHDCommonLib PUBLIC Boost::filesystem)
#----------------------------------------------------------------------------------------------------------------------
# sources
#----------------------------------------------------------------------------------------------------------------------

set(sources
    "src/openhd_util.cpp"
    "src/openhd_util_filesystem.cpp"
    "src/openhd_settings_persistent.cpp"
    "src/openhd_profile.cpp"
    "src/openhd_platform.cpp"
    "src/openhd_spdlog.cpp"
    "src/openhd_udp_log.cpp"
    "src/openhd_reboot_util.cpp"
    "src/openhd_config.cpp"

    "inc/openhd_settings_imp.hpp"
    "inc/include_json.hpp"
    "inc/openhd_action_handler.hpp"
    "inc/openhd_external_device.hpp"
    "inc/openhd_global_constants.hpp"
    "inc/openhd_led_codes.hpp"
    "inc/openhd_led_pi.hpp"
    "inc/openhd_link_statistics.hpp"
    "inc/openhd_platform.h"
    "inc/openhd_profile.h"
    "inc/openhd_settings_directories.hpp"
    "inc/openhd_settings_persistent.h"
    "inc/openhd_util.h"
    "inc/openhd_util_filesystem.h"
    "inc/openhd_spdlog.h"
    "inc/openhd_udp_log.h"
    "inc/openhd_reboot_util.h"
    "lib/ini/ini.hpp"
    "inc/openhd_config.h"
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

add_executable(test_ohd_reboot test/test_ohd_reboot.cpp)
target_link_libraries(test_ohd_reboot OHDCommonLib)

add_executable(test_config test/test_config.cpp)
target_link_libraries(test_config OHDCommonLib)