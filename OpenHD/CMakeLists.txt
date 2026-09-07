cmake_minimum_required(VERSION 3.28)
project(OpenHD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --- Kconfig Integration ---
find_package(Python3 REQUIRED)
set(KCONFIG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../Kconfig")
set(KCONFIG_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/kconfig.py")
set(KCONFIG_DOTCONFIG "${CMAKE_CURRENT_SOURCE_DIR}/../.config")
set(KCONFIG_HEADER "${CMAKE_BINARY_DIR}/autoconf.h")
set(KCONFIG_CMAKE "${CMAKE_BINARY_DIR}/kconfig.cmake")

execute_process(
    COMMAND ${Python3_EXECUTABLE} ${KCONFIG_SCRIPT}
            --kconfig ${KCONFIG_ROOT}
            --config ${KCONFIG_DOTCONFIG}
            --out-header ${KCONFIG_HEADER}
            --out-cmake ${KCONFIG_CMAKE}
    RESULT_VARIABLE KCONFIG_RESULT
)

if (NOT KCONFIG_RESULT EQUAL 0)
    message(FATAL_ERROR "Kconfig generation failed")
endif()

include(${KCONFIG_CMAKE})
include_directories(${CMAKE_BINARY_DIR})
# ---------------------------

# Use Kconfig variables instead of manual options
message("Air support enabled: ${OPENHD_AIR}")
if (OPENHD_AIR)
        add_definitions(-DENABLE_AIR)
endif ()

message("USB cameras enabled: ${OPENHD_USB_CAMERAS}")
if (OPENHD_USB_CAMERAS)
        add_definitions(-DENABLE_USB_CAMERAS)
endif ()

# Add subdirectories for libraries
add_subdirectory(ohd_common EXCLUDE_FROM_ALL)
add_subdirectory(ohd_interface EXCLUDE_FROM_ALL)
# EXCLUDE_FROM_ALL also excludes subdirectory install rules. Register the
# on-demand Pi scout bundle here so normal installs and Debian staging include it.
include(ohd_interface/nexmon/install.cmake)
add_subdirectory(ohd_telemetry EXCLUDE_FROM_ALL)
add_subdirectory(ohd_video EXCLUDE_FROM_ALL)

# Suppress specific warnings
add_compile_options(-Wno-address-of-packed-member -Wno-cast-align)

# Here we build the single executable that makes up OpenHD.
# It depends on all the libraries we have just added above
add_executable(openhd main.cpp
        ohd_common/inc/config_paths.h
        ohd_common/src/config_paths.cpp)

# Explicitly link stdc++fs for GCC 8.x compatibility with std::filesystem
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(openhd PRIVATE stdc++fs)
endif()

# Link the libraries
target_link_libraries(openhd PRIVATE OHDInterfaceLib OHDVideoLib OHDTelemetryLib OHDCommonLib)

# Check if libatomic is needed and available
include(CheckLibraryExists)
check_library_exists(atomic __atomic_fetch_add_1 "" HAVE_LIBATOMIC)

if (HAVE_LIBATOMIC)
    target_link_libraries(openhd PRIVATE atomic)
endif()

# Install the OpenHD executable
install(TARGETS openhd DESTINATION bin)
