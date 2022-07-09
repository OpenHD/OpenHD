cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Telemetry)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# include the general openhd stuff
include_directories(../ohd_common)

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
# Changed according to https://cliutils.gitlab.io/modern-cmake/chapters/packages/Boost.html
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
find_package(Boost REQUIRED COMPONENTS system thread regex filesystem)
message(STATUS "Boost version: ${Boost_VERSION}")
SET(OHD_TELEMETRY_USED_BOOST_LIBS Boost::system Boost::thread Boost::filesystem)

SET(ALL_TELEMETRY_SOURCE_FILES
        src/AirTelemetry.cpp
        src/GroundTelemetry.cpp
        src/endpoints/UDPEndpoint.cpp
        src/endpoints/UDPEndpoint2.cpp
        src/endpoints/TCPEndpoint.cpp
        src/endpoints/SerialEndpoint.cpp
        #src/endpoints/SerialEndpoint2.cpp
        #src/endpoints/WBEndpoint.cpp
        src/internal/OHDMainComponent.cpp
        src/mavsdk_param/XMavlinkParamProvider.cpp
        # temporarily until mavsdk becomes usable
        src/mavsdk_param/mavlink_parameter_set.cpp
        src/mavsdk_param/mavlink_parameter_subscription.cpp
        src/mavsdk_param/param_value.cpp
        src/mavsdk_param/mavlink_parameter_receiver.cpp
        src/mavsdk_param/mavlink_message_handler.cpp
        ## end param
        src/mavsdk_param/connection.cpp
        src/mavsdk_param/connection_result.cpp
        src/mavsdk_param/serial_connection.cpp
        src/mavsdk_param/mavlink_receiver.cpp
        src/mavsdk_param/mavsdk_time.cpp
        src/mavsdk_param/mavlink_channels.cpp
        )

# Build it as a library
add_library(OHDTelemetryLib
        STATIC
        ${ALL_TELEMETRY_SOURCE_FILES})
target_link_libraries(OHDTelemetryLib PRIVATE ${OHD_TELEMETRY_USED_BOOST_LIBS}
        #MAVSDK::mavsdk
        )
target_include_directories(OHDTelemetryLib PUBLIC src/)
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

###
find_package(MAVSDK)
if(MAVSDK_FOUND)
    add_executable(test_mavsdk_serial tests/test_mavsdk_serial.cpp)
    target_link_libraries(test_mavsdk_serial OHDTelemetryLib MAVSDK::mavsdk)

    add_executable(test_mavsdk_param tests/test_mavsdk_param.cpp)
    target_link_libraries(test_mavsdk_param OHDTelemetryLib MAVSDK::mavsdk)
endif()
#####