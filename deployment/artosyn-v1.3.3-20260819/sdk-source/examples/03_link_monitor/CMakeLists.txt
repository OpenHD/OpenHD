add_executable(l4_link_monitor
    link_monitor.c
)

target_link_libraries(l4_link_monitor
    PRIVATE
        l4_example_common
        m
)

if(NOT WIN32)
    set_target_properties(l4_link_monitor PROPERTIES
        BUILD_RPATH "\$ORIGIN/../ar8030"
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

INSTALL(TARGETS l4_link_monitor
    RUNTIME DESTINATION bin
)
