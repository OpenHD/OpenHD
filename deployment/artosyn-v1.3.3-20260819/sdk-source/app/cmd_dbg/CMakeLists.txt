add_executable(l4_cmd_dbg
    cmd_dbg.cpp
)

set_target_properties(l4_cmd_dbg PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(l4_cmd_dbg
    PRIVATE
        ar8030_client
        com
)

set_target_properties(l4_cmd_dbg PROPERTIES
    BUILD_RPATH "\$ORIGIN/../ar8030"
    INSTALL_RPATH "\$ORIGIN/../lib"
)

INSTALL(TARGETS l4_cmd_dbg
    RUNTIME DESTINATION bin
)
