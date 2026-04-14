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

set(ARTOSYN_SDK_ROOT "" CACHE PATH "Path to Artosyn host_drv SDK root")
set(ARTOSYN_SDK_LIB "" CACHE FILEPATH "Path to libar8030_client.a/.so")
set(ARTOSYN_SDK_DAEMON "" CACHE FILEPATH "Path to Artosyn daemon binary")

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

if (NOT ARTOSYN_SDK_ROOT AND DEFINED ENV{ARTOSYN_SDK_ROOT})
    set(ARTOSYN_SDK_ROOT "$ENV{ARTOSYN_SDK_ROOT}" CACHE PATH "Path to Artosyn host_drv SDK root" FORCE)
endif()
if (NOT ARTOSYN_SDK_LIB AND DEFINED ENV{ARTOSYN_SDK_LIB})
    set(ARTOSYN_SDK_LIB "$ENV{ARTOSYN_SDK_LIB}" CACHE FILEPATH "Path to libar8030_client.a/.so" FORCE)
endif()

if (ARTOSYN_SDK_ROOT AND NOT ARTOSYN_SDK_LIB)
    set(_artosyn_lib_candidates
        "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/libar8030_client.a"
        "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/libar8030_client.so"
        "${ARTOSYN_SDK_ROOT}/lib/libar8030_client.a"
        "${ARTOSYN_SDK_ROOT}/lib/libar8030_client.so"
        "${ARTOSYN_SDK_ROOT}/libar8030_client.a"
        "${ARTOSYN_SDK_ROOT}/libar8030_client.so"
    )
    foreach(_candidate IN LISTS _artosyn_lib_candidates)
        if (EXISTS "${_candidate}")
            set(ARTOSYN_SDK_LIB "${_candidate}" CACHE FILEPATH "Path to libar8030_client.a/.so" FORCE)
            break()
        endif()
    endforeach()
endif()

if (NOT ARTOSYN_SDK_ROOT OR NOT ARTOSYN_SDK_LIB)
    message(FATAL_ERROR
        "Artosyn link is mandatory.\n"
        "Set ARTOSYN_SDK_ROOT + ARTOSYN_SDK_LIB (cache or environment),\n"
        "or provide ARTOSYN_SDK_ROOT so CMake can auto-detect libar8030_client."
    )
endif()
if (NOT EXISTS "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030" OR
    NOT EXISTS "${ARTOSYN_SDK_ROOT}/host_drv/com")
    message(FATAL_ERROR
        "Invalid ARTOSYN_SDK_ROOT: expected host_drv/app/ar8030 and host_drv/com under ${ARTOSYN_SDK_ROOT}"
    )
endif()
set(_artosyn_sdk_libs ${ARTOSYN_SDK_LIB})
if (NOT _artosyn_sdk_libs)
    message(FATAL_ERROR "Invalid ARTOSYN_SDK_LIB: ${ARTOSYN_SDK_LIB}")
endif()
foreach(_artosyn_lib IN LISTS _artosyn_sdk_libs)
    if (NOT EXISTS "${_artosyn_lib}")
        message(FATAL_ERROR "Invalid ARTOSYN_SDK_LIB entry: ${_artosyn_lib}")
    endif()
endforeach()

if (NOT ARTOSYN_SDK_DAEMON)
    set(_artosyn_daemon_candidates
        "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/artosyn_daemon"
        "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/ar8030_daemon"
        "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/artlinkd"
        "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/bbd"
        "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030/bb_daemon"
        "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/artosyn_daemon"
        "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/ar8030_daemon"
        "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/artlinkd"
        "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/bbd"
        "${ARTOSYN_SDK_ROOT}/host_drv/build/app/ar8030/bb_daemon"
        "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/artosyn_daemon"
        "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/ar8030_daemon"
        "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/artlinkd"
        "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/bbd"
        "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/bb_daemon"
    )
    foreach(_daemon_candidate IN LISTS _artosyn_daemon_candidates)
        if (EXISTS "${_daemon_candidate}" AND NOT IS_DIRECTORY "${_daemon_candidate}")
            set(ARTOSYN_SDK_DAEMON "${_daemon_candidate}" CACHE FILEPATH "Path to Artosyn daemon binary" FORCE)
            break()
        endif()
    endforeach()
    if (NOT ARTOSYN_SDK_DAEMON)
        file(GLOB_RECURSE _artosyn_daemon_recursive
            "${ARTOSYN_SDK_ROOT}/host_drv/*artosyn_daemon*"
            "${ARTOSYN_SDK_ROOT}/host_drv/*ar8030_daemon*"
            "${ARTOSYN_SDK_ROOT}/host_drv/*artlinkd*"
            "${ARTOSYN_SDK_ROOT}/host_drv/*bb_daemon*"
            "${ARTOSYN_SDK_ROOT}/host_drv/*bbd")
        foreach(_daemon_candidate IN LISTS _artosyn_daemon_recursive)
            if (EXISTS "${_daemon_candidate}" AND NOT IS_DIRECTORY "${_daemon_candidate}")
                set(ARTOSYN_SDK_DAEMON "${_daemon_candidate}" CACHE FILEPATH "Path to Artosyn daemon binary" FORCE)
                break()
            endif()
        endforeach()
    endif()
endif()

set(_artosyn_runtime_libs)
foreach(_artosyn_lib IN LISTS _artosyn_sdk_libs)
    if (_artosyn_lib MATCHES "\\.so(\\..*)?$")
        list(APPEND _artosyn_runtime_libs "${_artosyn_lib}")
    endif()
endforeach()
set(_artosyn_extra_runtime_lib_candidates
    "${ARTOSYN_SDK_ROOT}/host_drv/com/libcom.so"
    "${ARTOSYN_SDK_ROOT}/host_drv/build/com/libcom.so"
    "${ARTOSYN_SDK_ROOT}/host_drv/install/bin/libcom.so"
)
foreach(_candidate IN LISTS _artosyn_extra_runtime_lib_candidates)
    if (EXISTS "${_candidate}" AND NOT IS_DIRECTORY "${_candidate}")
        list(APPEND _artosyn_runtime_libs "${_candidate}")
    endif()
endforeach()
list(REMOVE_DUPLICATES _artosyn_runtime_libs)

list(APPEND sources src/artosyn_link.cpp src/artosyn_link_settings.cpp)
target_compile_definitions(OHDInterfaceLib PUBLIC OHD_ENABLE_ARTOSYN=1)
target_include_directories(OHDInterfaceLib PUBLIC
    "${ARTOSYN_SDK_ROOT}/host_drv/app/ar8030"
    "${ARTOSYN_SDK_ROOT}/host_drv/com"
)
target_link_libraries(OHDInterfaceLib PUBLIC ${_artosyn_sdk_libs})

if (ARTOSYN_SDK_DAEMON AND EXISTS "${ARTOSYN_SDK_DAEMON}")
    install(PROGRAMS "${ARTOSYN_SDK_DAEMON}" DESTINATION bin)
else()
    message(WARNING "Artosyn daemon binary not found in SDK; runtime autostart may fail unless an external service provides it.")
endif()

foreach(_runtime_lib IN LISTS _artosyn_runtime_libs)
    if (EXISTS "${_runtime_lib}")
        install(FILES "${_runtime_lib}" DESTINATION lib)
    endif()
endforeach()

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
