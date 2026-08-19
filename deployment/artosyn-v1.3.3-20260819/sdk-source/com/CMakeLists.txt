
set(file
    ringbuffer.c
    ringbuffer_ext.c
    usbpack.c
    socketfd_port.c
    timspec_helper.c
    ioctl_tab.c
    com_log.c
    bg_proc.c
)

add_library(com STATIC ${file})

if(NOT WIN32)
    target_compile_options(com PRIVATE "-fPIC")
    install(FILES ../app/ar8030/ar_net_api.h DESTINATION include)
else()
    target_sources(com PRIVATE CrashDump.cpp)
    target_sources(com PRIVATE getopt/getopt.c getopt/getopt1.c)
    set_source_files_properties(getopt/getopt.c PROPERTIES COMPILE_FLAGS -DHAVE_STRING_H=1)
    target_include_directories(com PUBLIC getopt)
endif()

install(FILES bb_api.h DESTINATION include)
install(FILES bb_config.h DESTINATION include)
