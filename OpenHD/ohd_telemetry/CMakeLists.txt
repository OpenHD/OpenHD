cmake_minimum_required(VERSION 3.16.3)
project(OHDTelemetryLib)

set(CMAKE_CXX_STANDARD 17)

add_library(OHDTelemetryLib STATIC) # initialized below
add_library(OHDTelemetryLib::OHDTelemetryLib ALIAS OHDTelemetryLib)

# Only needed when building this submodule manually
add_subdirectory(../ohd_common commonlib EXCLUDE_FROM_ALL)

find_package(Threads REQUIRED)
target_link_libraries(OHDTelemetryLib PUBLIC Threads::Threads)

#find_package(spdlog REQUIRED)
#target_link_libraries(OHDTelemetryLib PRIVATE spdlog::spdlog)

# For joystick, optional (but if SDL2 is not found at compile time, RC won't be available at run time)
find_package(SDL2 QUIET)
if(SDL2_FOUND)
    message(STATUS "SDL2 found")
    target_compile_definitions(OHDTelemetryLib PUBLIC OPENHD_TELEMETRY_SDL_FOR_JOYSTICK_FOUND)
    target_include_directories(OHDTelemetryLib PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(OHDTelemetryLib PRIVATE ${SDL2_LIBRARIES})
else()
    message(STATUS "SDL Features (Joystick) will be unavailable!")
endif()

SET(sources
    "src/endpoints/MEndpoint.cpp"
    "src/endpoints/MEndpoint.h"
    "src/endpoints/SerialEndpoint.cpp"
    "src/endpoints/SerialEndpoint.h"
    "src/endpoints/UDPEndpoint.cpp"
    "src/endpoints/UDPEndpoint.h"
    "src/endpoints/WBEndpoint.cpp"
    "src/endpoints/WBEndpoint.h"

    "src/internal/LogCustomOHDMessages.hpp"
    "src/internal/OHDLinkStatisticsHelper.h"
    "src/internal/OHDMainComponent.cpp"
    "src/internal/OHDMainComponent.h"
    "src/internal/onboard_computer_status.hpp"
    "src/internal/onboard_computer_status_rpi.hpp"
    "src/internal/OnboardComputerStatusProvider.cpp"
    "src/internal/OnboardComputerStatusProvider.h"
        src/last_known_position/LastKnowPosition.cpp
     src/last_known_position/LastKnowPosition.h

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
    src/gpio_control/RaspberryPiGPIOControl.cpp
    src/gpio_control/RaspberryPiGPIOControl.h
    src/gpio_control/RaspberryPiGPIOControlSettings.h
    src/gpio_control/RaspberryPiGPIOControlSettings.cpp
    src/internal/ina219.cc
    src/internal/ina219.h
    src/endpoints/TCPEndpoint.cpp
    src/endpoints/TCPEndpoint.h
    src/GroundTelemetrySettings.cpp
    src/AirTelemetrySettings.cpp
)

# Build it as a library
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})
target_sources(OHDTelemetryLib PRIVATE ${sources})
target_include_directories(OHDTelemetryLib 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_link_libraries(OHDTelemetryLib PUBLIC OHDCommonLib)

# include mavlink (header-only)
add_library(mavlink INTERFACE)
# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
target_compile_options(mavlink INTERFACE -Wno-address-of-packed-member -Wno-cast-align)
target_include_directories(mavlink INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/mavlink-headers/mavlink/v2.0)
target_link_libraries(OHDTelemetryLib PUBLIC mavlink)

# All the test files for development
add_executable(test_serial_endpoint test/test_serial_endpoint.cpp)
target_link_libraries(test_serial_endpoint OHDTelemetryLib)

add_executable(test_udp_endpoint test/test_udp_endpoint.cpp)
target_link_libraries(test_udp_endpoint OHDTelemetryLib)

add_executable(test_tcp_server_endpoint test/test_tcp_server_endpoint.cpp)
target_link_libraries(test_tcp_server_endpoint OHDTelemetryLib)

add_executable(test_ground_locally test/test_ground_locally.cpp)
target_link_libraries(test_ground_locally OHDTelemetryLib)

add_executable(test_air_locally test/test_air_locally.cpp)
target_link_libraries(test_air_locally OHDTelemetryLib)

add_executable(test_ground_and_air_locally test/test_ground_and_air_locally.cpp)
target_link_libraries(test_ground_and_air_locally OHDTelemetryLib)

add_executable(test_onboard_computer_status_read_stuff test/test_onboard_computer_status_read_stuff.cpp)
target_link_libraries(test_onboard_computer_status_read_stuff OHDTelemetryLib)

add_executable(test_joystick_reader test/test_joystick_reader.cpp)
target_link_libraries(test_joystick_reader OHDTelemetryLib)

####
# NOTE: We do not need MAVSDK for OpenHD, the small amount of code we share is directly included
####