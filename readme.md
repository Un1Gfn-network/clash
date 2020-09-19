D-Bus

[wpa_supplicant D-Bus API](https://w1.fi/wpa_supplicant/devel/dbus.html)

[Dunst](https://wiki.archlinux.org/index.php/Dunst)
* dunstify/notify-send

org.freedesktop.Notifications'
* Execute in `d-feet` - `Session Bus` - `org.freedesktop.Notifications` - `Object path` - `/org/freedesktop/Notifications` - `Interfaces` - `org.freedesktop.Notifications` - `Methods` - `Notify()`
* [Architecture](https://wiki.ubuntu.com/NotifyOSD#Architecture)
* [C glib2/gio](https://wiki.archlinux.org/index.php/Desktop_notifications#C)

[Karunesh Johri D-Bus tutorial](https://www.softprayog.in/programming/d-bus-tutorial)

---

[SO answer ioctl/netlink interface up/down](https://stackoverflow.com/a/63950398)

[Generic steal_flag()](https://en.cppreference.com/w/c/language/generic)

tun2socks
* [wiki](https://github.com/ambrop72/badvpn/wiki/Tun2socks)
* <del>[Add UDP forwarding w/ badvpn-udpgw](https://github.com/ambrop72/badvpn/wiki/Tun2socks#udp-forwarding)</del>
* ambrop72/badvpn [`--socks5-udp`](https://github.com/ambrop72/badvpn/blob/master/tun2socks/tun2socks.c#:~:text=%21strcmp%28arg%2C%20%22--socks5-udp%22%29) (release too old, build badvpn-git)
* shadowsocks/badvpn [`--enable-udprelay`](https://github.com/shadowsocks/badvpn/blob/shadowsocks-android/tun2socks/tun2socks.c#:~:text=%21strcmp%28arg%2C%20%22--enable-udprelay%22%29)
* [go-tun2socks](https://github.com/eycorsican/go-tun2socks)

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

Q

* tell dhcpcd to refresh
* /usr/include/linux/if_link.h
  * IFLA_MAP
  * IFLA_LINKINFO

little-endian

```
rta_len=5 

rta         rta+4
|           |
00 00 00 00 08
01
```

DNS
* [DNS黑魔法](https://medium.com/@TachyonDevel/%E6%BC%AB%E8%B0%88%E5%90%84%E7%A7%8D%E9%BB%91%E7%A7%91%E6%8A%80%E5%BC%8F-dns-%E6%8A%80%E6%9C%AF%E5%9C%A8%E4%BB%A3%E7%90%86%E7%8E%AF%E5%A2%83%E4%B8%AD%E7%9A%84%E5%BA%94%E7%94%A8-62c50e58cbd0)
* [fake-ip](https://blog.skk.moe/post/what-happend-to-dns-in-proxy/)

netlink
* [libnl](https://www.infradead.org/~tgr/libnl/)
* [rtnetlink tutorial](https://www.linuxjournal.com/article/8498)
* [netlink tutorial](https://www.linuxjournal.com/article/7356)

Clash
* [config.yaml](https://lancellc.gitbook.io/clash/)
* [external controller API](https://clash.gitbook.io/doc/restful-api) ([short](https://github.com/Dreamacro/clash/wiki/external-controller-API-reference))

libcurl
*  https://curl.haxx.se/libcurl/c/SOME_FUNCTION.html

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

## Start VPN

extract

```bash
./clash_tun.out rixcloud
cat /tmp/ss-local.json
```

ss-local (tty3)

```bash
# Stop clash
ss-local -v -c /tmp/ss-local.json
```

tun2socks

```bash
su -
# IP=("$(jq </tmp/ss-local.json '.server' | tr -d '"')" "8.8.8.8" "8.8.4.4")
IP=("$(jq </tmp/ss-local.json '.server' | tr -d '"')")
GW="192.168.1.1"
echo ${IP[@]}

systemctl stop systemd-resolved.service
rm -fv /etc/resolv.conf
ln -sfv /run/systemd/resolve/stub-resolv.conf /etc/resolv.conf
systemctl start systemd-resolved.service

ip tuntap add dev tun0 mode tun
ip link set tun0 up
ip addr flush dev tun0
ip addr add dev tun0 10.0.0.1/24
for i in ${IP[@]}; do ip route add "$i" via "$GW"; done
ip route del default via "$GW"
ip route add default via 10.0.0.2
ip route

iptables -F OUTPUT
iptables -A OUTPUT -p udp -j REJECT
iptables -S

# badvpn-udpgw \
#   --logger stdout \
#   --loglevel info \
#   --listen-addr 127.0.0.1:7300
badvpn-tun2socks \
  --tundev tun0 \
  --netif-ipaddr 10.0.0.2 \
  --netif-netmask 255.255.255.0 \
  --socks-server-addr 127.0.0.1:1080
# --udpgw-remote-server-addr 127.0.0.1:7300
# --socks5-udp
# --enable-udprelay
```

## Stop VPN

```bash
killall badvpn-tun2socks
killall ss-local

iptables -F OUTPUT

killall -SIGINT dhcpcd
pgrep -a dhcpcd

ip addr flush dev tun0
ip link set tun0 down
ip link del dev tun0
ip route flush table main
ip route

systemctl stop systemd-resolved.service
rm -fv /etc/resolv.conf
ln -sfv /run/dhcpcd/hook-state/resolv.conf/wlp2s0.dhcp /etc/resolv.conf
```

dhcpcd

```bash
dhcpcd.sh
```
