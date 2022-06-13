
# https://mesonbuild.com/Reference-manual_functions.html#shared_library
LIBCLASHSO = shared_library('clash',
  'resolv.c',
  'restful.c',
  'yaml2profile.c',
  dependencies: [JSONC, CURL, YAML],
  implicit_include_directories: false,
  install: true,
)

# LIBCLASH = declare_dependency(include_directories: INC, link_with: LIBCLASHSO)

# libclash_la_LDFLAGS = -avoid-version
