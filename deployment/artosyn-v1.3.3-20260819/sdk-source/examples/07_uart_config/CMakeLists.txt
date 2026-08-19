add_executable(l4_uart_config
    uart_config.c
)

target_link_libraries(l4_uart_config
    PRIVATE
        l4_example_common
)

if(NOT WIN32)
    set_target_properties(l4_uart_config PROPERTIES
        BUILD_RPATH "\$ORIGIN/../ar8030"
        INSTALL_RPATH "\$ORIGIN/../lib"
    )
endif()

INSTALL(TARGETS l4_uart_config
    RUNTIME DESTINATION bin
)
