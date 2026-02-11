# RK3588 Cross-compilation toolchain file for OpenHD
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/rk3588-toolchain.cmake ..
#
# GCC 12.2.0 at /opt/gcc-12.2.0/ was built with --with-sysroot pointing to
# /opt/rk3588_debian12_kernel6_10/sysroot, so it already knows where to find
# C/C++ headers. We must NOT set CMAKE_SYSROOT because that causes CMake to
# pick up aarch64 host tools (like gmake) from the sysroot and fail.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# ── Paths ────────────────────────────────────────────────────────────────────
set(CROSS_COMPILE_PREFIX "aarch64-linux-gnu")
set(TOOLCHAIN_ROOT "/opt/gcc-12.2.0")
set(SYSROOT_PATH "/opt/rk3588_debian12_kernel6_10/sysroot")

# ── Compilers (custom GCC 12.2.0, sysroot is built-in) ──────────────────────
set(CMAKE_C_COMPILER   "${TOOLCHAIN_ROOT}/bin/${CROSS_COMPILE_PREFIX}-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_ROOT}/bin/${CROSS_COMPILE_PREFIX}-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_ROOT}/bin/${CROSS_COMPILE_PREFIX}-gcc")

# ── Binutils (system cross-binutils) ────────────────────────────────────────
set(CMAKE_LINKER   "/usr/bin/${CROSS_COMPILE_PREFIX}-ld")
set(CMAKE_AR       "/usr/bin/${CROSS_COMPILE_PREFIX}-ar")
set(CMAKE_RANLIB   "/usr/bin/${CROSS_COMPILE_PREFIX}-ranlib")
set(CMAKE_NM       "/usr/bin/${CROSS_COMPILE_PREFIX}-nm")
set(CMAKE_OBJCOPY  "/usr/bin/${CROSS_COMPILE_PREFIX}-objcopy")
set(CMAKE_OBJDUMP  "/usr/bin/${CROSS_COMPILE_PREFIX}-objdump")
set(CMAKE_STRIP    "/usr/bin/${CROSS_COMPILE_PREFIX}-strip")

# ── DO NOT set CMAKE_SYSROOT ────────────────────────────────────────────────
# The GCC already has a built-in sysroot. Setting CMAKE_SYSROOT causes CMake
# to discover aarch64 binaries (gmake, python3, etc.) inside the sysroot and
# try to execute them on the x86 host, which fails with:
#   "aarch64-binfmt-P: Could not open '/lib/ld-linux-aarch64.so.1'"
# Instead, we only set CMAKE_FIND_ROOT_PATH so find_library/find_package
# can locate target libraries.
set(CMAKE_FIND_ROOT_PATH "${SYSROOT_PATH}")

# Programs: search on HOST only (never pick up aarch64 binaries)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Libraries & headers: search in TARGET sysroot only
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ── Force host tools ────────────────────────────────────────────────────────
# Ensure host python3 is used for code generation (not target's python3)
find_program(PYTHON_EXECUTABLE NAMES python3 python
    PATHS /usr/bin /usr/local/bin
    NO_CMAKE_FIND_ROOT_PATH
    REQUIRED)
set(Python3_EXECUTABLE "${PYTHON_EXECUTABLE}" CACHE FILEPATH "" FORCE)

# Ensure host make is used
find_program(CMAKE_MAKE_PROGRAM NAMES make gmake
    PATHS /usr/bin /usr/local/bin
    NO_CMAKE_FIND_ROOT_PATH
    REQUIRED)

# Ensure host pkg-config is used
find_program(PKG_CONFIG_EXECUTABLE NAMES pkg-config
    PATHS /usr/bin /usr/local/bin
    NO_CMAKE_FIND_ROOT_PATH
    REQUIRED)
set(PKG_CONFIG_EXECUTABLE "${PKG_CONFIG_EXECUTABLE}" CACHE FILEPATH "" FORCE)

# ── Compiler test workarounds ────────────────────────────────────────────────
# Cannot run aarch64 binaries on x86 host, so use static lib for try_compile
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ── pthread (cannot run test programs during cross-compile) ──────────────────
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(Threads_FOUND TRUE)

# ── Compiler flags ───────────────────────────────────────────────────────────
set(CMAKE_C_FLAGS_INIT   "-O2 -pipe")
set(CMAKE_CXX_FLAGS_INIT "-O2 -pipe")

# ── pkg-config for cross-compilation ────────────────────────────────────────
set(ENV{PKG_CONFIG_PATH}
    "${SYSROOT_PATH}/usr/lib/pkgconfig:${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu/pkgconfig:${SYSROOT_PATH}/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_LIBDIR}
    "${SYSROOT_PATH}/usr/lib/pkgconfig:${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${SYSROOT_PATH}")

# ── Extra search paths for find_library / find_path ─────────────────────────
list(APPEND CMAKE_PREFIX_PATH
    "${SYSROOT_PATH}/usr"
    "${SYSROOT_PATH}/usr/local"
)

set(CMAKE_LIBRARY_PATH
    "${SYSROOT_PATH}/lib"
    "${SYSROOT_PATH}/lib/aarch64-linux-gnu"
    "${SYSROOT_PATH}/usr/lib"
    "${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu"
    "${SYSROOT_PATH}/usr/local/lib"
)

set(CMAKE_INCLUDE_PATH
    "${SYSROOT_PATH}/usr/include"
    "${SYSROOT_PATH}/usr/include/aarch64-linux-gnu"
    "${SYSROOT_PATH}/usr/local/include"
)

# ── Pre-set results for checks that cannot run on the host ───────────────────
set(PCAP_LINKS_SOLO FALSE CACHE BOOL "" FORCE)
set(PCAP_NEEDS_THREADS TRUE CACHE BOOL "" FORCE)

set(CMAKE_CROSSCOMPILING TRUE)
