Clash
* [config.yaml](https://lancellc.gitbook.io/clash/)
* external controller API
  * [synopsis](https://github.com/Dreamacro/clash/wiki/external-controller-API-reference)
  * [docbook](https://clash.gitbook.io/doc/restful-api)

libcurl
*  https://curl.haxx.se/libcurl/c/SOME_FUNCTION.html

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
