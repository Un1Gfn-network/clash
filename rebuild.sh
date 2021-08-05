#!/bin/bash -e

echo

read -erp "  Have you checked 'git status --ignored' (s --ignored) in case of accidental removal? "
echo

cd "$(git rev-parse --show-toplevel)"
git clean -dfX
git status --ignored
echo
read -erp "  OK? "
echo

cd "$(git rev-parse --show-toplevel)"
autoreconf -v -i -Wall
echo

cd "$(git rev-parse --show-toplevel)"/build
../configure \
  --prefix=/usr/local \
  CFLAGS="-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline -Wshadow -D_GNU_SOURCE" \
  LIBTOOLFLAGS="-v --no-silent"
echo

cd "$(git rev-parse --show-toplevel)"/build
# make --no-print-directory all
# make all V=1
make all
echo
