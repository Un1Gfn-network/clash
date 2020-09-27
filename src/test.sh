#!/bin/bash -e

function err {
  R="$?"
  if [ "$R" -ne 0 ]; then
    echo "err @ '$2'"
    exit 2
  fi
  echo "OK"
  exit 0
}
trap 'err "${BASH_SOURCE[0]}" "$BASH_COMMAND"' EXIT
trap 'exit 1' SIGINT

test "$(whoami)" = "root"

set +e
ip a f tunT
ip l s tunT down
ip l d tunT
set -e

ip r f t main
ip r a 192.168.1.0/24 dev wlp2s0 proto dhcp scope link src 192.168.1.223 metric 303
ip r a default via 192.168.1.1 dev wlp2s0 proto dhcp src 192.168.1.223 metric 303

sudo -u darren make

./clash_tun.out rixcloud
# valgrind ./route_ioctl.out
# valgrind ./route.out
