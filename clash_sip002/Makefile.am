bin_PROGRAMS = clash_sip002

clash_sip002_SOURCES = main.c
clash_sip002_CFLAGS = -I$(top_srcdir)/include $(LIBCRYPTO_CFLAGS) # nm -gD /usr/lib/libcrypto.so.1.1
clash_sip002_LDADD = $(top_builddir)/lib/libclash.la $(LIBCRYPTO_LIBS)
