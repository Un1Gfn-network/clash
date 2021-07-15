bin_PROGRAMS = clash_sip002

# https://stackoverflow.com/a/3168470
# nm -gD /usr/lib/libcrypto.so.1.1 | grep -i evp
# objdump -T /usr/lib/libcrypto.so.1.1 | grep -i evp
# readelf -Ws /usr/lib/libcrypto.so.1.1 | grep -i evp

clash_sip002_SOURCES = main.c
clash_sip002_CFLAGS = -I$(top_srcdir)/include $(LIBCRYPTO_CFLAGS) $(LIBQRENCODE_CFLAGS) $(READLINE_CFLAGS)
clash_sip002_LDADD = $(top_builddir)/lib/libclash.la $(LIBCRYPTO_LIBS) $(LIBQRENCODE_LIBS) $(READLINE_LIBS)
