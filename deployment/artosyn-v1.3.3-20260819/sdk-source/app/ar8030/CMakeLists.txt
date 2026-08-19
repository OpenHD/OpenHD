
set(file
    bb_dev.c
    session_ioctl.c
    socket.c
    session_callback.c
    ar8030.c
    session.c
    socket_fd.c
    session_debug.c
    session_socket.c
    session_socket_datagram_ext.c
    session_hotplug.c
    session_platform_ctrl.c
    session_daemon_ctrl.c
    session_uart_ext.c
    session_exec.c
    undefined_fun.c
)

if(APP_STATIC_LIB)
    add_library(ar8030_client STATIC ${file})
else()
    add_library(ar8030_client SHARED ${file})

    if(WIN32)
        install(FILES $<TARGET_PDB_FILE:ar8030_client> DESTINATION bin OPTIONAL)
    endif()
endif()

if(ADD_WRAP)
    target_sources(ar8030_client PRIVATE warp_helper/ar8030_callback_helper.cpp)
    target_sources(ar8030_client PRIVATE warp_helper/list_helper.cpp)
    target_sources(ar8030_client PRIVATE warp_helper/uart_helper.c)
endif()

if(WIN32)
    target_sources(ar8030_client PRIVATE socket_win.c)
else()
    target_sources(ar8030_client PRIVATE socket_linux.c)
    target_sources(ar8030_client PRIVATE net_dev.c)
endif()

if(WIN32)
    target_link_libraries(ar8030_client PUBLIC "Ws2_32.lib")
    target_link_libraries(ar8030_client PUBLIC "pthread.lib")
elseif(NOT ANDROID)
    target_compile_options(ar8030_client PRIVATE "-fPIC")
    target_link_libraries(ar8030_client PUBLIC -lpthread)
endif()

target_link_directories(ar8030_client PUBLIC ${PROJECT_SOURCE_DIR}/com)
target_link_libraries(ar8030_client PRIVATE com)

INSTALL(TARGETS ar8030_client
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

if(NOT APP_STATIC_LIB)
    INSTALL(TARGETS ar8030_client
        RUNTIME DESTINATION dev_helper
        ARCHIVE DESTINATION dev_helper
    )
endif()
