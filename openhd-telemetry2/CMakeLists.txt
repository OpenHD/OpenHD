cmake_minimum_required(VERSION 3.16.3)
project(XMavlinkService)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# Build and include wifibroadcast
include(lib/wifibroadcast/WBLib.cmake)

# include mavlink (header-only)
include_directories(lib/c_library_v2_openhd)

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread regex filesystem)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${WB_TARGET_LINK_LIBRARIES})

SET(ALL_TELEMETRY_SOURCE_FILES
        src/AirTelemetry.cpp
        src/GroundTelemetry.cpp
        src/endpoints/UDPEndpoint.cpp
        src/endpoints/TCPEndpoint.cpp
        src/endpoints/SerialEndpoint.cpp
        #src/endpoints/WBEndpoint.cpp
        src/ohd_telemetry/InternalTelemetry.cpp)

add_executable(XMavlinkService src/main.cpp
        ${ALL_TELEMETRY_SOURCE_FILES})

target_link_libraries(XMavlinkService ${USED_LIBS})

add_executable(SerialEndpointTest tests/test_serial_endpoint.cpp
        src/endpoints/SerialEndpoint.cpp)
target_link_libraries(SerialEndpointTest ${USED_LIBS})

add_executable(UDPEndpointTest tests/test_udp_endpoint.cpp
        src/endpoints/UDPEndpoint.cpp)
target_link_libraries(UDPEndpointTest ${USED_LIBS})

add_executable(OHDGenerateTelemetryTest tests/test_ohd_generate_telemetry.cpp
        src/ohd_telemetry/InternalTelemetry.cpp)
target_link_libraries(OHDGenerateTelemetryTest ${USED_LIBS})

add_executable(OHDRebootTest tests/test_ohd_reboot.cpp)
target_link_libraries(OHDRebootTest ${USED_LIBS})

add_executable(test_generate_log_message tests/test_generate_log_message.cpp)
target_link_libraries(test_generate_log_message ${USED_LIBS})

#####
add_executable(ServicesLocallyTest tests/test_services_locally.cpp
        ${ALL_TELEMETRY_SOURCE_FILES})

target_link_libraries(ServicesLocallyTest ${USED_LIBS})
#####