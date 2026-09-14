# OpenHD 3.0 Modular Build System
# Standard entry point for configuration and building

# --- Load Environment Overrides ---
# Use '-' to ignore error if .env is missing.
# NOTE: Avoid quotes in .env (e.g. use VAR=val instead of VAR="val")
-include .env

# Export all variables to sub-processes (CMake, Shell scripts)
export

# --- OS & uv Detection ---
ifeq ($(OS),Windows_NT)
	IS_WINDOWS := 1
	VENV_PYTHON := .venv/Scripts/python.exe
	UV_INSTALL_CMD := powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
	# Windows NPROC
	NPROC ?= $(NUMBER_OF_PROCESSORS)
else
	IS_WINDOWS := 0
	VENV_PYTHON := .venv/bin/python3
	UV_INSTALL_CMD := curl -LsSf https://astral.sh/uv/install.sh | sh
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		# macOS NPROC
		NPROC ?= $(shell sysctl -n hw.ncpu)
	else
		# Linux NPROC
		NPROC ?= $(shell nproc 2>/dev/null || echo 4)
	endif
endif

# Detect uv - check PATH first, then common local install locations
UV := $(shell command -v uv 2> /dev/null)
ifeq ($(UV),)
	ifeq ($(IS_WINDOWS),1)
		UV_PATH := $(USERPROFILE)\.local\bin\uv.exe
	else
		UV_PATH := $(HOME)/.local/bin/uv
	endif
	UV := $(shell if [ -f $(UV_PATH) ]; then echo $(UV_PATH); fi)
endif

# If still not found, we will install it during setup
# -------------------------

PYTHON ?= $(shell if [ -f $(VENV_PYTHON) ]; then echo $(VENV_PYTHON); else command -v python3 || echo python; fi)
SCRIPT_DIR = scripts
OPENHD_SOURCE_DIR := $(CURDIR)/OpenHD
BUILD_DIR = OpenHD/build
CMAKE_GEN = Ninja
BUILD_TYPE = Release
EXTRA_CMAKE =

# --- Auto-detect ccache for faster compilation ---
CCACHE := $(shell command -v ccache 2>/dev/null)
ifneq ($(CCACHE),)
    EXTRA_CMAKE += -DCMAKE_CXX_COMPILER_LAUNCHER=$(CCACHE) -DCMAKE_C_COMPILER_LAUNCHER=$(CCACHE)
endif

# Platform specific environment (Overridable via .env or environment)
RK3588_TOOLCHAIN ?= /opt/gcc-12.2.0
RK3588_SYSROOT   ?= /opt/rk3588_debian12_kernel6_10/sysroot
ORQA_SDK         ?= /opt/orqa-sdk

.PHONY: help setup submodules menuconfig build clean distclean config air ground rk3588 orqa debug release install check portable orqa-tools mpp-setup test coverage x21 luckfox lyra

help:
	@echo "OpenHD Modular Build System"
	@echo ""
	@echo "Available commands:"
	@echo "  make setup        - Install dependencies (uv + kconfiglib) and prepare env"
	@echo "  make submodules   - Initialize and update git submodules"
	@echo "  make menuconfig   - Start interactive configuration"
	@echo "  make config       - Generate headers/cmake files from .config"
	@echo "  make setconfig    - Set a value via CLI (SYM=SYMBOL VAL=VALUE)"
	@echo "  make build        - Configure and build (Default: Ninja, Release)"
	@echo "  make test         - Build and run all unit tests"
	@echo "  make coverage     - Build with coverage flags and generate report"
	@echo "  make air/ground   - Load profile and build"
	@echo "  make rk3588       - Cross-compile for RK3588"
	@echo "  make orqa         - Cross-compile for Orqa Controller"
	@echo "  make portable     - Portable cross-build (ARCH=arm64 SYSROOT=/path)"
	@echo "  make check        - Verify module isolation"
	@echo "  make x21          - Build for X21 platform (Usage: make x21 SDK=/path)"
	@echo "  make luckfox      - Build for Luckfox Pico platform (Usage: make luckfox SDK=/path)"
	@echo "  make lyra         - Build for Lyra ground module (Usage: make lyra SDK=/path)"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make distclean    - Remove all generated files"
	@echo ""

