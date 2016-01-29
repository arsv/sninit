################################################################################
#
# sninit
#
################################################################################

SNINIT_VERSION = HEAD
SNINIT_SOURCE_DIR = $(realpath $(BR2_EXTERNAL)/../..)
SNINIT_SERVICE_DIR = $(realpath $(BR2_EXTERNAL)/../service)
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

ifeq ($(BR2_PACKAGE_SNINIT_SERVICES),y)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_ACPID), acpid)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_AT), atd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_AVAHI), avahi)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_BANDWIDTHD), bandwidthd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_CHRONY), chrony)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_CONNMAN), connman)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_DBUS), dbus)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_DCRON), dcron)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_DNSMASQ), dnsmasq)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_DROPBEAR), dropbear)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_EJABBERD), ejabberd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_EXIM), exim)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_HAVEGED), haveged)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_INADYN), inadyn)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_INPUT_EVENT_DAEMON), inputevd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_IRQBALANCE), irqbalance)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_KODI), kodi)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_LIBIIO), iiod)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_LIGHTTPD), lighttpd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_MONGOOSE), mongoose)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_MPD), mpd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_MYSQL), mysqld)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_NEARD), neard)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_NETATALK), netatalk)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_NETMAN), netman)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_NETPLUG), netplug)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_NGINX), nginx)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_NTP), ntpd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_OPENNTPD), openntpd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_OFONO), ofono)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_OLSR), olsrd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_POSTGRESQL), postgresql)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_PROFTPD), proftpd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_PTPD), ptpd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_PTPD2), ptpd2)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_PULSEAUDIO), pulseaudio)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_SAMBA)$(BR2_PACKAGE_SAMBA4), nmbd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_SAMBA)$(BR2_PACKAGE_SAMBA4), smbd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_SQUID), squid)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_OPENSSH), sshd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_SUPERVISORD), supervisord)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_TRIGGERHAPPY), thd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_TINYHTTPD), tinyhttpd)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_TRANSMISSION), transmission)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_TVHEADEND), tvheadend)
SNINIT_SERVICES += $(if $(BR2_PACKAGE_VSFTPD), vsftpd)
endif

SNINIT_POST_INSTALL_TARGET_HOOKS = \
	$(if $(SNINIT_SERVICES),SNINIT_INSTALL_SERVICES)

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

define SNINIT_INSTALL_SERVICES
	mkdir -p $(TARGET_DIR)/etc/rc
	$(foreach _,$(SNINIT_SERVICES),\
		cp -a $(SNINIT_SERVICE_DIR)/$_ $(TARGET_DIR)/etc/rc/$_
	)
endef

$(eval $(generic-package))

# Buildroot will try to download the files even if it does not make sense.
# Suppress that by resetting $(2)_ALL_DOWNLOADS from pkg-generic.mk
SNINIT_ALL_DOWNLOADS =
