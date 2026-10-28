file(READ "${CMAKE_CURRENT_SOURCE_DIR}/assets/sample.h264" video_hex HEX)
string(REGEX REPLACE "(..)" "0x\\1," video_bytes "${video_hex}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/wifi_test_video.h"
    "#pragma once\ninline constexpr unsigned char wifi_test_video[] = {${video_bytes}};\n")
add_library(openhd_wifi_test STATIC wifi_test.cpp
    ../ohd_video/librtp/src/rtp-h264-pack.c
    ../ohd_video/librtp/src/rtp-h265-pack.c
    ../ohd_video/librtp/src/rtp-payload.c
    ../ohd_video/librtp/src/rtp-packet.c
    ../ohd_video/librtp/src/rtp-h264-bitstream.c)
target_compile_features(openhd_wifi_test PUBLIC cxx_std_17)
target_include_directories(openhd_wifi_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ../ohd_common/lib/json/include PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ../ohd_interface/inc ../ohd_common/inc ../platform/ground ../ohd_video/librtp/inc
    ../ohd_common/lib/spdlog/include)
target_link_libraries(openhd_wifi_test PUBLIC wifibroadcast)
if(OPENHD_PORTABLE_GROUND)
    target_link_libraries(openhd_wifi_test PRIVATE openhd_ground_config)
else()
    target_link_libraries(openhd_wifi_test PRIVATE OHDInterfaceLib)
endif()
find_package(Git QUIET)
set(source_revision "unknown")
if(DEFINED ENV{GITHUB_SHA} AND NOT "$ENV{GITHUB_SHA}" STREQUAL "")
    set(source_revision "$ENV{GITHUB_SHA}")
elseif(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE source_revision OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
endif()
target_compile_definitions(openhd_wifi_test PRIVATE OPENHD_WIFI_TEST_REVISION="${source_revision}")
if(WIN32)
    target_compile_definitions(openhd_wifi_test PRIVATE OS_WINDOWS)
endif()

add_executable(test_wifi_test_contract EXCLUDE_FROM_ALL test_contract.cpp)
target_link_libraries(test_wifi_test_contract PRIVATE openhd_wifi_test)
target_include_directories(test_wifi_test_contract PRIVATE ../ohd_interface/inc)
add_test(NAME wifi_test_contract COMMAND test_wifi_test_contract)
