#
# Makefile for net-snmp
#

VER = net-snmp-5.7.1

NET_SNMP_CFG = --prefix= 

ifdef CONFIG_CPU_BIG_ENDIAN
NET_SNMP_CFG += --with-endianness=big
else
NET_SNMP_CFG += --with-endianness=little
endif

ifdef CONFIG_USER_NETSNMP_BUILDSTATIC
NET_SNMP_LIB_EXT = a
NET_SNMP_EXE_DIR = 
else
NET_SNMP_LIB_EXT = so
NET_SNMP_EXE_DIR = .libs/
endif

ifdef CONFIG_USER_NETSNMP_SNMPD
ifdef CONFIG_USER_NETSNMP_SNMPD_MINI
NET_SNMP_CFG += --enable-mini-agent
endif
else
NET_SNMP_CFG += --disable-agent
endif

ifndef CONFIG_USER_NETSNMP_APPS
NET_SNMP_CFG += --disable-applications
endif

ifndef CONFIG_USER_NETSNMP_MANUALS
NET_SNMP_CFG += --disable-manuals
endif

ifndef CONFIG_USER_NETSNMP_SCRIPTS
NET_SNMP_CFG += --disable-scripts
endif

ifdef CONFIG_USER_NETSNMP_MIBS
ifndef CONFIG_USER_NETSNMP_MIBLOADING
NET_SNMP_CFG += --disable-mib-loading
endif
else
NET_SNMP_CFG += --disable-mibs --disable-mib-loading
endif

ifdef CONFIG_USER_NETSNMP_NOV1
NET_SNMP_CFG += --disable-snmpv1
endif

ifdef CONFIG_USER_NETSNMP_NOV2C
NET_SNMP_CFG += --disable-snmpv2c
endif

ifdef CONFIG_USER_NETSNMP_IPV6
ifndef CONFIG_IPV6
$(error CONFIG_IPV6 not defined)
else
NET_SNMP_CFG += --enable-ipv6
endif
endif

ifdef CONFIG_USER_NETSNMP_NOTRANSPORTS
NET_SNMP_CFG += --with-out-transports=$(CONFIG_USER_NETSNMP_EXTRANSPORTS)
endif

ifdef CONFIG_USER_NETSNMP_OPENSSL
ifndef CONFIG_LIB_LIBSSL
$(error CONFIG_LIB_LIBSSL not defined)
else
NET_SNMP_CFG += --with-openssl=$(ROOTDIR)/lib
endif
else
NET_SNMP_CFG += --without-openssl
endif

ifdef CONFIG_USER_NETSNMP_OVRDEFAULTS
NET_SNMP_CFG += --with-default-snmp-version=$(CONFIG_USER_NETSNMP_DEFVERSION)
NET_SNMP_CFG += --with-sys-contact=$(CONFIG_USER_NETSNMP_DEFSYSCONTACT)
NET_SNMP_CFG += --with-sys-location=$(CONFIG_USER_NETSNMP_DEFSYSLOCATION)
NET_SNMP_CFG += --with-logfile=$(CONFIG_USER_NETSNMP_DEFLOGFILE)
NET_SNMP_CFG += --with-persistent-directory=$(CONFIG_USER_NETSNMP_DEFPERSISDIR)
NET_SNMP_CFG += --with-enterprise-oid=$(CONFIG_USER_NETSNMP_DEFENTERPRISEOID)
else
NET_SNMP_CFG += --with-defaults
endif

LDFLAGS += -Bstatic

ifdef CONFIG_USER_NETSNMP_ADDMIBS
NET_SNMP_CFG += --with-mib-modules=$(CONFIG_USER_NETSNMP_ADDITIONALMIBS)
endif

all: build/Makefile
	$(MAKE) xEXT=${NET_SNMP_LIB_EXT} xDIR=${NET_SNMP_EXE_DIR} -C build

