###################################################################
# OpenHD
#
# Licensed under the GNU General Public License (GPL) Version 3.
#
# This software is provided "as-is," without warranty of any kind, express or
# implied, including but not limited to the warranties of merchantability,
# fitness for a particular purpose, and non-infringement. For details, see the
# full license in the LICENSE file provided with this source code.
#
# Non-Military Use Only:
# This software and its associated components are explicitly intended for
# civilian and non-military purposes. Use in any military or defense
# applications is strictly prohibited unless explicitly and individually
# licensed otherwise by the OpenHD Team.
#
# Contributors:
# A full list of contributors can be found at the OpenHD GitHub repository:
# https://github.com/OpenHD
#
# © OpenHD, All Rights Reserved.
################################################################################
$(info Building the OpenHD package...)

# The Git repository from which to clone the source code
OPENHD_SITE = https://github.com/openhd/OpenHD.git
OPENHD_SITE_METHOD = git
OPENHD_GIT_SUBMODULES = YES

# Set the version to the latest commit of the default branch
OPENHD_VERSION = 428a3733d17d1cb51366d4023cfa3625fb4da00a

# Enable Git submodules if the project requires them
OPENHD_GIT_SUBMODULES = YES

# Subdirectory inside the Git repo, if needed (if OpenHD is not in the root)
OPENHD_SUBDIR = OpenHD

# Install to the target system
OPENHD_INSTALL_TARGET = YES

# List of dependencies that must be built before OpenHD
OPENHD_DEPENDENCIES = poco libsodium gstreamer1 gst1-plugins-base libpcap host-pkgconf

# Additional configuration options for the CMake build
OPENHD_CONF_OPTS = \
    -DENABLE_USB_CAMERAS=OFF \
    -DARTOSYN_SDK_ROOT="$(ARTOSYN_SDK_ROOT)" \
    -DARTOSYN_SDK_LIB="$(ARTOSYN_SDK_LIB)" \
    -DCMAKE_EXE_LINKER_FLAGS="-lstdc++fs"

# Install init.d services to target
define OPENHD_INSTALL_TARGET_CMDS
    $(info OpenHD Build Directory: $(OPENHD_BUILDDIR))
    $(INSTALL) -d $(TARGET_DIR)/etc/init.d
    cp -r $(OPENHD_BUILDDIR)/../Buildroot/init.d/* $(TARGET_DIR)/etc/init.d/
    chmod +x $(TARGET_DIR)/etc/init.d/*
    $(INSTALL) -d $(TARGET_DIR)/usr/bin $(TARGET_DIR)/usr/lib
    daemon_src=""; \
    for candidate in \
        "$(ARTOSYN_SDK_ROOT)/host_drv/app/ar8030/artosyn_daemon" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/app/ar8030/ar8030_daemon" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/app/ar8030/artlinkd" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/app/ar8030/bbd" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/app/ar8030/bb_daemon" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/build/app/ar8030/artosyn_daemon" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/build/app/ar8030/ar8030_daemon" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/build/app/ar8030/artlinkd" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/build/app/ar8030/bbd" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/build/app/ar8030/bb_daemon" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/install/bin/artosyn_daemon" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/install/bin/ar8030_daemon" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/install/bin/artlinkd" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/install/bin/bbd" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/install/bin/bb_daemon"; do \
        if [ -f "$$candidate" ]; then daemon_src="$$candidate"; break; fi; \
    done; \
    if [ -z "$$daemon_src" ]; then \
        if [ -d "$(ARTOSYN_SDK_ROOT)/host_drv" ]; then \
            daemon_src="$$(find "$(ARTOSYN_SDK_ROOT)/host_drv" -type f \
                \( -iname "artosyn_daemon" -o -iname "ar8030_daemon" -o -iname "artlinkd" -o -iname "bbd" -o -iname "bb_daemon" \) \
                | head -n 1)"; \
        fi; \
    fi; \
    if [ -n "$$daemon_src" ]; then \
        cp "$$daemon_src" "$(TARGET_DIR)/usr/bin/$$(basename $$daemon_src)"; \
        chmod +x "$(TARGET_DIR)/usr/bin/$$(basename $$daemon_src)"; \
    else \
        echo "WARNING: Artosyn daemon not found in ARTOSYN_SDK_ROOT=$(ARTOSYN_SDK_ROOT)"; \
    fi; \
    for lib in $(subst ;, ,$(ARTOSYN_SDK_LIB)); do \
        if [ -f "$$lib" ] && echo "$$lib" | grep -q '\.so'; then \
            cp "$$lib" "$(TARGET_DIR)/usr/lib/"; \
        fi; \
    done; \
    for lib in \
        "$(ARTOSYN_SDK_ROOT)/host_drv/com/libcom.so" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/build/com/libcom.so" \
        "$(ARTOSYN_SDK_ROOT)/host_drv/install/bin/libcom.so"; do \
        if [ -f "$$lib" ]; then cp "$$lib" "$(TARGET_DIR)/usr/lib/"; fi; \
    done
endef

# Use Buildroot's CMake package infrastructure to handle the build
$(eval $(cmake-package))
