cmake_minimum_required(VERSION 3.12)
project(olog_exporter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(olog_exporter
    main.cpp
)

target_include_directories(olog_exporter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ohd_telemetry/lib/mavlink-headers
)
