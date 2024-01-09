cmake_minimum_required(VERSION 3.16.3)
project(OHDCommon)

if (TARGET OHDCommonLib)
    message(STATUS "OHDCommonLib already built")
    return()
endif()

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(OHDCommonLib STATIC) # initialized below
add_library(OHDCommonLib::OHDCommonLib ALIAS OHDCommonLib)

# We use spdlog and json pretty much everywhere in OpenHD. Build it here, then make publicly available
# 1) spdlog
#find_package(spdlog REQUIRED)
add_subdirectory(lib/spdlog)
# Public since we use it throughout OpenHD
target_link_libraries(OHDCommonLib PUBLIC spdlog::spdlog)

# 2) nlohmann::json
add_subdirectory(lib/json)
# Public since we use it throughout OpenHD
target_link_libraries(OHDCommonLib PUBLIC  nlohmann_json::nlohmann_json)

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
    "src/openhd_reboot_util.cpp"
    "src/openhd_config.cpp"
        "src/openhd_util_async.cpp"
    "src/openhd_external_device.cpp"
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

add_executable(test_logging test/test_logging.cpp)
target_link_libraries(test_logging OHDCommonLib)