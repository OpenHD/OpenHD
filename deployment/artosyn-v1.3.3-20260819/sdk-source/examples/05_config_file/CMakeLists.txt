add_executable(l4_config_file
    config_file.c
)

target_link_libraries(l4_config_file
    PRIVATE
        l4_example_common
)

if(NOT WIN32)
    set_target_properties(l4_config_file PROPERTIES
        BUILD_RPATH "\$ORIGIN/../ar8030"
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

INSTALL(TARGETS l4_config_file
    RUNTIME DESTINATION bin
)
