SET(GCC_PATH ${CMAKE_CURRENT_LIST_DIR}/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu)
SET(toolpathprefix ${GCC_PATH}/bin/aarch64-none-linux-gnu-)

SET(CMAKE_C_COMPILER ${toolpathprefix}gcc)
SET(CMAKE_CXX_COMPILER ${toolpathprefix}g++)
SET(CMAKE_STRIP  ${toolpathprefix}strip)
SET(CMAKE_AR ${toolpathprefix}ar)
SET(CMAKE_LD ${toolpathprefix}ld)


set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

message("path ${CMAKE_SYSROOT}")


