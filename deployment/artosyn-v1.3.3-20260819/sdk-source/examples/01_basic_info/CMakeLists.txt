add_executable(l4_basic_info
    basic_info.c
)

target_link_libraries(l4_basic_info
    PRIVATE
        l4_example_common
)

if(NOT WIN32)
    set_target_properties(l4_basic_info PROPERTIES
        BUILD_RPATH "\$ORIGIN/../ar8030"
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

INSTALL(TARGETS l4_basic_info
    RUNTIME DESTINATION bin
)
