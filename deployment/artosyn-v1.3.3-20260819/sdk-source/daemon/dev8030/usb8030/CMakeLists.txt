
set(LIBUSB_ENABLE_UDEV OFF CACHE INTERNAL "")

include(FetchContent)

FetchContent_Declare(
    libusb-cmake
    DOWNLOAD_EXTRACT_TIMESTAMP YES
    URL ${PROJECT_SOURCE_DIR}/third_package/libusb/libusb-cmake.tar
    URL_HASH MD5=fbcb49c19ae645efa9c9c77728651ffb
)

FetchContent_GetProperties(libusb-cmake)

if(NOT libusb-cmake_POPULATED)
    FetchContent_Populate(libusb-cmake)
    add_subdirectory(${libusb-cmake_SOURCE_DIR} ${libusb-cmake_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(file
    usb_dev.c
)

add_library(usb8030 STATIC ${file})

target_include_directories(usb8030 PRIVATE ../)
target_include_directories(usb8030 PRIVATE ../../)

target_include_directories(usb8030 PRIVATE usb-1.0)
target_link_libraries(usb8030 PUBLIC usb-1.0)
