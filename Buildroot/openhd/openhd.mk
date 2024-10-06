################################################################################
#
# OpenHD
#
################################################################################

# The Git repository from which to clone the source code
OPENHD_SITE = https://github.com/raphaelscholle/OpenHD.git
OPENHD_SITE_METHOD = git
OPENHD_GIT_SUBMODULES = YES

# Set the version to the latest commit of the default branch
OPENHD_VERSION = 2.6-evo

# Enable Git submodules if the project requires them
OPENHD_GIT_SUBMODULES = YES

# Subdirectory inside the Git repo, if needed (if OpenHD is not in the root)
OPENHD_SUBDIR = OpenHD

# No need to install to the staging directory, only target
OPENHD_INSTALL_STAGING = NO
OPENHD_INSTALL_TARGET = YES

# Additional configuration options for the CMake build
OPENHD_CONF_OPTS = -DENABLE_USB_CAMERAS=OFF

# List of dependencies that must be built before OpenHD
OPENHD_DEPENDENCIES = libsodium gstreamer1 gst1-plugins-base libpcap host-pkgconf

# Use Buildroot's CMake package infrastructure to handle the build
$(eval $(cmake-package))
