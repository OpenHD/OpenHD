# OpenHD convenience build entry point.
#
# Native builds are configured through Kconfig (.config, see `make menuconfig`)
# and handed to CMake as an initial cache, so the CMake option names and
# defaults stay the single source of truth. Cross and platform targets wrap
# the existing scripts unchanged; those scripts choose their own feature set.
#
# Plain `cmake -S OpenHD` builds (Buildroot, Yocto, CI scripts) do not depend
# on anything in this file.

SHELL := /bin/bash
.DEFAULT_GOAL := build

BUILD_DIR   ?= OpenHD/build
NPROC       ?= $(shell nproc 2>/dev/null || echo 4)
CMAKE_ARGS  ?=
GENERATOR   ?= $(if $(shell command -v ninja 2>/dev/null),Ninja,Unix Makefiles)
CONFIG_FILE := .config
CACHE_FILE   = $(BUILD_DIR)/openhd-config.cmake

VENV_PYTHON := .venv/bin/python3
PYTHON      ?= $(if $(wildcard $(VENV_PYTHON)),$(VENV_PYTHON),python3)
KCONFIG      = $(PYTHON) scripts/kconfig.py --config $(CONFIG_FILE)

UNIT_TESTS  := test_openhd_util test_config test_logging

.PHONY: help setup menuconfig config build air ground debug test \
        install clean distclean orqa portable rk3588 x21 luckfox lyra

help:
	@echo "OpenHD build"
	@echo ""
	@echo "Native (configured by .config):"
	@echo "  make setup             create .venv with kconfiglib"
	@echo "  make menuconfig        edit .config interactively"
	@echo "  make <name>_defconfig  load configs/<name>_defconfig into .config"
	@echo "  make [build]           configure and build into $(BUILD_DIR)"
	@echo "  make air|ground|debug  load the matching defconfig and build"
	@echo "  make test              build and run the ohd_common unit tests"
	@echo "  make install           cmake --install $(BUILD_DIR)"
	@echo ""
	@echo "Cross/platform (wrap scripts/, ignore .config):"
	@echo "  make orqa SDK=<orqa-sdk> [OUT=<build-dir>]"
	@echo "  make portable ARCH=arm64|armhf SYSROOT=<sysroot> [OUT=<build-dir>]"
	@echo "  make rk3588"
	@echo "  make x21|luckfox|lyra SDK=<sdk> OUT=<output-dir>"
	@echo ""
	@echo "Variables: BUILD_DIR NPROC GENERATOR CMAKE_ARGS PYTHON"

setup:
	@if [ ! -x "$(VENV_PYTHON)" ]; then \
		if command -v uv >/dev/null 2>&1; then uv venv --quiet .venv; \
		else python3 -m venv .venv; fi; \
	fi
	@if command -v uv >/dev/null 2>&1; then \
		uv pip install --quiet --python "$(VENV_PYTHON)" -r requirements.txt; \
	else \
		"$(VENV_PYTHON)" -m pip install --quiet -r requirements.txt; \
	fi
	@echo "kconfiglib installed into .venv"

menuconfig:
	@KCONFIG_CONFIG=$(CONFIG_FILE) srctree=$(CURDIR) $(PYTHON) -m menuconfig Kconfig

%_defconfig:
	@$(KCONFIG) defconfig configs/$@

config:
	@$(KCONFIG) cmake $(CACHE_FILE)

build: config
	@set -eo pipefail; \
	extra=(); \
	if [ "$$($(KCONFIG) get MAKE_ARTOSYN)" = y ]; then \
		source OpenHD/scripts/resolve_artosyn_sdk.sh; \
		resolve_artosyn_sdk; \
		extra+=("-DARTOSYN_SDK_ROOT=$${ARTOSYN_SDK_ROOT:-}" \
		        "-DARTOSYN_SDK_LIB=$${ARTOSYN_SDK_LIB:-}" \
		        "-DARTOSYN_SDK_DAEMON=$${ARTOSYN_SDK_DAEMON:-}" \
		        "-DARTOSYN_SDK_TUNTAP=$${ARTOSYN_SDK_TUNTAP:-}"); \
	fi; \
	if [ ! -f "$(BUILD_DIR)/CMakeCache.txt" ]; then extra+=(-G "$(GENERATOR)"); fi; \
	cmake -S OpenHD -B "$(BUILD_DIR)" -C "$(CACHE_FILE)" "$${extra[@]}" $(CMAKE_ARGS); \
	cmake --build "$(BUILD_DIR)" --parallel $(NPROC)

air: default_defconfig
	@$(MAKE) --no-print-directory build

ground: ground_defconfig
	@$(MAKE) --no-print-directory build

debug: debug_defconfig
	@$(MAKE) --no-print-directory build

test: build
	@cmake --build "$(BUILD_DIR)" --parallel $(NPROC) --target $(UNIT_TESTS)
	@set -e; for t in $(UNIT_TESTS); do echo "== $$t"; "$(BUILD_DIR)/ohd_common/$$t"; done

install:
	@cmake --install "$(BUILD_DIR)"

orqa:
	@test -n "$(SDK)" || { echo "Usage: make orqa SDK=<orqa-sdk> [OUT=<build-dir>]"; exit 1; }
	@bash ./scripts/build_orqa_cross.sh "$(SDK)" $(if $(OUT),"$(OUT)")

portable:
	@test -n "$(ARCH)" -a -n "$(SYSROOT)" || { echo "Usage: make portable ARCH=arm64|armhf SYSROOT=<sysroot> [OUT=<build-dir>]"; exit 1; }
	@bash ./scripts/build_portable_cross.sh "$(ARCH)" "$(SYSROOT)" $(if $(OUT),"$(OUT)")

rk3588:
	@cd OpenHD && bash ./build_rk3588_cross.sh

x21 luckfox lyra:
	@test -n "$(SDK)" -a -n "$(OUT)" || { echo "Usage: make $@ SDK=<sdk> OUT=<output-dir>"; exit 1; }
	@bash ./scripts/build_$@_$(if $(filter lyra,$@),ground_)component.sh "$(SDK)" "$(OUT)"

clean:
	rm -rf "$(BUILD_DIR)"

distclean: clean
	rm -f $(CONFIG_FILE) $(CONFIG_FILE).old
	rm -rf .venv
