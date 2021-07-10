# https://www.gnu.org/software/automake/manual/html_node/A-Library.html

# Proposal - use libtool instead of hard-coded *.a to support Windows *.lib
# (1) Remove disable-static from LT_INIT() in configure.ac
# (2) Enable  static & disable shared in lib/Makefile.am
# (3) Disable static & enable  shared in clash_tun/Makefile.am (here)
# (4) noinst_LIBRARIES = \
# 	main.la \
# 	bus.la \
# 	ioctl.la \
# 	proc.la \
# 	profile.la \
# 	shadowsocks.la

# Put libmain.a before any other lib*.a in case of the following error 
# main.c:*: undefined reference to `...'

# Bob Proulx - library name enforcement?
# https://lists.gnu.org/archive/html/automake/2004-05/msg00016.html
# clash_tun/Makefile.am:*: error: '*.a' is not a standard library name
# clash_tun/Makefile.am:*: did you mean 'lib*.a'?

noinst_LIBRARIES = \
	main.a \
	bus.a \
	ioctl.a \
	proc.a \
	profile.a \
	shadowsocks2.a

# gcc(1): Directories specified with -I options are scanned in left-to-right order
main_a_CFLAGS = -I$(top_srcdir)/include
bus_a_CFLAGS = $(LIBSYSTEMD_CFLAGS)
ioctl_a_CFLAGS = $(LIBBSD_CFLAGS)
proc_a_CFLAGS = $(LIBPROCPS_CFLAGS)
profile_a_CFLAGS = -I$(top_srcdir)/include $(JSONC_CFLAGS) $(SSLIBEV_CFLAGS)
shadowsocks2_a_CFLAGS = $(SSLIBEV_CFLAGS)

# https://www.gnu.org/software/automake/manual/html_node/Per_002dObject-Flags.html

bin_PROGRAMS = clash_tun

clash_tun_SOURCES = \
	netlink.c \
	privilege.c \
	status.c

clash_tun_LDFLAGS = -pthread

clash_tun_LDADD = $(noinst_LIBRARIES) $(top_builddir)/lib/libclash.la \
	$(JSONC_LIBS) \
	$(LIBBSD_LIBS) \
	$(LIBCURL_LIBS) \
	$(LIBPROCPS_LIBS) \
	$(LIBSYSTEMD_LIBS) \
	$(SSLIBEV_LIBS)
