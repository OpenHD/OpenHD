cmake_minimum_required(VERSION 3.16)
project(openhd_topotek_plugin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(OPENHD_PLUGIN_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include" CACHE PATH
    "Directory containing openhd_plugin.h")

add_library(openhd_topotek SHARED
    src/topotek_plugin.cpp
    src/topotek_protocol.cpp)
target_include_directories(openhd_topotek PRIVATE
    "${OPENHD_PLUGIN_INCLUDE_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/src")
set_target_properties(openhd_topotek PROPERTIES
    OUTPUT_NAME openhd_topotek
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES)

install(TARGETS openhd_topotek
    LIBRARY DESTINATION lib/openhd/plugins)

option(TOPOTEK_BUILD_TESTS "Build Topotek protocol tests" ON)
if(TOPOTEK_BUILD_TESTS)
  enable_testing()
  add_executable(test_topotek_protocol
      test/test_topotek_protocol.cpp
      src/topotek_protocol.cpp)
  target_include_directories(test_topotek_protocol PRIVATE
      "${CMAKE_CURRENT_LIST_DIR}/src")
  add_test(NAME topotek_protocol COMMAND test_topotek_protocol)

  add_executable(test_topotek_plugin test/test_topotek_plugin.cpp)
  target_include_directories(test_topotek_plugin PRIVATE
      "${OPENHD_PLUGIN_INCLUDE_DIR}")
  target_link_libraries(test_topotek_plugin PRIVATE ${CMAKE_DL_LIBS})
  add_test(NAME topotek_plugin
      COMMAND test_topotek_plugin $<TARGET_FILE:openhd_topotek>)
endif()
