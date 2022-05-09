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
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread regex filesystem)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

SET(OHD_TELEMETRY_USED_BOOST_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

SET(ALL_TELEMETRY_SOURCE_FILES
        src/AirTelemetry.cpp
        src/GroundTelemetry.cpp
        src/endpoints/UDPEndpoint.cpp
        src/endpoints/TCPEndpoint.cpp
        src/endpoints/SerialEndpoint.cpp
        #src/endpoints/WBEndpoint.cpp
        src/internal/InternalTelemetry.cpp)

# Build it as a library
add_library(OHDTelemetryLib SHARED ${ALL_TELEMETRY_SOURCE_FILES})
target_link_libraries(OHDTelemetryLib PUBLIC ${OHD_TELEMETRY_USED_BOOST_LIBS})
target_include_directories(OHDTelemetryLib PUBLIC src/)
# include mavlink (header-only)
target_include_directories(OHDTelemetryLib PUBLIC ../lib/c_library_v2_openhd)
# Note: We do not need wifibroadcast as a whole here, we can get the header-only files.
target_include_directories(OHDTelemetryLib PUBLIC ../lib/wifibroadcast/src)

# All the test files for development
add_executable(test_serial_endpoint tests/test_serial_endpoint.cpp)
target_link_libraries(test_serial_endpoint OHDTelemetryLib)

add_executable(test_udp_endpoint tests/test_udp_endpoint.cpp)
target_link_libraries(test_udp_endpoint OHDTelemetryLib)

add_executable(test_ohd_generate_telemetry tests/test_ohd_generate_telemetry.cpp)
target_link_libraries(test_ohd_generate_telemetry OHDTelemetryLib)

add_executable(test_ohd_reboot tests/test_ohd_reboot.cpp)
target_link_libraries(test_ohd_reboot OHDTelemetryLib)

add_executable(test_generate_log_message tests/test_generate_log_message.cpp)
target_link_libraries(test_generate_log_message OHDTelemetryLib)

add_executable(test_services_locally tests/test_services_locally.cpp)
target_link_libraries(test_services_locally OHDTelemetryLib)
#####