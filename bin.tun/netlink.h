#pragma once

#include <stdbool.h>

void netlink_init();

void netlink_end();

#define netlink_add_route(dev,dst,via) netlink_route(true,false,dev,dst,via)
#define netlink_del_route(dev,dst,via) netlink_route(false,false,dev,dst,via)
#define netlink_add_gateway(dev,via) netlink_route(true,true,dev,NULL,via)
#define netlink_del_gateway(dev,via) netlink_route(false,true,dev,NULL,via)
void netlink_route(const bool add,const bool gw,const char *__restrict const dev,const char *__restrict const dst,const char *__restrict const via);

// void netlink_print_route();

void netlink_get_gateway(char *__restrict const s);

// void netlink_print_link();

void netlink_del_link(const char *__restrict const dev);

// void netlink_print_addr();

void netlink_tun_addr(const char *__restrict const dev,const char *__restrict const ipv4,const unsigned char prefixlen);

#define netlink_up(D) netlink_flags(true,D)
#define netlink_down(D) netlink_flags(false,D) // Can change qdisc from fq_codel to noop?
void netlink_flags(const bool up,const char *__restrict const dev);
