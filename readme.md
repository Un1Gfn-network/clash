[Add UDP forwarding w/ badvpn-udpgw](https://github.com/ambrop72/badvpn/wiki/Tun2socks#udp-forwarding)

Clash
* [config.yaml](https://lancellc.gitbook.io/clash/)
* external controller API
  * [synopsis](https://github.com/Dreamacro/clash/wiki/external-controller-API-reference)
  * [docbook](https://clash.gitbook.io/doc/restful-api)

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

tun2socks
* [wiki](https://github.com/ambrop72/badvpn/wiki/Tun2socks)

[RESTful](https://en.wikipedia.org/wiki/Representational_state_transfer)

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
