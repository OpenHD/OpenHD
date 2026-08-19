add_executable(l4_pair_manager
    pair_manager.c
)

target_link_libraries(l4_pair_manager
    PRIVATE
        l4_example_common
)

if(NOT WIN32)
    set_target_properties(l4_pair_manager PROPERTIES
        BUILD_RPATH "\$ORIGIN/../ar8030"
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

INSTALL(TARGETS l4_pair_manager
    RUNTIME DESTINATION bin
)
