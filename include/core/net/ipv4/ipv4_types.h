/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV4_TYPES_H
#define VLSX_CORE_NET_IPV4_TYPES_H

#include <stdint.h>

typedef struct {
    uint32_t value;
    uint8_t octets[4];
} ipv4_addr_t;

typedef enum {
    IPV4_CLASS_A_PUBLIC,
    IPV4_CLASS_B_PUBLIC,
    IPV4_CLASS_C_PUBLIC,
    IPV4_CLASS_DEFAULT_ROUTE,
    IPV4_CLASS_LIMITED_BROADCAST,
    IPV4_CLASS_THIS_NETWORK,
    IPV4_CLASS_CGNAT,
    IPV4_CLASS_LOOPBACK,
    IPV4_CLASS_APIPA,
    IPV4_CLASS_IETF_PROTOCOL,
    IPV4_CLASS_TEST_NET_1,
    IPV4_CLASS_BENCHMARKING,
    IPV4_CLASS_TEST_NET_2,
    IPV4_CLASS_TEST_NET_3,
    IPV4_CLASS_PRIVATE_A,
    IPV4_CLASS_PRIVATE_B,
    IPV4_CLASS_PRIVATE_C,
    IPV4_CLASS_MULTICAST,
    IPV4_CLASS_EXPERIMENTAL,
} ipv4_class_t;

typedef struct {
    ipv4_addr_t base;
    uint8_t prefix;
    ipv4_addr_t mask;
    ipv4_addr_t network;
    ipv4_addr_t broadcast;
    ipv4_addr_t first_host;
    ipv4_addr_t last_host;
    uint32_t host_count;
    ipv4_class_t class;
} ipv4_result_t;

#endif /* VLSX_CORE_NET_IPV4_TYPES_H */
