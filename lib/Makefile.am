AM_CFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libclash.la

#                    config.h RESTFUL_PORT
libclash_la_CFLAGS = -I$(top_builddir) \
	$(LIBCURL_CFLAGS) \
	$(JSONC_CFLAGS) \
	-I$(top_srcdir)/include

libclash_la_SOURCES = resolv.c restful.c

libclash_la_LDFLAGS = -avoid-version
libclash_la_LIBADD = $(LIBCURL_LIBS) $(JSONC_LIBS)
