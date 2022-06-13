# this flag belongs to libtool, which is not used
# libclash_la_LDFLAGS = -avoid-version

# executable(..., dependencies: [], ..., link_with: [LIBCLASHSO], ...)
# https://mesonbuild.com/Reference-manual_functions.html#shared_library
LIBCLASHSO = shared_library('clash',
  'resolv.c',
  'restful.c',
  'yaml2profile.c',
  dependencies: [JSONC, CURL, YAML],
  implicit_include_directories: false,
  install: true,
  # install_rpath
  soversion: '',
  # version,
)

# executable(..., dependencies: [LIBCLASH], ..., link_with: [], ...)
# LIBCLASH = declare_dependency(include_directories: INC, link_with: LIBCLASHSO)
