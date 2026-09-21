cmake_minimum_required(VERSION 3.16.3)
project(dump1090)

set(CMAKE_C_STANDARD 99)

add_library(dump1090 STATIC dump1090.c anet.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RTLSDR REQUIRED librtlsdr)

target_include_directories(dump1090 PRIVATE ${RTLSDR_INCLUDE_DIRS})
target_include_directories(dump1090 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dump1090 PUBLIC ${RTLSDR_LIBRARIES} m pthread)
