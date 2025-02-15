cmake_minimum_required(VERSION 3.16.3)
project(OHDCommon)

if (TARGET OHDCommonLib)
    message(STATUS "OHDCommonLib already built")
    return()
endif()

#----------------------------------------------------------------------------------------------------------------------
# General settings and options
#----------------------------------------------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress warnings related to nlohmann::json on some compilers
add_compile_options(-Wno-psabi)

add_library(OHDCommonLib STATIC) # Initialized below
add_library(OHDCommonLib::OHDCommonLib ALIAS OHDCommonLib)

#----------------------------------------------------------------------------------------------------------------------
# Explicitly link stdc++fs for GCC 8.x compatibility with std::filesystem
#----------------------------------------------------------------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(OHDCommonLib PRIVATE stdc++fs)
endif()

#----------------------------------------------------------------------------------------------------------------------
# Dependencies: spdlog and nlohmann::json
#----------------------------------------------------------------------------------------------------------------------
set(SPDLOG_MASTER_PROJECT OFF)
set(SPDLOG_PROJECT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/spdlog)
set(SPDLOG_SOURCES_DIRECTORY ${SPDLOG_PROJECT_DIRECTORY}/src)

set(spdlog_sources
    "${SPDLOG_SOURCES_DIRECTORY}/bundled_fmtlib_format.cpp"
)

target_precompile_headers(OHDCommonLib PRIVATE ${SPDLOG_PROJECT_DIRECTORY}/include/spdlog/spdlog.h)
target_include_directories(OHDCommonLib PUBLIC ${SPDLOG_PROJECT_DIRECTORY}/include)

# Add nlohmann::json
add_subdirectory(lib/json)
target_link_libraries(OHDCommonLib PUBLIC nlohmann_json::nlohmann_json)

# Add pthread support
find_package(Threads REQUIRED)
target_link_libraries(OHDCommonLib PUBLIC Threads::Threads)

#----------------------------------------------------------------------------------------------------------------------
# Sources
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
    "src/openhd_action_handler.cpp"
    "src/openhd_udp.cpp"
    "src/openhd_tcp.cpp"
    "src/openhd_led.cpp"
    "src/openhd_buttons.cpp"
    "src/openhd_settings_imp.cpp"
    "src/openhd_settings_directories.cpp"
    "src/openhd_settings_persistent.cpp"
    "src/openhd_util_time.cpp"
    "src/openhd_bitrate.cpp"
    "src/openhd_thermal.cpp"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

#----------------------------------------------------------------------------------------------------------------------
# Target configuration
#----------------------------------------------------------------------------------------------------------------------
target_sources(OHDCommonLib PRIVATE ${sources})
target_include_directories(OHDCommonLib PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc/>")

set_target_properties(OHDCommonLib PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
    LINKER_LANGUAGE CXX
)

#----------------------------------------------------------------------------------------------------------------------
# Test Executables
#----------------------------------------------------------------------------------------------------------------------
add_executable(test_openhd_util test/test_openhd_util.cpp)
target_link_libraries(test_openhd_util OHDCommonLib)

add_executable(test_ohd_reboot test/test_ohd_reboot.cpp)
target_link_libraries(test_ohd_reboot OHDCommonLib)

add_executable(test_config test/test_config.cpp)
target_link_libraries(test_config OHDCommonLib)

add_executable(test_logging test/test_logging.cpp)
target_link_libraries(test_logging OHDCommonLib)

# Not part of the main interface but useful for debugging
add_executable(test_led test/test_led.cpp)
target_link_libraries(test_led OHDCommonLib)

add_executable(test_openhd_async test/test_openhd_async.cpp)
target_link_libraries(test_openhd_async OHDCommonLib)

add_executable(test_tcp_server test/test_tcp_server.cpp)
target_link_libraries(test_tcp_server OHDCommonLib)
