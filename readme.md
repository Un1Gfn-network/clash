---

<!-- https://www.w3schools.com/charsets/ref_utf_geometric.asp -->
**&#9656; readme.md**\
&bullet; [clash_tun/readme.md](https://github.com/Un1Gfn-network/clash/tree/master/clash_tun)

---

## Doxygen

https://stackoverflow.com/a/42479100

Find headers to add to `INPUT =`

    grep -h -r '#include.*<.*>' "$(git rev-parse --show-toplevel)" | sed 's, *//.*$,,g' | sort | uniq

## Misc

<!-- https://en.wikipedia.org/wiki/List_of_logic_symbols -->

configure.ac [`AC_DEFINE(X...)`](https://www.gnu.org/software/autoconf/manual/autoconf-2.70/html_node/Defining-Symbols.html) &rArr;\
&bullet; Makefile `DEFS = -DX...`\
&bullet; config.h `#define X...`

configure.ac [`AC_ARG_VAR(X...)`](https://www.gnu.org/software/autoconf/manual/autoconf-2.70/html_node/Setting-Output-Variables.html#index-AC_005fARG_005fVAR-1) + `./configure ...=...` &rArr;\
&bullet; `X` is AC_SUBST'ed &nbsp; (use `$(X)` manually in Makefile.am)

    cd "$(git rev-parse --show-toplevel)" && \
    git clean -dfX && \
    git status --ignored

<div></div>

    cd "$(git rev-parse --show-toplevel)" && \
    autoreconf -v -i -Wall

<div></div>

    cd "$(git rev-parse --show-toplevel)"/build && \
    ../configure \
      --prefix=/usr/local \
      RESTFUL_PORT=9090 \
      CFLAGS="-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline -Wshadow -D_GNU_SOURCE" \
      LIBTOOLFLAGS="-v --no-silent"

<div></div>

    cd "$(git rev-parse --show-toplevel)"/build
    make --no-print-directory all

<div></div>

    cd "$(git rev-parse --show-toplevel)"/build
    rm -rf /tmp/x && make --no-print-directory DESTDIR=/tmp/x install && tree -aC /tmp/x
    [ -f /tmp/raw.yaml ] && env LD_LIBRARY_PATH=/tmp/x/usr/local/lib /tmp/x/usr/local/bin/clash_convert </tmp/raw.yaml
    make DESTDIR=/tmp/x uninstall && tree -aC /tmp/x && rm -rv /tmp/x

<div></div>

    cd "$(git rev-parse --show-toplevel)"/build
    sudo make --no-print-directory uninstall
    tree -aC /usr/local
    sudo make --no-print-directory install
    tree -aC /usr/local

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

little-endian

```
rta_len=5 

rta         rta+4
|           |
00 00 00 00 08
01
```

[python percent encode $1](https://unix.stackexchange.com/questions/159253/decoding-url-encoding-percent-encoding)

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

## Garbage

<details><summary>garbage</summary>

<br>

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