setup: ## Setup project environment (installs uv + kconfiglib)
	@if [ ! -d ".venv" ]; then \
		UV=$$(command -v uv 2>/dev/null); \
		if [ -z "$$UV" ]; then \
			if [ $(IS_WINDOWS) -eq 1 ]; then UV_L="$(USERPROFILE)\.local\bin\uv.exe"; else UV_L="$(HOME)/.local/bin/uv"; fi; \
			if [ ! -f "$$UV_L" ]; then \
				echo "uv not found. Trying to install uv or fallback to python3..."; \
				$(UV_INSTALL_CMD) 2>/dev/null || true; \
			fi; \
			if [ -f "$$UV_L" ]; then UV="$$UV_L"; fi; \
		fi; \
		if [ -n "$$UV" ]; then \
			"$$UV" venv --quiet .venv 2>/dev/null || true; \
		elif command -v python3 >/dev/null 2>&1; then \
			python3 -m venv .venv 2>/dev/null || true; \
		fi; \
	fi
	@if [ -d ".venv" ]; then \
		UV=$$(command -v uv 2>/dev/null); \
		if [ -z "$$UV" ]; then \
			if [ $(IS_WINDOWS) -eq 1 ]; then UV="$(USERPROFILE)\.local\bin\uv.exe"; else UV="$(HOME)/.local/bin/uv"; fi; \
		fi; \
		if [ -f "$$UV" ]; then \
			"$$UV" pip install --quiet -r requirements.txt 2>/dev/null || true; \
		elif [ -f "$(VENV_PYTHON)" ]; then \
			"$(VENV_PYTHON)" -m pip install --quiet -r requirements.txt 2>/dev/null || true; \
		fi; \
	fi
	@if [ -e ".git" ]; then \
		UNINITIALIZED=$$(git submodule status | grep "^-" | awk '{print $$2}'); \
		if [ -n "$$UNINITIALIZED" ]; then \
			echo "Initializing missing submodules..."; \
			for sub in $$UNINITIALIZED; do \
				echo "  Syncing $$sub..."; \
				GIT_TERMINAL_PROMPT=0 git submodule update --init "$$sub" || echo "  [!] Warning: Could not sync $$sub (likely private)"; \
				if [ -d "$$sub/.git" ] || [ -f "$$sub/.git" ]; then \
					echo "  Recursing into $$sub..."; \
					GIT_TERMINAL_PROMPT=0 git submodule update --init --recursive "$$sub" 2>/dev/null || echo "  [!] Note: Nested submodules in $$sub partially failed"; \
				fi; \
			done; \
		fi; \
	fi
	@echo "Environment ready."
	@echo "Checking CMake version..."
	@cmake_version=$$(cmake --version | head -n1 | grep -oE '[0-9]+\.[0-9]+'); \
	 if [ $$(echo "$$cmake_version < 3.16" | bc -l) -eq 1 ]; then \
	   echo "Error: CMake 3.16+ is required. Found $$cmake_version"; \
	   exit 1; \
	 fi

submodules:
	@echo "Initializing and updating submodules..."
	@git submodule update --init --recursive
	@echo "Submodules updated."

menuconfig: setup
	@echo "Starting Menuconfig..."
	@bash $(SCRIPT_DIR)/menuconfig.sh

