cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Interface)

set(CMAKE_CXX_STANDARD 17)

# include the general openhd stuff
include_directories(../ohd_common)

# Build and include wifibroadcast
include(../lib/wifibroadcast/WBLib.cmake)

SET(ALL_INTERFACE_SOURCE_FILES
        src/WifiCards.cpp
        #src/EthernetCards.cpp
        src/WBStreams.cpp
        src/OHDInterface.cpp)

add_library(OHDInterfaceLib
        STATIC
        ${ALL_INTERFACE_SOURCE_FILES})

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
# Changed according to https://cliutils.gitlab.io/modern-cmake/chapters/packages/Boost.html
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(OHDInterfaceLib PUBLIC Boost::filesystem)

target_include_directories(OHDInterfaceLib PUBLIC inc/)
target_link_libraries(OHDInterfaceLib PUBLIC ${WB_TARGET_LINK_LIBRARIES})
#target_include_directories(OHDInterfaceLib PUBLIC ${WB_INCLUDE_DIRECTORES})

# Custom testing, requires written jsons from discovery
add_executable(test_interface test/test_interface.cpp)
target_link_libraries(test_interface OHDInterfaceLib)

#special, temporary
add_executable(test_common_util test/test_common_util.cpp)
target_link_libraries(test_common_util OHDInterfaceLib)