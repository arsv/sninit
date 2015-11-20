################################################################################
#
# sninit
#
################################################################################

SNINIT_VERSION = HEAD
SNINIT_SOURCE_DIR = $(realpath $(BR2_EXTERNAL)/../..)
SNINIT_LICENSE = GPLv3
SNINIT_LICENSE_FILES = COPYING

SNINIT_CONF_OPTS = inittab=/etc/inittab initdir=/etc/rc

ifeq ($(BR2_PACKAGE_SNINIT_BUNDLED_LIBC),y)
SNINIT_CONF_OPTS += bundled
else ifeq ($(BR2_TOOLCHAIN_USES_MUSL),y)
SNINIT_CONF_OPTS += musl
endif

ifeq ($(BR2_PACKAGE_SNINIT_DEBUG),y)
SNINIT_CONF_OPTS += debug
endif

ifneq ($(BR2_PACKAGE_SNINIT_BUILTIN_INITTAB),"")
SNINIT_CONF_OPTS += builtin=$(BR2_PACKAGE_SNINIT_BUILTIN_INITTAB)
endif

define SNINIT_EXTRACT_CMDS
	git archive --remote=$(SNINIT_SOURCE_DIR) $(SNINIT_VERSION) | tar -x -C $(@D)
endef

define SNINIT_CONFIGURE_CMDS
	(cd $(SNINIT_SRCDIR) && ./configure \
		cc="$(TARGET_CC)" \
		cflags="$(TARGET_CFLAGS)" \
		ldflags="$(TARGET_LDFLAGS)" \
		ar="$(TARGET_AR)" \
		$(SNINIT_CONF_OPTS) )
endef

define SNINIT_BUILD_CMDS
	$(MAKE) -C $(SNINIT_SRCDIR)
endef

define SNINIT_INSTALL_TARGET_CMDS
	$(MAKE) -C $(SNINIT_SRCDIR) DESTDIR=$(TARGET_DIR) install
endef

$(eval $(generic-package))

# Buildroot will try to download the files even if it does not make sense.
# Suppress that by resetting $(2)_ALL_DOWNLOADS from pkg-generic.mk
SNINIT_ALL_DOWNLOADS =
