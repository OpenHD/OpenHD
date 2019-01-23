cmake_minimum_required(VERSION 3.3)
project(splash)

set(CMAKE_C_STANDARD 11)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux")
link_directories(/opt/vc/lib/)

set(SOURCE_FILES_SPLASH
        main.c)

add_executable(db_splash ${SOURCE_FILES_SPLASH})
if(UNIX AND NOT APPLE)
    target_link_libraries(db_splash shapes)
endif()