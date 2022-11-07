cmake_minimum_required(VERSION 3.16.3)
project(OHDTelemetryLib)

set(CMAKE_CXX_STANDARD 17)

add_library(OHDTelemetryLib STATIC) # initialized below
add_library(OHDTelemetryLib::OHDTelemetryLib ALIAS OHDTelemetryLib)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
# Changed according to https://cliutils.gitlab.io/modern-cmake/chapters/packages/Boost.html
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
find_package(Boost REQUIRED COMPONENTS filesystem)
message(STATUS "Boost version: ${Boost_VERSION}")
SET(OHD_TELEMETRY_USED_BOOST_LIBS Boost::filesystem)

find_package(Threads REQUIRED)

# For joystick
find_package(SDL2)
if(SDL2_FOUND)
    message(STATUS "SDL2 found")
    target_compile_definitions(OHDTelemetryLib PRIVATE OPENHD_SDL_FOR_JOYSTICK_FOUND)
    target_include_directories(OHDTelemetryLib PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(OHDTelemetryLib PRIVATE ${SDL2_LIBRARIES})
else()
    message(STATUS "SDL Features (Joystick) will be unavailable!")
endif()

SET(sources
    "src/endpoints/MEndpoint.hpp"
    "src/endpoints/SerialEndpoint.cpp"
    "src/endpoints/SerialEndpoint.h"
    "src/endpoints/UDPEndpoint.cpp"
    "src/endpoints/UDPEndpoint.h"
    "src/endpoints/UDPEndpoint2.cpp"
    "src/endpoints/UDPEndpoint2.h"

    "src/internal/LogCustomOHDMessages.hpp"
    "src/internal/OHDLinkStatisticsHelper.h"
    "src/internal/OHDMainComponent.cpp"
    "src/internal/OHDMainComponent.h"
    "src/internal/OnboardComputerStatus.hpp"
    "src/internal/OnboardComputerStatusProvider.cpp"
    "src/internal/OnboardComputerStatusProvider.h"
    "src/internal/RebootUtil.hpp"
    "src/internal/StatusTextAccumulator.hpp"

    "src/mavsdk_temporary/connection.cpp"
    "src/mavsdk_temporary/connection.h"
    "src/mavsdk_temporary/connection_result.cpp"
    "src/mavsdk_temporary/connection_result.h"
    "src/mavsdk_temporary/locked_queue.h"
    "src/mavsdk_temporary/log.h"
    "src/mavsdk_temporary/mavlink_channels.cpp"
    "src/mavsdk_temporary/mavlink_channels.h"
    "src/mavsdk_temporary/mavlink_include.h"
    "src/mavsdk_temporary/mavlink_message_handler.cpp"
    "src/mavsdk_temporary/mavlink_message_handler.h"
    "src/mavsdk_temporary/mavlink_parameter_receiver.cpp"
    "src/mavsdk_temporary/mavlink_parameter_receiver.h"
    "src/mavsdk_temporary/mavlink_parameter_set.cpp"
    "src/mavsdk_temporary/mavlink_parameter_set.h"
    "src/mavsdk_temporary/mavlink_parameter_subscription.cpp"
    "src/mavsdk_temporary/mavlink_parameter_subscription.h"
    "src/mavsdk_temporary/mavlink_receiver.cpp"
    "src/mavsdk_temporary/mavlink_receiver.h"
    "src/mavsdk_temporary/mavsdk_time.cpp"
    "src/mavsdk_temporary/mavsdk_time.h"
    "src/mavsdk_temporary/param_value.cpp"
    "src/mavsdk_temporary/param_value.h"
    "src/mavsdk_temporary/sender.h"
    "src/mavsdk_temporary/sender_wrapper.h"
    "src/mavsdk_temporary/XMavlinkParamProvider.cpp"
    "src/mavsdk_temporary/XMavlinkParamProvider.h"
    
    "src/rc/JoystickReader.cpp"
    "src/rc/JoystickReader.h"
    "src/rc/RcJoystickSender.cpp"
    "src/rc/RcJoystickSender.h"

    "src/routing/MavlinkComponent.hpp"
    "src/routing/MavlinkSystem.hpp"

    "src/AirTelemetry.cpp"
    "src/AirTelemetry.h"
    "src/AirTelemetrySettings.h"
    "src/GroundTelemetry.cpp"
    "src/GroundTelemetry.h"
    "src/mav_helper.h"
    "src/mav_include.h"
    "src/MavlinkComponent.h"
    "src/OHDTelemetry.cpp"
    "src/OHDTelemetry.h"
)

# Build it as a library
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})
target_sources(OHDTelemetryLib PRIVATE ${sources})
target_include_directories(OHDTelemetryLib 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)


target_link_libraries(OHDTelemetryLib PRIVATE ${OHD_TELEMETRY_USED_BOOST_LIBS}
        #MAVSDK::mavsdk
        )
target_link_libraries(OHDTelemetryLib PUBLIC Threads::Threads)

find_package(spdlog REQUIRED)
target_link_libraries(OHDTelemetryLib PRIVATE spdlog::spdlog)

# include mavlink (header-only)
add_library(mavlink INTERFACE)
target_include_directories(mavlink INTERFACE ../lib/c_library_v2_openhd/)
target_link_libraries(OHDTelemetryLib PUBLIC mavlink)
#target_include_directories(OHDTelemetryLib PUBLIC ../lib/c_library_v2_openhd)

# Note: We do not need wifibroadcast as a whole here, we can get the header-only files.
target_include_directories(OHDTelemetryLib PUBLIC ../lib/wifibroadcast/src)

# All the test files for development
add_executable(test_serial_endpoint tests/test_serial_endpoint.cpp)
target_link_libraries(test_serial_endpoint OHDTelemetryLib)

add_executable(test_udp_endpoint tests/test_udp_endpoint.cpp)
target_link_libraries(test_udp_endpoint OHDTelemetryLib)

add_executable(test_ohd_reboot tests/test_ohd_reboot.cpp)
target_link_libraries(test_ohd_reboot OHDTelemetryLib)

add_executable(test_generate_log_message tests/test_generate_log_message.cpp)
target_link_libraries(test_generate_log_message OHDTelemetryLib)

add_executable(test_ground_locally tests/test_ground_locally.cpp)
target_link_libraries(test_ground_locally OHDTelemetryLib)

add_executable(test_air_locally tests/test_air_locally.cpp)
target_link_libraries(test_air_locally OHDTelemetryLib)

add_executable(test_ground_and_air_locally tests/test_ground_and_air_locally.cpp)
target_link_libraries(test_ground_and_air_locally OHDTelemetryLib)

add_executable(test_onboard_computer_status_read_stuff tests/test_onboard_computer_status_read_stuff.cpp)
target_link_libraries(test_onboard_computer_status_read_stuff OHDTelemetryLib)

add_executable(test_joystick_reader tests/test_joystick_reader.cpp)
target_link_libraries(test_joystick_reader OHDTelemetryLib)

####
# NOTE: We do not need MAVSDK for OpenHD, we only compile some test tools here if
# MAVSDK would be found
find_package(MAVSDK QUIET)
if(MAVSDK_FOUND)
    add_executable(test_mavsdk_serial tests/test_mavsdk_serial.cpp)
    target_link_libraries(test_mavsdk_serial OHDTelemetryLib MAVSDK::mavsdk)

    add_executable(test_mavsdk_param tests/test_mavsdk_param.cpp)
    target_link_libraries(test_mavsdk_param OHDTelemetryLib MAVSDK::mavsdk)
endif()
#####