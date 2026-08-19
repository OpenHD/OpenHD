add_executable(l4_project_event
    project_event.c
)

target_link_libraries(l4_project_event
    PRIVATE
        l4_example_common
)

if(NOT WIN32)
    set_target_properties(l4_project_event PROPERTIES
        BUILD_RPATH "\$ORIGIN/../ar8030"
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

INSTALL(TARGETS l4_project_event
    RUNTIME DESTINATION bin
)
