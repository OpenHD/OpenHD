add_library(openhd_dummy_plugin SHARED dummy_plugin.cpp)

set_target_properties(openhd_dummy_plugin PROPERTIES
    OUTPUT_NAME "openhd_dummy_plugin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(openhd_dummy_plugin PROPERTIES PREFIX "")
endif()

target_include_directories(openhd_dummy_plugin
    PRIVATE
        ${CMAKE_SOURCE_DIR}/ohd_common/inc)

target_link_libraries(openhd_dummy_plugin
    PRIVATE
        OHDCommonLib)

install(TARGETS openhd_dummy_plugin
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/openhd/plugins"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/openhd/plugins"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/openhd/plugins")
