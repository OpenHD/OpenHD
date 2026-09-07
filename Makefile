# OpenHD 3.0 Modular Build System - 2026 Standard
# Standard entry point for configuration and building

PYTHON = python3
SCRIPT_DIR = scripts
BUILD_DIR = OpenHD/build
CMAKE_GEN = Ninja
BUILD_TYPE = Release
EXTRA_CMAKE =

# Detect number of cores
NPROC = $(shell nproc 2>/dev/null || echo 4)

# Platform specific environment (Overridable)
RK3588_TOOLCHAIN = /opt/gcc-12.2.0
RK3588_SYSROOT = /opt/rk3588_debian12_kernel6_10/sysroot
ORQA_SDK = /opt/orqa-sdk

.PHONY: help setup submodules menuconfig build clean distclean config air ground rk3588 orqa debug release install check portable orqa-tools mpp-setup test coverage

help:
	@echo "OpenHD Modular Build System (2026 Standard)"
	@echo ""
	@echo "Available commands:"
	@echo "  make setup        - Install dependencies (kconfiglib) and prepare env"
	@echo "  make submodules   - Initialize and update git submodules"
	@echo "  make menuconfig   - Start interactive configuration"
	@echo "  make config       - Generate headers/cmake files from .config"
	@echo "  make build        - Configure and build (Default: Ninja, Release)"
	@echo "  make test         - Build and run all unit tests"
	@echo "  make coverage     - Build with coverage flags and generate report"
	@echo "  make air/ground   - Load profile and build"
	@echo "  make rk3588       - Cross-compile for RK3588"
	@echo "  make orqa         - Cross-compile for Orqa Controller"
	@echo "  make portable     - Portable cross-build (ARCH=arm64 SYSROOT=/path)"
	@echo "  make check        - Verify module isolation"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make distclean    - Remove all generated files"
	@echo ""

setup:
	@echo "Checking for kconfiglib..."
	@$(PYTHON) -c "import kconfiglib" 2>/dev/null || (echo "Installing kconfiglib..." && pip3 install kconfiglib)
	@echo "Checking CMake version..."
	@cmake_version=$$(cmake --version | head -n1 | grep -oE '[0-9]+\.[0-9]+'); \
	 if [ $$(echo "$$cmake_version < 3.28" | bc -l) -eq 1 ]; then \
	   echo "Error: CMake 3.28+ is required for C++20 modules. Found $$cmake_version"; \
	   exit 1; \
	 fi
	@echo "Environment ready."

submodules:
	@echo "Initializing and updating submodules..."
	@git submodule update --init --recursive
	@echo "Submodules updated."

menuconfig: setup
	@echo "Starting Menuconfig..."
	@$(PYTHON) -m menuconfig
	@$(MAKE) config

config: setup
	@echo "Generating configuration files..."
	@if [ ! -f .config ]; then echo "Warning: .config missing, using defaults."; fi
	@$(PYTHON) $(SCRIPT_DIR)/kconfig.py --kconfig Kconfig --config .config --out-header autoconf.h --out-cmake kconfig.cmake

# Helper to resolve Artosyn SDK if enabled in .config
resolve_artosyn:
	@if grep -q "CONFIG_OPENHD_ENABLE_ARTOSYN=y" .config 2>/dev/null; then \
		echo "Resolving Artosyn SDK..."; \
		bash -c "source OpenHD/scripts/resolve_artosyn_sdk.sh && resolve_artosyn_sdk && \
		         echo \"ARTOSYN_SDK_ROOT=\$$ARTOSYN_SDK_ROOT\" > .artosyn_env && \
		         echo \"ARTOSYN_SDK_LIB=\$$ARTOSYN_SDK_LIB\" >> .artosyn_env && \
		         echo \"ARTOSYN_SDK_DAEMON=\$$ARTOSYN_SDK_DAEMON\" >> .artosyn_env && \
		         echo \"ARTOSYN_SDK_TUNTAP=\$$ARTOSYN_SDK_TUNTAP\" >> .artosyn_env"; \
	fi