config: setup ## Generate headers/cmake files from .config (supports overrides via SET="SYM1=VAL1 SYM2=VAL2")
	@echo "Generating configuration files..."
	@if [ ! -f .config ]; then echo "Warning: .config missing, using defaults."; fi
	@set_args=""; \
	for pair in $(SET); do \
		sym=$${pair%%=*}; \
		val=$${pair#*=}; \
		set_args="$$set_args --set $$sym $$val"; \
	done; \
	$(PYTHON) $(SCRIPT_DIR)/kconfig.py --kconfig Kconfig --config .config --out-header autoconf.h --out-cmake kconfig.cmake $$set_args

setconfig: setup ## Set a single configuration value (Usage: make setconfig SYM=SYMBOL_NAME VAL=VALUE)
	@if [ -z "$(SYM)" ] || [ -z "$(VAL)" ]; then \
		echo "Usage: make setconfig SYM=SYMBOL_NAME VAL=VALUE"; \
		exit 1; \
	fi
	@$(PYTHON) $(SCRIPT_DIR)/kconfig.py --kconfig Kconfig --config .config --out-header autoconf.h --out-cmake kconfig.cmake --set $(SYM) $(VAL)

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
	@effective_build_type="$(BUILD_TYPE)"; \
	if [ "$$effective_build_type" = "Release" ] && grep -q '^CONFIG_OPENHD_DEBUG=y$$' .config 2>/dev/null; then \
		effective_build_type=Debug; \
	fi; \
	echo "Starting Build ($$effective_build_type) using $(CMAKE_GEN)..."
	@if [ -f .artosyn_env ]; then . ./.artosyn_env && export EXTRA_ARTOSYN="-DARTOSYN_SDK_ROOT=\$$ARTOSYN_SDK_ROOT -DARTOSYN_SDK_LIB=\$$ARTOSYN_SDK_LIB -DARTOSYN_SDK_DAEMON=\$$ARTOSYN_SDK_DAEMON -DARTOSYN_SDK_TUNTAP=\$$ARTOSYN_SDK_TUNTAP"; fi; \
	effective_build_type="$(BUILD_TYPE)"; \
	if [ "$$effective_build_type" = "Release" ] && grep -q '^CONFIG_OPENHD_DEBUG=y$$' .config 2>/dev/null; then \
		effective_build_type=Debug; \
	fi; \
	cmake -S "$(OPENHD_SOURCE_DIR)" -B "$(BUILD_DIR)" -G "$(CMAKE_GEN)" -DCMAKE_BUILD_TYPE=$$effective_build_type -DPython3_EXECUTABLE=$(realpath $(PYTHON)) -DCMAKE_VERBOSE_MAKEFILE=ON $(EXTRA_CMAKE) $$EXTRA_ARTOSYN && \
	cmake --build "$(BUILD_DIR)" -j$(NPROC)

test: build
	@echo "Building unit test targets..."
	@cmake --build "$(BUILD_DIR)" --target test_openhd_util test_config test_logging -j$(NPROC)
	@echo "Running unit tests..."
	@"$(BUILD_DIR)"/ohd_common/test_openhd_util && "$(BUILD_DIR)"/ohd_common/test_config && "$(BUILD_DIR)"/ohd_common/test_logging

coverage: config resolve_artosyn
	@echo "Building with Coverage analysis..."
	@if [ -f .artosyn_env ]; then . ./.artosyn_env && export EXTRA_ARTOSYN="-DARTOSYN_SDK_ROOT=\$$ARTOSYN_SDK_ROOT -DARTOSYN_SDK_LIB=\$$ARTOSYN_SDK_LIB -DARTOSYN_SDK_DAEMON=\$$ARTOSYN_SDK_DAEMON -DARTOSYN_SDK_TUNTAP=\$$ARTOSYN_SDK_TUNTAP"; fi; \
	cmake -S "$(OPENHD_SOURCE_DIR)" -B "$(BUILD_DIR)" -G "$(CMAKE_GEN)" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPython3_EXECUTABLE=$(realpath $(PYTHON)) $(EXTRA_CMAKE) $$EXTRA_ARTOSYN && \
	cmake --build "$(BUILD_DIR)" --target test_openhd_util test_config test_logging -j$(NPROC)
	@echo "Running tests for coverage..."
	@"$(BUILD_DIR)"/ohd_common/test_openhd_util || true
	@"$(BUILD_DIR)"/ohd_common/test_config || true
	@"$(BUILD_DIR)"/ohd_common/test_logging || true
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
		gst_perf_plugin="$$SDKTARGETSYSROOT/usr/lib/gstreamer-1.0/libgstperf.so" && \
		$(MAKE) build EXTRA_CMAKE="-DCMAKE_DISABLE_FIND_PACKAGE_SDL2=TRUE" && \
		echo "Validating Orqa binary..." && \
		readelf -h $(BUILD_DIR)/openhd | grep -q 'Machine:.*AArch64' && \
		readelf -d $(BUILD_DIR)/openhd | tee $(BUILD_DIR)/openhd-needed.txt | grep -q 'libPocoFoundation.so' && \
		test -f "$$gst_perf_plugin" && \
		readelf -h "$$gst_perf_plugin" | grep -q 'Machine:.*AArch64' && \
		readelf -d "$$gst_perf_plugin" | tee $(BUILD_DIR)/gst-perf-needed.txt >/dev/null && \
		strings "$$gst_perf_plugin" | tee $(BUILD_DIR)/gst-perf-strings.txt | grep -q 'bitrate-interval' && \
		grep -q 'on-bitrate' $(BUILD_DIR)/gst-perf-strings.txt && \
		! readelf -d $(BUILD_DIR)/openhd | grep -q 'libSDL2' || (echo "Validation failed (check SDL2 linkage)!"; exit 1); \
	else \
		echo "Orqa SDK not found at $(ORQA_SDK)"; exit 1; \
	fi

orqa-tools:
	@echo "Building Orqa tools (iwconfig)..."
	@bash scripts/build_orqa_iwconfig.sh $(ORQA_SDK) $(BUILD_DIR)/iwconfig

mpp-setup:
	@echo "Installing Rockchip MPP into Sysroot..."
	@if [ -z "$(SYSROOT)" ]; then echo "Usage: make mpp-setup ARCH=arm64 SYSROOT=/path"; exit 1; fi
	@bash scripts/install_rockchip_mpp_sysroot.sh arm64 $(SYSROOT)

portable:
	@echo "Starting Portable Cross-build for $(ARCH)..."
	@if [ -z "$(ARCH)" ] || [ -z "$(SYSROOT)" ]; then echo "Usage: make portable ARCH=arm64 SYSROOT=/path"; exit 1; fi
	@cp configs/release_defconfig .config
	@case "$(ARCH)" in \
		arm64) triplet=aarch64-linux-gnu ;; \
		armhf) triplet=arm-linux-gnueabihf ;; \
		*) echo "Unsupported ARCH=$(ARCH). Use ARCH=arm64 or ARCH=armhf."; exit 1 ;; \
	esac; \
	POCO_CFG=$$(find $(SYSROOT)/usr -name PocoConfig.cmake -print -quit) && \
	SDL2_CFG=$$(find $(SYSROOT)/usr \( -name sdl2-config.cmake -o -name SDL2Config.cmake \) -print -quit) && \
	test -n "$$POCO_CFG" || { echo "Could not find PocoConfig.cmake under $(SYSROOT)/usr"; exit 1; }; \
	test -n "$$SDL2_CFG" || { echo "Could not find an SDL2 CMake config under $(SYSROOT)/usr"; exit 1; }; \
	POCO_DIR=$$(dirname "$$POCO_CFG") && \
	SDL2_DIR=$$(dirname "$$SDL2_CFG") && \
	 export OPENHD_ARTOSYN_FORCE_SOURCE_BUILD=1 OPENHD_SYSROOT="$(SYSROOT)" OPENHD_CROSS_TRIPLET="$$triplet" && \
	 $(MAKE) build SET="OPENHD_ENABLE_ARTOSYN=y" EXTRA_CMAKE="-DCMAKE_TOOLCHAIN_FILE=$(OPENHD_SOURCE_DIR)/cmake/portable-linux-toolchain.cmake -DPoco_DIR=$$POCO_DIR -DSDL2_DIR=$$SDL2_DIR -DOPENHD_MPP_ONLY=OFF -DENABLE_LIBCAMERA=OFF" && \
	 echo "Generating cross-build manifest..." && \
	 (echo "ARCH=$(ARCH)"; echo "SYSROOT=$(SYSROOT)"; echo "OPENHD_CROSS_TRIPLET=$$triplet"; [ -f .artosyn_env ] && cat .artosyn_env) > $(BUILD_DIR)/openhd-cross.env

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
	@mkdir -p OpenHD/ohd_common/build_check && cd OpenHD/ohd_common/build_check && cmake .. -DCMAKE_BUILD_TYPE=Debug -DPython3_EXECUTABLE=$(realpath $(PYTHON)) && cmake --build .
	@mkdir -p OpenHD/ohd_interface/build_check && cd OpenHD/ohd_interface/build_check && cmake .. -DCMAKE_BUILD_TYPE=Debug -DPython3_EXECUTABLE=$(realpath $(PYTHON)) && cmake --build .
	@mkdir -p OpenHD/ohd_telemetry/build_check && cd OpenHD/ohd_telemetry/build_check && cmake .. -DCMAKE_BUILD_TYPE=Debug -DPython3_EXECUTABLE=$(realpath $(PYTHON)) && cmake --build .
	@mkdir -p OpenHD/ohd_video/build_check && cd OpenHD/ohd_video/build_check && cmake .. -DCMAKE_BUILD_TYPE=Debug -DPython3_EXECUTABLE=$(realpath $(PYTHON)) && cmake --build .

