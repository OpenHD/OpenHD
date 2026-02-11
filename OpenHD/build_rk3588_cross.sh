#!/bin/bash

################################################################################
# OpenHD RK3588 Cross-compilation Build Script
# 
# Prerequisites:
# - Cross-compilation toolchain in /opt/gcc-12.2.0/
# - RK3588 sysroot in /opt/rk3588_debian12_kernel6_10/sysroot/
# - Required dependencies installed in sysroot:
#   - libgstreamer1.0-dev, libgstreamer-plugins-base1.0-dev
#   - libpoco-dev, libsdl2-dev, libudev-dev, libusb-1.0-0-dev
################################################################################

set -e  # Exit on error

# Configuration
TOOLCHAIN_ROOT="/opt/gcc-12.2.0"
SYSROOT_PATH="/opt/rk3588_debian12_kernel6_10/sysroot"
CROSS_PREFIX="aarch64-linux-gnu"
BUILD_DIR="build_rk3588"
INSTALL_PREFIX="/opt/openhd"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Verify toolchain exists
if [ ! -d "$TOOLCHAIN_ROOT" ]; then
    print_error "Toolchain not found at $TOOLCHAIN_ROOT"
    exit 1
fi

if [ ! -d "$SYSROOT_PATH" ]; then
    print_error "Sysroot not found at $SYSROOT_PATH" 
    exit 1
fi

print_status "Starting RK3588 cross-compilation build..."

# Set up environment variables for cross-compilation
export PKG_CONFIG_PATH="${SYSROOT_PATH}/usr/lib/pkgconfig:${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu/pkgconfig:${SYSROOT_PATH}/usr/share/pkgconfig"
export PKG_CONFIG_LIBDIR="${SYSROOT_PATH}/usr/lib/pkgconfig:${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR="${SYSROOT_PATH}"

# Add toolchain to PATH
export PATH="${TOOLCHAIN_ROOT}/bin:$PATH"

print_status "Environment configured:"
print_status "  Toolchain: $TOOLCHAIN_ROOT"
print_status "  Sysroot: $SYSROOT_PATH"
print_status "  Cross prefix: $CROSS_PREFIX"

# Parse command line arguments
CLEAN_BUILD=false
RECONFIGURE=false
for arg in "$@"; do
    case $arg in
        --clean)
            CLEAN_BUILD=true
            ;;
        --reconfigure)
            RECONFIGURE=true
            ;;
        --help|-h)
            echo "Usage: $0 [options]"
            echo "  --clean        Clean build (delete build dir and rebuild everything)"
            echo "  --reconfigure  Force CMake reconfiguration without cleaning"
            echo "  (no options)   Incremental build (only rebuild changed files)"
            exit 0
            ;;
    esac
done

# Clean build if requested
if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
    print_status "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Only run CMake configure if not yet configured or explicitly requested
if [ ! -f "CMakeCache.txt" ] || [ "$RECONFIGURE" = true ] || [ "$CLEAN_BUILD" = true ]; then
    print_status "Configuring CMake with RK3588 toolchain..."

    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/rk3588-toolchain.cmake \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
          -DENABLE_AIR=ON \
          -DENABLE_USB_CAMERAS=ON \
          -DCMAKE_VERBOSE_MAKEFILE=ON \
          -G "Unix Makefiles" \
          ..

    if [ $? -ne 0 ]; then
        print_error "CMake configuration failed!"
        exit 1
    fi
else
    print_status "Using existing CMake configuration (incremental build)"
    print_status "Use --reconfigure to force reconfiguration, --clean for full rebuild"
fi

print_status "Building OpenHD for RK3588 (incremental)..."

# Build with parallel jobs - make will only rebuild changed files
make -j$(nproc)

if [ $? -eq 0 ]; then
    print_status "Build completed successfully!"
    print_status "Binary located at: $(pwd)/openhd"
    
    # Optional: strip binary to reduce size  
    STRIP_BIN="/usr/bin/${CROSS_PREFIX}-strip"
    if [ -f "$STRIP_BIN" ] && [ -x "$STRIP_BIN" ]; then
        "$STRIP_BIN" openhd
        print_status "Binary stripped for deployment"
    elif command -v ${CROSS_PREFIX}-strip &> /dev/null; then
        ${CROSS_PREFIX}-strip openhd
        print_status "Binary stripped with fallback strip"
    else
        print_warning "Cross-platform strip not available, binary not stripped"
    fi
    
    # Show binary info
    file openhd
    ls -lh openhd
    
else
    print_error "Build failed!"
    exit 1
fi

print_status "RK3588 cross-compilation completed!"
