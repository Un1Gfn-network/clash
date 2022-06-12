
# https://stackoverflow.com/a/3168470
# nm -gD /usr/lib/libcrypto.so.1.1 | grep -i evp
# objdump -T /usr/lib/libcrypto.so.1.1 | grep -i evp
# readelf -Ws /usr/lib/libcrypto.so.1.1 | grep -i evp

# https://mesonbuild.com/Reference-manual_functions.html#executable
executable('clash_sip002',
  'main.c',
  dependencies: [CRYPTO, QRENCODE, CURL],
  implicit_include_directories: false,
  install: true,
  link_with: [LIBCLASHSO],
)
