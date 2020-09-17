#!/bin/bash -x

function err {
  echo "error on line $1: '$2'"
  echo "quit"
  /bin/false # (2/2) Exit if 'set -e'
}

trap 'err $LINENO "$BASH_COMMAND"' ERR # (1/2) Exit if 'set -e'

trap 'exit 1' SIGINT

test $(whoami) == "root"

set +e
ip a f tunT
ip l s tunT down
ip l d tunT
set -e

ip r f t main
ip r a 192.168.1.0/24 dev wlp2s0 proto dhcp scope link src 192.168.1.223 metric 303
ip r a default via 192.168.1.1 dev wlp2s0 proto dhcp src 192.168.1.223 metric 303

sudo -u darren make
sudo -u darren ./clash_tun.out rixcloud

valgrind ./route_ioctl.out
valgrind ./route.out
