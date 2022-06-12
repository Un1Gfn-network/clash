# https://mesonbuild.com/Reference-manual_functions.html#executable
executable('clash_tun',
  'bus.c',
  'ioctl.c',
  'main.c',
  'netlink.c',
  'privilege.c',
  'proc.c',
  'profile.c',
  'shadowsocks2.c',
  'status.c',
  dependencies: [JSONC, CURL, SYSTEMD, PROCPS, SSLIBEV],  # BSD
  implicit_include_directories: false,
  install: true,
  link_with: [LIBCLASHSO],
)
