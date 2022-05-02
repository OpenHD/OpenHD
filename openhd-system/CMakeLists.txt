cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-System)

set(CMAKE_CXX_STANDARD 17)

# from https://mavlink.io/en/mavgen_c/
#You can supress the warnings in CMake using
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# include the general openhd stuff
include_directories(../openhd-common)

include_directories(inc)

# from https://github.com/alexott/boost-asio-examples/blob/master/CMakeLists.txt
# Find and include boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system thread regex filesystem)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

SET(ALL_SYSTEM_SOURCE_FILES
        src/cameras.cpp
        src/ethernet.cpp
        src/main.cpp
        src/nl.c
        src/platform.cpp
        src/profile.cpp
        src/wifi.cpp)

# The main service that is run on OpenHD
add_executable(openhd_system src/main.cpp
        ${ALL_INTERFACE_SOURCE_FILES})
target_link_libraries(openhd_system ${USED_LIBS})