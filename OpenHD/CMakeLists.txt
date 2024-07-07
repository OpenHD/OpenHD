cmake_minimum_required(VERSION 3.16.3)
project(OpenHD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Find the ncurses package
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

# Allows to explicitly disable air support - removes a few dependencies
option(ENABLE_AIR "Enable air support" ON)
message("Air support enabled: ${ENABLE_AIR}")
if (ENABLE_AIR)
    add_definitions(-DENABLE_AIR)
endif()

# Allows to explicitly disable USB camera support - removes a few dependencies
# (Libusb an libv4l2), only makes sense if air support is enabled
option(ENABLE_USB_CAMERAS "USB cameras support" ON)
message("USB cameras enabled: ${ENABLE_USB_CAMERAS}")
if (ENABLE_USB_CAMERAS)
    add_definitions(-DENABLE_USB_CAMERAS)
endif()

# Add all necessary subdirectories
add_subdirectory(ohd_common EXCLUDE_FROM_ALL)
add_subdirectory(ohd_interface EXCLUDE_FROM_ALL)
add_subdirectory(ohd_telemetry EXCLUDE_FROM_ALL)
add_subdirectory(ohd_video EXCLUDE_FROM_ALL)

# Here we build the single executable that makes up OpenHD.
# It depends on all the libraries we have just added above
add_executable(openhd main.cpp)

# Link ncurses and other libraries
target_link_libraries(openhd PRIVATE
    ${CURSES_LIBRARIES}
    OHDInterfaceLib
    OHDVideoLib
    OHDTelemetryLib
    OHDCommonLib
)

# Install the OpenHD executable
install(TARGETS openhd DESTINATION bin)
