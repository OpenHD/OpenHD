set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_TESTING OFF CACHE INTERNAL "")
set(ENABLE_PYTHON OFF CACHE INTERNAL "")

include(FetchContent)

set(libtuntap_patch git apply --ignore-whitespace --reject ${PROJECT_SOURCE_DIR}/third_package/libtuntap-win.patch)

FetchContent_Declare(
    libtuntap
    DOWNLOAD_EXTRACT_TIMESTAMP YES
    URL ${PROJECT_SOURCE_DIR}/third_package/libtuntap.tar
    URL_HASH MD5=a045bdf3199e3f185c7917f25eeeed5c
    PATCH_COMMAND ${libtuntap_patch}
    UPDATE_DISCONNECTED 1
)

FetchContent_GetProperties(libtuntap)

if(NOT libtuntap_POPULATED)
    FetchContent_Populate(libtuntap)
    add_subdirectory(${libtuntap_SOURCE_DIR} ${libtuntap_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(l4_tuntap)
target_sources(l4_tuntap PRIVATE tuntap.cpp)
target_link_libraries(l4_tuntap PRIVATE ar8030_client tuntap++ com)

if(NOT WIN32)
    set_target_properties(l4_tuntap PROPERTIES
        BUILD_RPATH "\$ORIGIN/../ar8030"
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

INSTALL(TARGETS l4_tuntap
    RUNTIME DESTINATION bin
)
