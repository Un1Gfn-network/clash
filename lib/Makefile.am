AM_CFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libclash.la

# gcc(1): Directories specified with -I options are scanned in left-to-right order
#                    config.h RESTFUL_PORT
libclash_la_CFLAGS = -I$(top_builddir) \
	-I$(top_srcdir)/include \
	$(JSONC_CFLAGS) \
	$(LIBCURL_CFLAGS) \
	$(LIBYAML_CFLAGS)

libclash_la_SOURCES = resolv.c restful.c yaml2profile.c

libclash_la_LDFLAGS = -avoid-version
libclash_la_LIBADD = $(JSONC_LIBS) $(LIBCURL_LIBS) $(LIBYAML_LIBS)
