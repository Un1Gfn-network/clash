---

<!-- https://www.w3schools.com/charsets/ref_utf_geometric.asp -->
&bullet; [readme.md](https://github.com/Un1Gfn-network/clash)\
**&#9656; clash_tun/readme.md**

---

[SO answer ioctl/netlink interface up/down](https://stackoverflow.com/a/63950398)

ambrop72/badvpn/tun2socks
* [wiki](https://github.com/ambrop72/badvpn/wiki/Tun2socks)
* [`--socks5-udp`](https://github.com/ambrop72/badvpn/blob/master/tun2socks/tun2socks.c#:~:text=%21strcmp%28arg%2C%20%22--socks5-udp%22%29) (release too old, build badvpn-git) :heavy_check_mark:

Clash\
&bullet; [config.yaml](https://lancellc.gitbook.io/clash/)\
&bullet; [external controller API](https://clash.gitbook.io/doc/restful-api) ([short](https://github.com/Dreamacro/clash/wiki/external-controller-API-reference))

libcurl\
&bullet; https://curl.haxx.se/libcurl/c/SOME_FUNCTION.html


json-c\
&bullet; [synopsis](https://github.com/json-c/json-c#using-json-c-)\
&bullet; [doc](http://json-c.github.io/json-c/json-c-current-release/doc/html/index.html)

According to [redsocks](https://github.com/darkk/redsocks/blob/master/README.md)
>Probably, the better way is to use on-device VPN daemon to intercept
traffic via [`VpnService` API for Android](https://developer.android.com/reference/android/net/VpnService.html)
and [`NETunnelProvider` family of APIs for iOS](https://developer.apple.com/documentation/networkextension).
That may require some code doing [TCP Reassembly](https://wiki.wireshark.org/TCP_Reassembly)
like [`tun2socks`](https://github.com/ambrop72/badvpn/wiki/Tun2socks).

[tproxy](https://www.kernel.org/doc/html/latest/networking/tproxy.html)

http://127.0.0.1:6170/proxies

```
{
  "proxies":{
    ...
    "GLOBAL":{
      ...
      "name":"GLOBAL",
      "now":"香港标准 IEPL 中继 17",
      "type":"Selector"
    },
  }
}
```

http://127.0.0.1:6170/proxies/GLOBAL

```
{
  ...
  "name":"GLOBAL",
  "now":"香港标准 IEPL 中继 17",
  "type":"Selector"
  ...
}
```

Start VPN

```bash
# Extract
make clean
make clash_tun.out
./clash_tun.out rixcloud
cat /tmp/ss-local.json

# ss-local (tty3)

# Stop clash
ss-local -v -c /tmp/ss-local.json

# tun2socks

su -

# resolvectl revert wlp2s0
resolvectl llmnr      wlp2s0 no
resolvectl mdns       wlp2s0 no
resolvectl dnsovertls wlp2s0 no
resolvectl dnssec     wlp2s0 no
resolvectl dns        wlp2s0 ""
resolvectl dns        wlp2s0 "127.127.127.127"
resolvectl flush-caches
resolvectl dns        wlp2s0 "8.8.8.8" "8.8.4.4"

ip tuntap add dev tun0 mode tun
ip link set tun0 up
ip addr flush dev tun0
ip addr add dev tun0 10.0.0.1/24
# for i in ${IP[@]}; do ip route add "$i" via "$GW"; done
ip route add "$(jq </tmp/ss-local.json '.server' | tr -d '"')" via "192.168.1.1"
ip route del default via "192.168.1.1"
ip route add default via "10.0.0.2"

# iptables -F OUTPUT
# iptables -A OUTPUT -p udp -j REJECT
# iptables -S

badvpn-tun2socks \
  --tundev tun0 \
  --netif-ipaddr 10.0.0.2 \
  --netif-netmask 255.255.255.0 \
  --socks-server-addr 127.0.0.1:1080 \
  --socks5-udp
```

Stop VPN

```bash
killall badvpn-tun2socks
killall ss-local

ip addr flush dev tun0
ip link set tun0 down
ip link del dev tun0
ip route del "$(jq </tmp/ss-local.json '.server' | tr -d '"')" via "192.168.1.1"
ip route del default via "10.0.0.2"
ip route add default via "192.168.1.1" dev wlp2s0 proto dhcp scope global src 192.168.1.223 metric 303

resolvectl llmnr      wlp2s0 no
resolvectl mdns       wlp2s0 no
resolvectl dnsovertls wlp2s0 no
resolvectl dnssec     wlp2s0 no
resolvectl dns        wlp2s0 ""
resolvectl dns        wlp2s0 "127.127.127.127"
resolvectl flush-caches
resolvectl dns        wlp2s0 "192.168.1.1"
```

## Todo
clash_tun/ioctl.c:207:  steal_flag_u16(&(ifr.ifr_flags),IFF_MULTICAST,"multicast");
clash_tun/ioctl.c:355:    steal_flag_u32(&(i->ifa_flags),IFF_UP,"up");

`steal_flag*()` -> [Generic](https://en.cppreference.com/w/c/language/generic) `steal_flag()`

[go-tun2socks](https://github.com/eycorsican/go-tun2socks)

[libnl](https://www.infradead.org/~tgr/libnl/)

proc.c - inspect_proc() - wait till clash is dead\
&bullet; \__NR_pidfd_open + (pthread_create()+) \__NR_pidfd_send_signal + epoll_wait()\
&bullet; ptrace(2)\
&bullet; [netlink](https://bewareofgeek.livejournal.com/2945.html)

main.c - start_badvpn() - enforce print order btw child & parent proc w/ semaphore

[Makefile - Managing Large Projects](https://www.oreilly.com/library/view/managing-projects-with/0596006101/ch06.html)