build/Makefile: Makefile
	rm -fr build
	mkdir build
	chmod u+x $(VER)/configure
	( \
		cd build; \
		CC="$(CC) $(CFLAGS)" LDFLAGS="$(LDFLAGS)" LIBS="$(LDLIBS)" \
			sh ../$(VER)/configure $(CONFIGURE_OPTS) $(NET_SNMP_CFG); \
	)

romfs:
ifndef CONFIG_USER_NETSNMP_BUILDSTATIC
	$(ROMFSINST) -e CONFIG_FMT_USE_FDPIC_ELF build/snmplib/.libs/libnetsnmp.so.30.0.1 /lib/libnetsnmp.so.30
	$(ROMFSINST) -e CONFIG_FMT_USE_FDPIC_ELF build/agent/.libs/libnetsnmpagent.so.30.0.1 /lib/libnetsnmpagent.so.30
	$(ROMFSINST) -e CONFIG_FMT_USE_FDPIC_ELF build/agent/.libs/libnetsnmpmibs.so.30.0.1 /lib/libnetsnmpmibs.so.30
	$(ROMFSINST) -e CONFIG_FMT_USE_FDPIC_ELF build/agent/helpers/.libs/libnetsnmphelpers.so.30.0.1 /lib/libnetsnmphelpers.so.30
ifdef CONFIG_USER_NETSNMP_APPS_TRAPD
	$(ROMFSINST) -e CONFIG_FMT_USE_FDPIC_ELF build/apps/.libs/libnetsnmptrapd.so.5.2.1 /lib/libnetsnmptrapd.so.5
endif
endif
ifdef CONFIG_USER_NETSNMP_SNMPD
	$(ROMFSINST) build/agent/${NET_SNMP_EXE_DIR}snmpd /bin/snmpd
endif
ifdef CONFIG_USER_NETSNMP_APPS_BULKGET
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpbulkget /bin/snmpbulkget
endif
ifdef CONFIG_USER_NETSNMP_APPS_BULKWALK
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpbulkwalk /bin/snmpbulkwalk
endif
ifdef CONFIG_USER_NETSNMP_APPS_DELTA
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpdelta /bin/snmpdelta
endif
ifdef CONFIG_USER_NETSNMP_APPS_DF
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpdf /bin/snmpdf
endif
ifdef CONFIG_USER_NETSNMP_APPS_GET
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpget /bin/snmpget
endif
ifdef CONFIG_USER_NETSNMP_APPS_GETNEXT
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpgetnext /bin/snmpgetnext
endif
ifdef CONFIG_USER_NETSNMP_APPS_SET
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpset /bin/snmpset
endif
ifdef CONFIG_USER_NETSNMP_APPS_STATUS
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpstatus /bin/snmpstatus
endif
ifdef CONFIG_USER_NETSNMP_APPS_TABLE
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmptable /bin/snmptable
endif
ifdef CONFIG_USER_NETSNMP_APPS_TRANSLATE
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmptranslate /bin/snmptranslate
endif
ifdef CONFIG_USER_NETSNMP_APPS_TRAP
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmptrap /bin/snmptrap
endif
ifdef CONFIG_USER_NETSNMP_APPS_TRAPD
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmptrapd /bin/snmptrapd
endif
ifdef CONFIG_USER_NETSNMP_APPS_USM
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpusm /bin/snmpusm
endif
ifdef CONFIG_USER_NETSNMP_APPS_VACM
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpvacm /bin/snmpvacm
endif
ifdef CONFIG_USER_NETSNMP_APPS_WALK
	$(ROMFSINST) build/apps/${NET_SNMP_EXE_DIR}snmpwalk /bin/snmpwalk
endif
ifdef CONFIG_USER_NETSNMP_APPS_NETSTAT
	$(ROMFSINST) build/apps/snmpnetstat/${NET_SNMP_EXE_DIR}snmpnetstat /bin/snmpnetstat
endif
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_FLATFSD -s config /etc/snmp

clean:
	rm -rf build
