cmake_minimum_required(VERSION 3.16.3)

add_library(OHDAiLib STATIC
        src/ai_pipeline_helper.cpp
        inc/ai_pipeline_helper.hpp
)

target_include_directories(OHDAiLib PUBLIC
        inc
        ../ohd_common/inc
        ../ohd_video/inc
)

# Link against common library for spdlog
target_link_libraries(OHDAiLib PRIVATE OHDCommonLib)

# Find and link OpenCV
find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
    target_include_directories(OHDAiLib PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(OHDAiLib PRIVATE ${OpenCV_LIBRARIES})
    message(STATUS "Found OpenCV version ${OpenCV_VERSION}")
endif()
