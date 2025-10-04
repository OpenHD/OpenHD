cmake_minimum_required(VERSION 3.12)
project(mavlink_uart_logger LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Curses REQUIRED)

add_executable(mavlink_uart_logger
    main.cpp
)

target_include_directories(mavlink_uart_logger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ohd_telemetry/lib/mavlink-headers
    ${CURSES_INCLUDE_DIR}
)

target_link_libraries(mavlink_uart_logger PRIVATE
    ${CURSES_LIBRARIES}
)
