################################################################################
#
# Simple Poco Build (Net + Foundation Only) with Logging
#
################################################################################

POCO_VERSION = 1.13.2
POCO_SITE = $(call github,pocoproject,poco,poco-$(POCO_VERSION)-release)
POCO_LICENSE = BSL-1.0
POCO_LICENSE_FILES = LICENSE
POCO_INSTALL_STAGING = YES

# Dependencies: Include only the necessary ones for Net and Foundation
POCO_DEPENDENCIES = \
    pcre2 \
    zlib \
    expat \
    openssl 

# Log file for verbose output
POCO_BUILD_LOG = poco_build.log

# Verbose echo statements to track progress
define POCO_VERBOSE_MSG
    @echo "***************************************************************************"
    @echo "* Building Poco version $(POCO_VERSION)"
    @echo "* Including components: Foundation, Net"
    @echo "* Using configuration options: $(POCO_CONF_OPTS)"
    @echo "***************************************************************************"
endef

# Only include Foundation and Net, exclude everything else
POCO_OMIT = \
    PageCompiler \
    Data \
    Data/ODBC \
    Data/MySQL \
    Data/SQLite \
    Data/PostgreSQL \
    JSON \
    JWT \
    MongoDB \
    PDF \
    Prometheus \
    Redis \
    Util \
    XML \
    Zip

# Always include Foundation and Net components
POCO_CONF_OPTS += --minimal
$(info [INFO] Poco Foundation and Net components included)

# Disable certain features based on toolchain (e.g., no fpenvironment for uClibc)
ifeq ($(BR2_TOOLCHAIN_USES_UCLIBC),y)
POCO_CONF_OPTS += --no-fpenvironment --no-wstring
endif

# Disable fpenvironment for soft floating point configuration
ifeq ($(BR2_SOFT_FLOAT),y)
POCO_CONF_OPTS += --no-fpenvironment
endif

# Set shared library target by default
POCO_MAKE_TARGET = shared_release

# Ensure we link against atomic if necessary
POCO_LDFLAGS=$(TARGET_LDFLAGS)
ifeq ($(BR2_TOOLCHAIN_HAS_LIBATOMIC),y)
POCO_LDFLAGS += -latomic
endif

# Configure Poco with verbose output and log to file
define POCO_CONFIGURE_CMDS
    $(POCO_VERBOSE_MSG)
    (cd $(@D); $(TARGET_MAKE_ENV) ./configure \
        --config=Linux \
        --prefix=/usr \
        --ldflags="$(POCO_LDFLAGS)" \
        --omit="$(POCO_OMIT)" \
        $(POCO_CONF_OPTS) \
        --unbundled \
        --no-tests \
        --no-samples)
endef

# Use $(MAKE1) to avoid failures with highly parallel builds, and log the build process to file
define POCO_BUILD_CMDS
    @echo "[INFO] Building Poco with target $(POCO_MAKE_TARGET)"
    $(TARGET_MAKE_ENV) $(MAKE1) V=1 POCO_TARGET_OSARCH=$(ARCH) CROSS_COMPILE=$(TARGET_CROSS) \
        DEFAULT_TARGET=$(POCO_MAKE_TARGET) -C $(@D) >> $(POCO_BUILD_LOG) 2>&1
endef

# Install into staging directory, and log the installation process to file
define POCO_INSTALL_STAGING_CMDS
    @echo "[INFO] Installing Poco into the staging directory"
    $(TARGET_MAKE_ENV) $(MAKE) DESTDIR=$(STAGING_DIR) POCO_TARGET_OSARCH=$(ARCH) \
        DEFAULT_TARGET=$(POCO_MAKE_TARGET) install -C $(@D) >> $(POCO_BUILD_LOG) 2>&1
endef

# Install into target directory, and log the installation process to file
define POCO_INSTALL_TARGET_CMDS
    @echo "[INFO] Installing Poco into the target directory"
    $(TARGET_MAKE_ENV) $(MAKE) DESTDIR=$(TARGET_DIR) POCO_TARGET_OSARCH=$(ARCH) \
        DEFAULT_TARGET=$(POCO_MAKE_TARGET) install -C $(@D) >> $(POCO_BUILD_LOG) 2>&1
endef

# Evaluate the package
$(eval $(generic-package))
