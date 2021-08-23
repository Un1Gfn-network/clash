#!/bin/bash
# https://github.com/Un1Gfn/beaglebone/blob/master/Documentation/Framebuffer.rst

function compile {
  gcc \
    -std=gnu11 -g -O0 -Wextra -Wall -Winline -Wshadow -fanalyzer \
    $(pkg-config --cflags ncurses) \
    framebuffer.c \
    $(pkg-config --libs ncurses)
}

case "$1" in

printscr|screenshot|capture)
  compile || exit 1
  rm -fv /tmp/fb.raw
  t=5
  read -erp "press <Enter>, then a screenshot will be taken in $t seconds"
  (
    sleep "$t" &>/dev/null
    cat /dev/fb0 1>/tmp/fb.raw 2>/dev/null
  ) &
  # PID="$!"
  # ps aux | grep "$PID"
  # wait "$PID"
  ./a.out
  ;;

*)
  cocmpile \
  && read -erp "compile done - press <Enter> to run " \
  && ./a.out \
  && clear \
  && reset
  ;;

esac
