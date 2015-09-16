################################################################################
#
# sninit
#
################################################################################

SNINIT_VERSION = HEAD
SNINIT_SITE = $(call github,arsv,sninit,$(SNINIT_VERSION))
SNINIT_LICENSE = GPLv3
SNINIT_LICENSE_FILES = COPYING

# sninit can be built with autotools-package, but the most complex
# part, SNINIT_CONFIGURE_CMDS, still needs changes to build it right
# (like setting prefix=/ instead of /usr), and the remaining
# SNINIT_*_CMDS are trivial.
# So to keep things together, generic-package is used instead.

SNINIT_CONF_OPTS = inittab=/etc/inittab initdir=/etc/rc

ifeq ($(BR2_PACKAGE_SNINIT_BUNDLED_LIBC),y)
SNINIT_CONF_OPTS += bundled
else ifeq ($(BR2_TOOLCHAIN_USES_MUSL),y)
SNINIT_CONF_OPTS += musl
else
SNINIT_CONF_OPTS += syslibc
endif

ifeq ($(BR2_PACKAGE_SNINIT_DEBUG),y)
SNINIT_CONF_OPTS += debug
endif

ifeq ($(BR2_PACKAGE_SNINIT_BUNDLED_LIBC),y)
ifeq ($(BR2_TOOLCHAIN_USES_MUSL),y)
SNINIT_CONF_OPTS += notz
else
SNINIT_CONF_OPTS += tz
endif
endif

ifneq ($(BR2_PACKAGE_SNINIT_BUILTIN_INITTAB),"")
SNINIT_CONF_OPTS += builtin=$(BR2_PACKAGE_SNINIT_BUILTIN_INITTAB)
endif

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
