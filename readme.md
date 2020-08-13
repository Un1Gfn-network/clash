Clash
* [config.yaml](https://lancellc.gitbook.io/clash/)
* external controller API
  * [synopsis](https://github.com/Dreamacro/clash/wiki/external-controller-API-reference)
  * [docbook](https://clash.gitbook.io/doc/restful-api)

libcurl
*  https://curl.haxx.se/libcurl/c/SOME_FUNCTION.html

According to [redsocks](https://github.com/darkk/redsocks/blob/master/README.md)
>Probably, the better way is to use on-device VPN daemon to intercept
traffic via [`VpnService` API for Android](https://developer.android.com/reference/android/net/VpnService.html)
and [`NETunnelProvider` family of APIs for iOS](https://developer.apple.com/documentation/networkextension).
That may require some code doing [TCP Reassembly](https://wiki.wireshark.org/TCP_Reassembly)
like [`tun2socks`](https://github.com/ambrop72/badvpn/wiki/Tun2socks).

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
