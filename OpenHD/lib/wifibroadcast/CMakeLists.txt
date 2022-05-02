cmake_minimum_required(VERSION 3.16.3)
project(Wifibroadcast)

set(CMAKE_CXX_STANDARD 17)

include(WBLib.cmake)

add_executable(wfb_tx executables/tx.cpp)
target_link_libraries(wfb_tx  ${WB_TARGET_LINK_LIBRARIES})

add_executable(wfb_rx executables/rx.cpp)
target_link_libraries(wfb_rx  ${WB_TARGET_LINK_LIBRARIES})

add_executable(benchmark executables/benchmark.cpp)
target_link_libraries(benchmark  ${WB_TARGET_LINK_LIBRARIES})

add_executable(udp_generator_validator executables/udp_generator_validator.cpp)
target_link_libraries(udp_generator_validator  ${WB_TARGET_LINK_LIBRARIES})

add_executable(unit_test executables/unit_test.cpp)
target_link_libraries(unit_test  ${WB_TARGET_LINK_LIBRARIES})

#add_executable(socket_helper_test executables/socket_helper_test.cpp)
#target_link_libraries(socket_helper_test  ${WB_TARGET_LINK_LIBRARIES})