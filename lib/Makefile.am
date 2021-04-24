AM_CFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libclash.la

libclash_la_CFLAGS = $(LIBCURL_CFLAGS) $(JSONC_CFLAGS) -DRESTFUL_PORT=$(RESTFUL_PORT) -I$(top_srcdir)/include
libclash_la_SOURCES = resolv.c restful.c

libclash_la_LDFLAGS = -avoid-version
libclash_la_LIBADD = $(LIBCURL_LIBS) $(JSONC_LIBS)
