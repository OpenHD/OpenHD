cmake_minimum_required(VERSION 3.16.3)
project(OpenHD-Interface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the CMAKE_MODULE_PATH to include the directory where FindPoco.cmake is located
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

# Only needed when we build this submodule manually
add_subdirectory(../ohd_common commonlib EXCLUDE_FROM_ALL)

# Build and include wifibroadcast
include(lib/wifibroadcast/wifibroadcast/WBLib.cmake)

# Find the Poco package (using the custom FindPoco.cmake file)
find_package(Poco REQUIRED COMPONENTS Foundation Net) # Add other components if needed

add_library(OHDInterfaceLib STATIC) # initialized below
add_library(OHDInterfaceLib::OHDInterfaceLib ALIAS OHDInterfaceLib)

option(ENABLE_ARTOSYN_LINK "Enable Artosyn 8030 link (requires SDK)" OFF)
set(ARTOSYN_SDK_ROOT "" CACHE PATH "Path to Artosyn host_drv SDK root")
set(ARTOSYN_SDK_LIB "" CACHE FILEPATH "Path to libar8030_client.a/.so")

set(sources
    src/wifi_card_discovery.cpp
    src/ohd_interface.cpp
    src/usb_tether_listener.cpp
    src/wb_link.cpp
    src/video_crypto.cpp
    src/wifi_hotspot.cpp
    src/wb_link_helper.cpp
    src/wifi_command_helper.cpp
    src/wifi_card.cpp
    src/wb_link_manager.cpp
    src/networking_settings.cpp
    src/wb_link_settings.cpp
    src/wifi_client.cpp
    src/microhard_link.cpp
    src/ethernet_link.cpp
    src/ethernet_manager.cpp
)

if (ENABLE_ARTOSYN_LINK)
    if (NOT ARTOSYN_SDK_ROOT OR NOT ARTOSYN_SDK_LIB)
        message(FATAL_ERROR "ENABLE_ARTOSYN_LINK=ON requires ARTOSYN_SDK_ROOT and ARTOSYN_SDK_LIB")
    endif()
    list(APPEND sources src/artosyn_link.cpp src/artosyn_link_settings.cpp)
    target_compile_definitions(OHDInterfaceLib PUBLIC OHD_ENABLE_ARTOSYN=1)
    target_include_directories(OHDInterfaceLib PUBLIC
        "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030"
        "${ARTOSYN_SDK_ROOT}/host_drv/com"
    )
    target_link_libraries(OHDInterfaceLib PUBLIC "${ARTOSYN_SDK_LIB}")
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

include(CMakePackageConfigHelpers)
target_sources(OHDInterfaceLib PRIVATE ${sources})

target_include_directories(OHDInterfaceLib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>")

# Find and link POCO libraries
target_include_directories(OHDInterfaceLib PUBLIC ${Poco_INCLUDE_DIRS})
target_link_libraries(OHDInterfaceLib PUBLIC Poco::Foundation Poco::Net)

# Link with other libraries
target_link_libraries(OHDInterfaceLib PUBLIC OHDCommonLib)
target_link_libraries(OHDInterfaceLib PUBLIC ${CMAKE_DL_LIBS})

# needed to set wifi cards to monitor mode and more
# include(cmake/FindLibNL.cmake)
# target_include_directories(OHDInterfaceLib PRIVATE ${LibNL_INCLUDE_DIR})
# target_link_libraries(OHDInterfaceLib PRIVATE ${LibNL_LIBRARIES})

target_include_directories(OHDInterfaceLib PUBLIC inc/)
target_link_libraries(OHDInterfaceLib PUBLIC ${WB_TARGET_LINK_LIBRARIES})
# target_include_directories(OHDInterfaceLib PUBLIC ${WB_INCLUDE_DIRECTORIES})

# For some reason, otherwise pi complains about threads
find_package(Threads REQUIRED)
target_link_libraries(OHDInterfaceLib PUBLIC Threads::Threads)

# Custom testing
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
