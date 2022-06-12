#!/bin/bash

hostname=99.HKAIRPORT.COM
port=114514
method=chacha20-ietf-poly1305
password=P@SSW02D
tag=HK99

userinfo="$(base64 -w0 <<<"$method:$password")"
SS_URI="ss://$userinfo@$hostname:$port#$tag"

echo "$userinfo"
echo "$SS_URI"

qrencode -tUTF8 "$SS_URI"
