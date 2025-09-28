find_library(PIGPIO_LIB pigpio)

if (NOT PIGPIO_LIB)
    message(WARNING "pigpio library not found; skipping Raspberry Pi rover motors plugin")
    return()
endif()

add_library(openhd_rpi_rover_motors_plugin SHARED
    rpi_rover_motors_plugin.cpp
    RaspberryPiRoverMotors.cpp)

set_target_properties(openhd_rpi_rover_motors_plugin PROPERTIES
    OUTPUT_NAME "openhd_rpi_rover_motors"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(openhd_rpi_rover_motors_plugin PROPERTIES PREFIX "")
endif()

target_include_directories(openhd_rpi_rover_motors_plugin
    PRIVATE
        ${CMAKE_SOURCE_DIR}/ohd_common/inc
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(openhd_rpi_rover_motors_plugin
    PRIVATE
        OHDCommonLib
        ${PIGPIO_LIB})

install(TARGETS openhd_rpi_rover_motors_plugin
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/openhd/plugins"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/openhd/plugins"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/openhd/plugins")
