

set(file
    dev8030.c
    hotplug_rpc.c
)

add_library(dev8030 STATIC ${file})
target_include_directories(dev8030 PRIVATE ../)
target_include_directories(dev8030 PRIVATE ./)

if(USING_8030USB)
    add_subdirectory(usb8030)
    target_link_directories(dev8030 PUBLIC usb8030)
    target_link_libraries(dev8030 PRIVATE usb8030)
endif()

if(USING_8030SDIO)
    add_subdirectory(sdio8030)
    target_link_directories(dev8030 PUBLIC sdio8030)
    target_link_libraries(dev8030 PRIVATE sdio8030)
endif()

if(USING_8030UART)
    add_subdirectory(uart8030)
    target_link_directories(dev8030 PUBLIC uart8030)
    target_link_libraries(dev8030 PRIVATE uart8030)
endif()


if(USING_8030DRV)
    add_subdirectory(drv8030)
    target_link_directories(dev8030 PUBLIC drv8030)
    target_link_libraries(dev8030 PRIVATE drv8030)
endif()
