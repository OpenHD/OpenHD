add_library(l4_example_common STATIC
    bb_demo_common.c
)

target_include_directories(l4_example_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/com
)

target_link_libraries(l4_example_common
    PUBLIC
        ar8030_client
)
