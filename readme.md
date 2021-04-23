# [clash](https://github.com/Un1Gfn-network/clash)

## Misc

convert.c replace `GSList` with `SLIST`\
https://stackoverflow.com/questions/7627099/how-to-use-list-from-sys-queue-h \
[queue(3bsd)](https://man.archlinux.org/man/queue.3bsd)
/usr/include/bsd/sys/queue.h
<s>/usr/include/sys/queue.h</s>

|bin||
|-|-|
|clash_update|default_browser -> raw.yaml -> clash.db|
|clash_run   |( clash.db -> config.yaml ), Country.mmdb -> /tmp/clash/|
|clash_qr    |current_server_title() -> clash.db -> libqrencode.so -> QR code|
|clash_tun   |current_server_title() -> clash.db -> start_ss(), netlink_\*() start_badvpn()|

clash.db (implement key-value store with single-row table)

|n_rows|table_name||
|:-:|-|-|
|any|`ANNOUNCEMENTS`|extraced from loopback nodes|
|1  |`PORT`||
|1  |`CIPHER`||
|1  |`PASSWORD`||
|any|`NODES`||
|1  |`PASSWORD`||
|1  |`RAW`|original yaml|

excluding libyaml/

    tree -a -C -I libyaml\|.git\|.gitignore --dirsfirst
    find . -path ./libyaml -prune -false -o -iname '*.c' -o -iname '*.h'

[Glibc NPTL src](https://sourceware.org/git/?p=glibc.git;a=tree;f=nptl)

Change filename

    make clean
    git mv -- {old,new}.h
    git mv -- {old,new}.c
    find . -type f \( -name '*.c' -o -name '*.h' \) -exec grep -H 'old.h' {} \;
    # Check previous output before invoking sed
    find . -type f \( -name '*.c' -o -name '*.h' \) -exec sed -i 's/old.h/new.h/g' {} \;
    git diff

[SO answer ioctl/netlink interface up/down](https://stackoverflow.com/a/63950398)

ambrop72/badvpn/tun2socks
* [wiki](https://github.com/ambrop72/badvpn/wiki/Tun2socks)
* [`--socks5-udp`](https://github.com/ambrop72/badvpn/blob/master/tun2socks/tun2socks.c#:~:text=%21strcmp%28arg%2C%20%22--socks5-udp%22%29) (release too old, build badvpn-git) :heavy_check_mark:

little-endian

```
rta_len=5 

rta         rta+4
|           |
00 00 00 00 08
01
```

Clash
* [config.yaml](https://lancellc.gitbook.io/clash/)
* [external controller API](https://clash.gitbook.io/doc/restful-api) ([short](https://github.com/Dreamacro/clash/wiki/external-controller-API-reference))

libcurl
* https://curl.haxx.se/libcurl/c/SOME_FUNCTION.html

[python percent encode $1](https://unix.stackexchange.com/questions/159253/decoding-url-encoding-percent-encoding)

json-c
* [synopsis](https://github.com/json-c/json-c#using-json-c-)
* [doc](http://json-c.github.io/json-c/json-c-current-release/doc/html/index.html)

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

## D-Bus

[wpa_supplicant D-Bus API](https://w1.fi/wpa_supplicant/devel/dbus.html)

[Dunst](https://wiki.archlinux.org/index.php/Dunst)
* dunstify/notify-send

org.freedesktop.Notifications
* Execute in `d-feet` - `Session Bus` - `org.freedesktop.Notifications` - `Object path` - `/org/freedesktop/Notifications` - `Interfaces` - `org.freedesktop.Notifications` - `Methods` - `Notify()`
* [Architecture](https://wiki.ubuntu.com/NotifyOSD#Architecture)
* [C glib2/gio](https://wiki.archlinux.org/index.php/Desktop_notifications#C)

sd_bus_message_append(3) `Table 1. Item type specifiers` `TYPES STRING GRAMMAR`  
bus_message_read(3) `Table 1. Item type specifiers`

Clean-up Variable Attribute - sd_event(3)

```C
__attribute__((cleanup(sd_event_unrefp))) sd_event *event = NULL;
```

org.freedesktop.Notifications

```bash
BUSCTL="busctl --user --no-pager"
$BUSCTL list
$BUSCTL tree org.freedesktop.Notifications
$BUSCTL introspect org.freedesktop.Notifications /org/freedesktop/Notifications
$BUSCTL --xml-interface introspect org.freedesktop.Notifications /org/freedesktop/Notifications | less -SRM +%
$BUSCTL call org.freedesktop.Notifications /org/freedesktop/Notifications org.freedesktop.DBus.Peer GetMachineId
#       call SERVICE                       OBJECT                         INTERFACE                     METHOD [SIGNATURE      [ARGUMENT...]                                  ]
$BUSCTL call org.freedesktop.Notifications /org/freedesktop/Notifications org.freedesktop.Notifications Notify "susssasa{sv}i" "app_name" 0 "app_icon" "summary" "body" 0 0 0
unset -v BUSCTL
```

org.freedesktop.resolve1

```bash
su -
# input password
BUSCTL="busctl --system --no-pager"
SERVICE="org.freedesktop.resolve1"
OBJECT="/org/freedesktop/resolve1"
INTERFACE="org.freedesktop.resolve1.Manager"
METHOD="SetLinkDNS"
$BUSCTL tree $SRV
$BUSCTL introspect $SERVICE $OBJECT | less -SRM +%
$BUSCTL --xml-interface introspect $SERVICE $OBJECT | less -SRM +%
$BUSCTL call $SERVICE $OBJECT $INTERFACE $METHOD 'ia(iay)' 3   1   2 4 192 168 1 1
$BUSCTL call $SERVICE $OBJECT $INTERFACE $METHOD 'ia(iay)' 3   2   2 4 8 8 8 8       2 4 8 8 4 4
```

## Todo

[Generic steal_flag()](https://en.cppreference.com/w/c/language/generic)

[go-tun2socks](https://github.com/eycorsican/go-tun2socks)

[libnl](https://www.infradead.org/~tgr/libnl/)

proc.c - inspect_proc() - wait till clash is dead
* \__NR_pidfd_open + (pthread_create()+) \__NR_pidfd_send_signal + epoll_wait()
* ptrace(2)
* [netlink](https://bewareofgeek.livejournal.com/2945.html)

main.c - start_badvpn() - enforce print order btw child & parent proc w/ semaphore

[Makefile - Managing Large Projects](https://www.oreilly.com/library/view/managing-projects-with/0596006101/ch06.html)

## Garbage

<details><summary>garbage</summary>

<br>

[Android tcpdump](https://www.androidtcpdump.com/android-tcpdump/compile)

```bash
cd /tmp
mkdir tcpdump
cd tcpdump
# sudo pacman -Syu aarch64-linux-gnu-gcc
proxychains wget https://www.tcpdump.org/release/tcpdump-4.9.3.tar.gz
proxychains wget https://www.tcpdump.org/release/libpcap-1.9.1.tar.gz
tar xf libpcap-1.9.1.tar.gz
tar xf tcpdump-4.9.3.tar.gz

cd /tmp/tcpdump/libpcap-1.9.1/
export CC=/usr/bin/aarch64-linux-gnu-gcc
./configure --host=aarch64-linux --with-pcap=linux
make -j4

cd /tmp/tcpdump/tcpdump-4.9.3
export CC=/usr/bin/aarch64-linux-gnu-gcc
export ac_cs_linux_vers=4 # Android
export CFLAGS=-static
export CPPFLAGS=-static
export LDFLAGS=-static
./configure --host=aarch64-linux --disable-ipv6
make -j4

adb push tcpdump /sdcard/Download/
adb shell
# su
cd /data/data/com.termux/files/home/
mv /sdcard/Download/tcpdump ./
chmod +x tcpdump
./tcpdump -i any -w android.pcap
mv android.pcap /sdcard/Download
# exit
exit

cd /tmp/tcpdump
adb pull /sdcard/Download/android.pcap

```

</details>
