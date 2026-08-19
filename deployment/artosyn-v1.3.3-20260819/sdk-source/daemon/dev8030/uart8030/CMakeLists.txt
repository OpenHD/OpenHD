set(file
    uart_dev.c
    uart_pkt.c
)

add_library(uart8030 STATIC ${file})

if(WIN32)
    target_sources(uart8030 PRIVATE win_uart_backend.cpp)
    target_sources(uart8030 PRIVATE win_uart_list.cpp)
else()
    target_sources(uart8030 PRIVATE linux_uart_backend.c)
endif()

target_link_libraries(uart8030 PRIVATE com)
target_include_directories(uart8030 PRIVATE ../)
target_include_directories(uart8030 PRIVATE ../../)