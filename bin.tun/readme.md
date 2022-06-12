[SO answer ioctl/netlink interface up/down](https://stackoverflow.com/a/63950398)

ambrop72/badvpn/tun2socks [wiki](https://github.com/ambrop72/badvpn/wiki/Tun2socks)

gitbook/lancellc [config.yaml](https://lancellc.gitbook.io/clash/)\
gitbook/clash [RESTful API](https://clash.gitbook.io/doc/restful-api)\
githubwiki [RESTful API](https://github.com/Dreamacro/clash/wiki/external-controller-API-reference)

libcurl\
&bullet; https://curl.haxx.se/libcurl/c/[FUNCTION_NAME].html

json-c\
&bullet; [synopsis](https://github.com/json-c/json-c#using-json-c-)\
&bullet; [doc](http://json-c.github.io/json-c/json-c-current-release/doc/html/index.html)

according to [redsocks](https://github.com/darkk/redsocks/blob/master/README.md)
>Probably, the better way is to use on-device VPN daemon to intercept
traffic via [`VpnService` API for Android](https://developer.android.com/reference/android/net/VpnService.html)
and [`NETunnelProvider` family of APIs for iOS](https://developer.apple.com/documentation/networkextension).
That may require some code doing [TCP Reassembly](https://wiki.wireshark.org/TCP_Reassembly)
like [`tun2socks`](https://github.com/ambrop72/badvpn/wiki/Tun2socks).

linux 2.2+ iptables [tproxy](https://www.kernel.org/doc/html/latest/networking/tproxy.html)

http://127.0.0.1:9090/proxies

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

http://127.0.0.1:9090/proxies/GLOBAL

```
{
  ...
  "name":"GLOBAL",
  "now":"香港标准 IEPL 中继 17",
  "type":"Selector"
  ...
}
```

stop VPN (su)

```bash
killall {badvpn-,}tun2socks
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
resolvectl dns        wlp2s0 "223.5.5.5"       "223.6.6.6"
```

start VPN (su)

```bash

clash_tun

# switch to another terminal

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

# -interface 
tun2socks \
  -device tun://tun0 \
  -loglevel info \
  -proxy socks5://127.0.0.1:1080
```


## Todo

clash_tun/ioctl.c:207:  steal_flag_u16(&(ifr.ifr_flags),IFF_MULTICAST,"multicast");\
clash_tun/ioctl.c:355:  steal_flag_u32(&(i->ifa_flags),IFF_UP,"up");

`steal_flag*()` -> [Generic](https://en.cppreference.com/w/c/language/generic) `steal_flag()`

[libnl](https://www.infradead.org/~tgr/libnl/) instead of netlink.c

proc.c - inspect_proc() - wait till clash is dead\
&bullet; \__NR_pidfd_open + (pthread_create()+) \__NR_pidfd_send_signal + epoll_wait()\
&bullet; ptrace(2)\
&bullet; [netlink](https://bewareofgeek.livejournal.com/2945.html)

main.c - start_badvpn() - enforce print order btw child & parent proc w/ semaphore

[Makefile - Managing Large Projects](https://www.oreilly.com/library/view/managing-projects-with/0596006101/ch06.html)
