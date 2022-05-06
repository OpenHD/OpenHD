cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Interface)

set(CMAKE_CXX_STANDARD 17)

# include the general openhd stuff
include_directories(../ohd_common)

include_directories(inc)

# Build and include wifibroadcast
include(../lib/wifibroadcast/WBLib.cmake)

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread regex filesystem)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

SET(INTERFACE_USED_BOOST_LIBRARIES ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

SET(ALL_INTERFACE_SOURCE_FILES
        src/wifi.cpp
        src/ethernet.cpp
        src/WBStreams.cpp
        src/OHDInterface.cpp)

add_library(OHDInterfaceLib
        SHARED
        ${ALL_INTERFACE_SOURCE_FILES})
target_link_libraries(OHDInterfaceLib ${INTERFACE_USED_BOOST_LIBRARIES})
target_link_libraries(OHDInterfaceLib ${WB_TARGET_LINK_LIBRARIES})
#target_include_directories(OHDInterfaceLib PUBLIC ${WB_INCLUDE_DIRECTORES})

# The main service that is run on OpenHD
add_executable(openhd_interface src/main.cpp)
target_link_libraries(openhd_interface OHDInterfaceLib)