#
# Defines WB_TARGET_LINK_LIBRARIES
# which can then be used to integrate wifibroadcast into your project

cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_CXX_STANDARD 17)

# Build and include wifibroadcast
# ----------------------------------
add_library( radiotap
        SHARED
        ${CMAKE_CURRENT_LIST_DIR}/src/external/radiotap/radiotap.c
        )

add_library( fec
        SHARED
        ${CMAKE_CURRENT_LIST_DIR}/src/external/fec/fec.cpp
        )

add_library( wifibroadcast
        SHARED
        ${CMAKE_CURRENT_LIST_DIR}/src/WBReceiver.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/WBTransmitter.cpp
        )

include(${CMAKE_CURRENT_LIST_DIR}/cmake/FindPCAP.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/FindSodium.cmake)

include_directories(${sodium_INCLUDE_DIR})
include_directories(${PCAP_INCLUDE_DIR})

SET(WB_TARGET_LINK_LIBRARIES wifibroadcast radiotap fec ${PCAP_LIBRARY} ${sodium_LIBRARY_RELEASE})

include_directories(${CMAKE_CURRENT_LIST_DIR}/src/HelperSources)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -mavx2 -faligned-new=256")
# ----------------------------------