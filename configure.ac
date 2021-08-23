AC_PREREQ([2.71])
AC_INIT([clash utilities],
        m4_bregexp(m4_quote(m4_esyscmd_s([printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"])),[r[1-9][0-9]*\.[0-9a-z]+], [\&]),
        [Un1Gfn@gmail.com],
        [clash],
        [https://github.com/Un1Gfn-network/clash])
AC_CONFIG_SRCDIR([./include/libclash.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
# http://gnu-autoconf.7623.n7.nabble.com/Setting-a-default-value-for-an-AC-ARG-VAR-td21082.html
# https://github.com/haishanh/yacd/blob/master/src/index.template.ejs
# <div id="app" data-base-url="http://127.0.0.1:9090"></div>
# https://clash.gitbook.io/doc/restful-api
AC_ARG_VAR(RESTFUL_PORT,[external-controller: [9090]])
test "x$RESTFUL_PORT" = "x" && RESTFUL_PORT=9090
AC_DEFINE_UNQUOTED([RESTFUL_PORT], [$RESTFUL_PORT], [external-controller: [9090]])

# Checks for programs.
AC_PROG_CC([gcc])
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_SED

# Checks for libraries.
# AC_SUBST(LIBCURL_CFLAGS, m4_quote(m4_esyscmd_s([curl-config --cflags])))
# AC_SUBST(LIBCURL_LIBS, m4_quote(m4_esyscmd_s([curl-config --libs])))
PKG_PROG_PKG_CONFIG(1.7.3)
PKG_PREREQ(0.29.1)
PKG_CHECK_MODULES(LIBCURL, libcurl >= 7.76.0)
PKG_CHECK_MODULES(GLIB20, glib-2.0 >= 2.68.1)
PKG_CHECK_MODULES(JSONC, json-c = 0.15)
PKG_CHECK_MODULES(LIBBSD, libbsd >= 0.10.0)
PKG_CHECK_MODULES(LIBPROCPS, libprocps = 3.3.17)
PKG_CHECK_MODULES(LIBSYSTEMD, libsystemd >= 248)
PKG_CHECK_MODULES(LIBYAML, yaml-0.1 = 0.2.5)
PKG_CHECK_MODULES(SSLIBEV, shadowsocks-libev = 3.3.5)
PKG_CHECK_MODULES(LIBCRYPTO, libcrypto = 1.1.1k)
PKG_CHECK_MODULES(LIBQRENCODE, libqrencode = 4.1.1)
PKG_CHECK_MODULES(READLINE, readline = 8.1)

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h strings.h sys/ioctl.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_HEADER_ASSERT
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero dup2 inet_ntoa memset regcomp setlocale socket strdup strerror strstr])

# Automake
# -Wobsolete
# -Woverride
# -Wportability
# -Wportability-recursive
# -Wextra-portability
# -Wsyntax
# -Wunsupported
# -Wall
# -Werror
AM_INIT_AUTOMAKE([
  foreign
  no-dist
  no-installinfo
  no-installman
  nostdinc
  no-texinfo.tex
  subdir-objects
  1.16.3
  -Wall
])
AM_PROG_AR
AM_SILENT_RULES([yes])

# Libtool
LT_PREREQ([2.4.6])
LT_INIT([shared disable-static])

# Doxygen
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN"; then
    AC_MSG_ERROR([doxygen not found])
    /bin/false
    exit -1
fi
AC_CHECK_PROGS([DOT], [dot])
if test -z "$DOT"; then
    AC_MSG_ERROR([graphviz/dot not found])
    /bin/false
    exit -1
fi
AC_CHECK_PROGS([PDF2SVG], [pdf2svg])
if test -z "$PDF2SVG"; then
    AC_MSG_ERROR([pdf2svg not found (for HTML_FORMULA_FORMAT)])
    /bin/false
    exit -1
fi

# Output
AC_CONFIG_FILES([
  Makefile
  include/Makefile
  lib/Makefile
  clash_misc/Makefile
  clash_sip002/Makefile
  clash_tun/Makefile
  doc/Makefile
  doc/Doxyfile
])
AC_OUTPUT

# Kill rpath
AC_MSG_RESULT([__file__:__line__: removing rpath from libtool...])
sed -i.old s/'^hardcode_libdir_flag_spec.*$'/'hardcode_libdir_flag_spec="-D__LIBTOOL_IS_A_FOOL__"'/g libtool
diff -u0 libtool.old libtool
test "$?" -eq 1 || (exit -1)
rm -v libtool.old
