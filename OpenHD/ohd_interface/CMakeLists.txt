cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Interface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  Poco
  URL https://github.com/pocoproject/poco/archive/refs/tags/poco-1.12.5p2-release.zip
)
FetchContent_MakeAvailable(Poco)

# Only needed when we build this submodule manually
add_subdirectory(../ohd_common commonlib EXCLUDE_FROM_ALL)

# Build and include wifibroadcast
include(lib/wifibroadcast/wifibroadcast/WBLib.cmake)

# Add the OHDInterfaceLib static library
add_library(OHDInterfaceLib STATIC)
add_library(OHDInterfaceLib::OHDInterfaceLib ALIAS OHDInterfaceLib)

# Set the source files
set(sources
    src/wifi_card_discovery.cpp
    src/ohd_interface.cpp
    src/usb_tether_listener.cpp
    src/wb_link.cpp
    src/wifi_hotspot.cpp
    src/wb_link_helper.cpp
    src/wifi_command_helper.cpp
    src/wifi_card.cpp
    src/wb_link_manager.cpp
    src/networking_settings.cpp
    src/wb_link_settings.cpp
    src/wifi_client.cpp
    src/microhard_link.cpp
    src/ethernet_manager.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

# Include public headers
target_include_directories(OHDInterfaceLib PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>")

# Add the source files to the target
target_sources(OHDInterfaceLib PRIVATE ${sources})

# Link Poco Libraries
target_link_libraries(OHDInterfaceLib PUBLIC
    Poco::Foundation
    Poco::Util
    Poco::Net
    OHDCommonLib
    ${WB_TARGET_LINK_LIBRARIES}
    Threads::Threads
)

# Custom testing executables
add_executable(test_interface test/test_interface.cpp)
target_link_libraries(test_interface OHDInterfaceLib)

add_executable(test_usb_tether_listener test/test_usb_tether_listener.cpp)
target_link_libraries(test_usb_tether_listener OHDInterfaceLib)

add_executable(test_ethernet_listener test/test_ethernet_listener.cpp)
target_link_libraries(test_ethernet_listener OHDInterfaceLib)

add_executable(test_wifi_hotspot test/test_wifi_hotspot.cpp)
target_link_libraries(test_wifi_hotspot OHDInterfaceLib)

add_executable(test_wifi_commands test/test_wifi_commands.cpp)
target_link_libraries(test_wifi_commands OHDInterfaceLib)

add_executable(test_wifi_set_channel test/test_wifi_set_channel.cpp)
target_link_libraries(test_wifi_set_channel OHDInterfaceLib)
