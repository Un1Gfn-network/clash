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

for tun in "tun0" "tunT"; do
  set +e
  ip a f "$tun"
  ip l s "$tun" down
  ip l d "$tun"
  set -e
done

ip r f t main
ip r a 192.168.1.0/24 dev wlp2s0 proto dhcp scope link src 192.168.1.223 metric 303
ip r a default via 192.168.1.1 dev wlp2s0 proto dhcp src 192.168.1.223 metric 303

resolvectl llmnr      wlp2s0 no
resolvectl mdns       wlp2s0 no
resolvectl dnsovertls wlp2s0 no
resolvectl dnssec     wlp2s0 no
resolvectl dns        wlp2s0 ""
resolvectl dns        wlp2s0 "127.127.127.127"
resolvectl flush-caches
resolvectl dns        wlp2s0 "8.8.8.8" "8.8.4.4"

sudo -u darren make

./clash_tun.out rixcloud
# gdb --args ./clash_tun.out rixcloud
# valgrind ./clash_tun.out rixcloud
# valgrind ./route_ioctl.out
# valgrind ./route.out
