#pragma once

#define SS_LOCAL_JSON "/tmp/ss-local.json"
// #define TUN "tunT"
#define TUN "tun0"
#define WLO "wlp2s0"

#define LOCAL_PORT_I 1080
#define LOCAL_PORT_S "1080"

// #define LOCAL_ADDR "127.0.0.1" // Both 10.0.0.* and 192.168.1.* are rejected
#define LOCAL_ADDR "0.0.0.0"

// 11:22:33:44:55:66
#define MAC_L (2*6+5)

// #define SZ 16384
#define SZ 8192

#define USR "darren"

#define SS_LOG "/tmp/ss-local.log"
#define TUN_LOG "/tmp/badvpn-tun2socks.log"

#define DNS {"8.8.8.8","8.8.4.4",NULL}

// ss-local(1)
// #define TIMEOUT 0 // Drops any connection instantly in newer versions
// #define TIMEOUT 60 // Default
#define TIMEOUT 7 // Default

// Moved to libclash.h
// #define YAML_PATH