build: config resolve_artosyn
	@echo "Starting Build ($(BUILD_TYPE)) using $(CMAKE_GEN)..."
	@if [ -f .artosyn_env ]; then . ./.artosyn_env && export EXTRA_ARTOSYN="-DARTOSYN_SDK_ROOT=\$$ARTOSYN_SDK_ROOT -DARTOSYN_SDK_LIB=\$$ARTOSYN_SDK_LIB -DARTOSYN_SDK_DAEMON=\$$ARTOSYN_SDK_DAEMON -DARTOSYN_SDK_TUNTAP=\$$ARTOSYN_SDK_TUNTAP"; fi; \
	mkdir -p $(BUILD_DIR) && \
	cd $(BUILD_DIR) && cmake -G "$(CMAKE_GEN)" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_VERBOSE_MAKEFILE=ON $(EXTRA_CMAKE) $$EXTRA_ARTOSYN .. && cmake --build . -j$(NPROC)

test: build
	@echo "Running unit tests..."
	@cd $(BUILD_DIR) && ./test_openhd_util && ./test_config && ./test_logging

coverage: config resolve_artosyn
	@echo "Building with Coverage analysis..."
	@if [ -f .artosyn_env ]; then . ./.artosyn_env && export EXTRA_ARTOSYN="-DARTOSYN_SDK_ROOT=\$$ARTOSYN_SDK_ROOT -DARTOSYN_SDK_LIB=\$$ARTOSYN_SDK_LIB -DARTOSYN_SDK_DAEMON=\$$ARTOSYN_SDK_DAEMON -DARTOSYN_SDK_TUNTAP=\$$ARTOSYN_SDK_TUNTAP"; fi; \
	mkdir -p $(BUILD_DIR) && \
	cd $(BUILD_DIR) && cmake -G "$(CMAKE_GEN)" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $(EXTRA_CMAKE) $$EXTRA_ARTOSYN .. && \
	cmake --build . -j$(NPROC)
	@echo "Running tests for coverage..."
	@cd $(BUILD_DIR) && ./test_openhd_util || true && ./test_config || true && ./test_logging || true
	@echo "Generating coverage report (coverage.xml)..."
	@gcovr --sonarqube -o coverage.xml -r . --exclude '.*lib/.*' --exclude '.*test/.*'

air:
	@echo "Loading Air Unit configuration..."
	@cp configs/air_defconfig .config
	@$(MAKE) build

ground:
	@echo "Loading Ground Station configuration..."
	@cp configs/ground_defconfig .config
	@$(MAKE) build

rk3588:
	@echo "Loading RK3588 configuration and environment..."
	@cp configs/rk3588_defconfig .config
	@export PATH=$(RK3588_TOOLCHAIN)/bin:$$PATH && \
	 export PKG_CONFIG_SYSROOT_DIR=$(RK3588_SYSROOT) && \
	 export PKG_CONFIG_LIBDIR=$(RK3588_SYSROOT)/usr/lib/pkgconfig:$(RK3588_SYSROOT)/usr/lib/aarch64-linux-gnu/pkgconfig && \
	 export PKG_CONFIG_PATH=$(RK3588_SYSROOT)/usr/lib/pkgconfig:$(RK3588_SYSROOT)/usr/lib/aarch64-linux-gnu/pkgconfig:$(RK3588_SYSROOT)/usr/share/pkgconfig && \
	 $(MAKE) build EXTRA_CMAKE="-DCMAKE_TOOLCHAIN_FILE=cmake/rk3588-toolchain.cmake -DCMAKE_INSTALL_PREFIX=/opt/openhd"
	@echo "Validating RK3588 binary..."
	@readelf -h $(BUILD_DIR)/openhd | grep -q 'Machine:.*AArch64' || (echo "Error: Wrong architecture!"; exit 1)
	@echo "Stripping binary..."
	@aarch64-linux-gnu-strip $(BUILD_DIR)/openhd 2>/dev/null || echo "Strip failed, skipping."

