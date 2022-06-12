dnl printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
dnl r[1-9][0-9]*\.[0-9a-z]+
dnl \&
dnl Un1Gfn@gmail.com
dnl clash
dnl https://github.com/Un1Gfn-network/clash

dnl https://github.com/haishanh/yacd/blob/master/src/index.template.ejs
dnl <div id="app" data-base-url="http://127.0.0.1:9090"></div>
dnl https://clash.gitbook.io/doc/restful-api

PKG_CHECK_MODULES(LIBBSD, libbsd >= 0.10.0)
PKG_CHECK_MODULES(LIBPROCPS, libprocps = 3.3.17)
PKG_CHECK_MODULES(LIBSYSTEMD, libsystemd >= 248)
PKG_CHECK_MODULES(SSLIBEV, shadowsocks-libev = 3.3.5)
PKG_CHECK_MODULES(READLINE, readline = 8.1)

AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h strings.h sys/ioctl.h sys/socket.h unistd.h])
AC_CHECK_FUNCS([bzero dup2 inet_ntoa memset regcomp setlocale socket strdup strerror strstr])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
AC_CHECK_PROGS([DOT], [dot])
AC_CHECK_PROGS([PDF2SVG], [pdf2svg])
