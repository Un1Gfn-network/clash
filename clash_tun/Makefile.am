# https://www.gnu.org/software/automake/manual/html_node/A-Library.html

# Fix the following error by putting libmain.a before any other lib*.a 
# main.c:*: undefined reference to `...'

noinst_LIBRARIES = \
	libmain.a \
	libbus.a \
	libioctl.a \
	libproc.a \
	libprofile.a \
	libshadowsocks.a

libmain_a_CFLAGS = -I$(top_srcdir)/include
libbus_a_CFLAGS = $(LIBSYSTEMD_CFLAGS)
libioctl_a_CFLAGS = $(LIBBSD_CFLAGS)
libproc_a_CFLAGS = $(LIBPROCPS_CFLAGS)
libprofile_a_CFLAGS = $(JSONC_CFLAGS) $(LIBYAML_CFLAGS) $(SSLIBEV_CFLAGS) -I$(top_srcdir)/include
libshadowsocks_a_CFLAGS = $(SSLIBEV_CFLAGS)

libmain_a_SOURCES        = main.c
libbus_a_SOURCES         = bus.c
libioctl_a_SOURCES       = ioctl.c
libproc_a_SOURCES        = proc.c
libprofile_a_SOURCES     = profile.c
libshadowsocks_a_SOURCES = shadowsocks.c

# https://www.gnu.org/software/automake/manual/html_node/Per_002dObject-Flags.html

bin_PROGRAMS = clash_tun

clash_tun_SOURCES = \
	file.c \
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
	$(LIBYAML_LIBS) \
	$(SSLIBEV_LIBS)
