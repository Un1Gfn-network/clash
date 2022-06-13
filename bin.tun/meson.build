# https://mesonbuild.com/Reference-manual_functions.html#executable

executable('clash_tun',
  'ioctl.c',
  'netlink.c',
  'privilege.c',
  'status.c',
  implicit_include_directories: false,
  install: true,
  link_whole: [
    static_library('main.a',         'main.c',         implicit_include_directories: false, dependencies: [CURL]),
    static_library('bus.a',          'bus.c',          implicit_include_directories: false, dependencies: [SYSTEMD]),
    static_library('proc.a',         'proc.c',         implicit_include_directories: false, dependencies: [PROCPS]),
    static_library('profile.a',      'profile.c',      implicit_include_directories: false, dependencies: [JSONC]),
    static_library('shadowsocks2.a', 'shadowsocks2.c', implicit_include_directories: false, dependencies: [SSLIBEV]),
  ],
  link_with: [LIBCLASHSO],
)

# clash_tun_LDFLAGS = -pthread