x21: config ## Build for X21 platform (Usage: make x21 SDK=/path/to/sdk [OUT=/path])
	@if [ -z "$(SDK)" ]; then echo "Usage: make x21 SDK=/path/to/x21-sdk [OUT=/path]"; exit 1; fi
	@$(PYTHON) $(SCRIPT_DIR)/package_component.py --platform x21 --sdk "$(SDK)" --out "$(if $(OUT),$(OUT),$(BUILD_DIR)/x21)"

luckfox: config ## Build for Luckfox platform (Usage: make luckfox SDK=/path/to/luckfox-sdk [OUT=/path])
	@if [ -z "$(SDK)" ]; then echo "Usage: make luckfox SDK=/path/to/luckfox-sdk [OUT=/path]"; exit 1; fi
	@$(PYTHON) $(SCRIPT_DIR)/package_component.py --platform luckfox --sdk "$(SDK)" --out "$(if $(OUT),$(OUT),$(BUILD_DIR)/luckfox)"

lyra: config ## Build for Lyra ground platform (Usage: make lyra SDK=/path/to/lyra-sdk [OUT=/path])
	@if [ -z "$(SDK)" ]; then echo "Usage: make lyra SDK=/path/to/lyra-sdk [OUT=/path]"; exit 1; fi
	@$(PYTHON) $(SCRIPT_DIR)/package_component.py --platform lyra --sdk "$(SDK)" --out "$(if $(OUT),$(OUT),$(BUILD_DIR)/lyra)"

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