orqa:
	@echo "Loading Orqa configuration and environment..."
	@cp configs/orqa_defconfig .config
	@if [ -f $(ORQA_SDK)/environment-setup-armv8a-poky-linux ]; then \
		. $(ORQA_SDK)/environment-setup-armv8a-poky-linux && \
		$(MAKE) build EXTRA_CMAKE="-DCMAKE_TOOLCHAIN_FILE=cmake/orqa-toolchain.cmake -DCMAKE_DISABLE_FIND_PACKAGE_SDL2=TRUE" && \
		echo "Validating Orqa binary..." && \
		readelf -h $(BUILD_DIR)/openhd | grep -q 'Machine:.*AArch64' && \
		readelf -d $(BUILD_DIR)/openhd | grep -q 'libPocoFoundation.so' && \
		! readelf -d $(BUILD_DIR)/openhd | grep -q 'libSDL2' || (echo "Validation failed (check SDL2 linkage)!"; exit 1); \
	else \
		echo "Orqa SDK not found at $(ORQA_SDK)"; exit 1; \
	fi

orqa-tools:
	@echo "Building Orqa tools (iwconfig)..."
	@bash scripts/build_orqa_iwconfig.sh $(ORQA_SDK) $(BUILD_DIR)/orqa-tools/iwconfig

mpp-setup:
	@echo "Installing Rockchip MPP into Sysroot..."
	@if [ -z "$(SYSROOT)" ]; then echo "Usage: make mpp-setup ARCH=arm64 SYSROOT=/path"; exit 1; fi
	@bash scripts/install_rockchip_mpp_sysroot.sh arm64 $(SYSROOT)

portable:
	@echo "Starting Portable Cross-build for $(ARCH)..."
	@if [ -z "$(ARCH)" ] || [ -z "$(SYSROOT)" ]; then echo "Usage: make portable ARCH=arm64 SYSROOT=/path"; exit 1; fi
	@cp configs/release_defconfig .config
	@POCO_DIR=$$(find $(SYSROOT)/usr -name PocoConfig.cmake -print -quit | xargs dirname) && \
	 SDL2_DIR=$$(find $(SYSROOT)/usr \( -name sdl2-config.cmake -o -name SDL2Config.cmake \) -print -quit | xargs dirname) && \
	 export OPENHD_ARTOSYN_FORCE_SOURCE_BUILD=1 && \
	 $(MAKE) build EXTRA_CMAKE="-DCMAKE_TOOLCHAIN_FILE=cmake/portable-linux-toolchain.cmake -DOPENHD_SYSROOT=$(SYSROOT) -DOPENHD_CROSS_TRIPLET=$$( [ "$(ARCH)" = "arm64" ] && echo "aarch64-linux-gnu" || echo "arm-linux-gnueabihf") -DPoco_DIR=$$POCO_DIR -DSDL2_DIR=$$SDL2_DIR -DOPENHD_MPP_ONLY=OFF -DENABLE_LIBCAMERA=OFF" && \
	 echo "Generating cross-build manifest..." && \
	 (echo "ARCH=$(ARCH)"; echo "SYSROOT=$(SYSROOT)"; [ -f .artosyn_env ] && cat .artosyn_env) > $(BUILD_DIR)/openhd-cross.env

debug:
	@echo "Setting up Debug build..."
	@cp configs/debug_defconfig .config
	@$(MAKE) build BUILD_TYPE=Debug

release:
	@echo "Setting up Release build..."
	@cp configs/release_defconfig .config
	@$(MAKE) build BUILD_TYPE=Release

install:
	@echo "Installing OpenHD..."
	cd $(BUILD_DIR) && cmake --install .

check: config
	@echo "Building submodules independently..."
	@mkdir -p OpenHD/ohd_common/build_check && cd OpenHD/ohd_common/build_check && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .
	@mkdir -p OpenHD/ohd_interface/build_check && cd OpenHD/ohd_interface/build_check && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .
	@mkdir -p OpenHD/ohd_telemetry/build_check && cd OpenHD/ohd_telemetry/build_check && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .
	@mkdir -p OpenHD/ohd_video/build_check && cd OpenHD/ohd_video/build_check && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .

clean:
	@echo "Cleaning artifacts..."
	rm -rf $(BUILD_DIR)
	rm -rf OpenHD/ohd_common/build_check
	rm -rf OpenHD/ohd_interface/build_check
	rm -rf OpenHD/ohd_telemetry/build_check
	rm -rf OpenHD/ohd_video/build_check
	rm -f autoconf.h kconfig.cmake .artosyn_env coverage.xml

distclean: clean
	rm -f .config
