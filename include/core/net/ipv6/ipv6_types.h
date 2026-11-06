/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV6_TYPES_H
#define VLSX_CORE_NET_IPV6_TYPES_H

#include <stdint.h>

#define IPV6_HEXTET_COUNT 8
#define IPV6_ADDRESS_TEXT_MAX 46
#define IPV6_HOST_TEXT_MAX 46

typedef struct {
    uint16_t hextets[IPV6_HEXTET_COUNT];
} ipv6_addr_t;

typedef enum {
    IPV6_TYPE_UNSPECIFIED,
    IPV6_TYPE_LOOPBACK,
    IPV6_TYPE_IPV4_MAPPED,
    IPV6_TYPE_IPV4_COMPATIBLE,
    IPV6_TYPE_LINK_LOCAL,
    IPV6_TYPE_UNIQUE_LOCAL,
    IPV6_TYPE_MULTICAST,
    IPV6_TYPE_SITE_LOCAL,
    IPV6_TYPE_DOCUMENTATION,
    IPV6_TYPE_6TO4,
    IPV6_TYPE_TEREDO,
    IPV6_TYPE_GLOBAL_UNICAST,
    IPV6_TYPE_RESERVED,
} ipv6_type_t;

typedef struct {
    ipv6_addr_t base;
    uint8_t prefix;
    ipv6_addr_t network;
    ipv6_addr_t interface_id;
    ipv6_type_t type;
    char original[IPV6_ADDRESS_TEXT_MAX];
    char no_leading_zeros[IPV6_ADDRESS_TEXT_MAX];
    char compressed[IPV6_ADDRESS_TEXT_MAX];
    char expanded[IPV6_ADDRESS_TEXT_MAX];
    char network_text[IPV6_ADDRESS_TEXT_MAX];
    char interface_id_text[IPV6_HOST_TEXT_MAX];
} ipv6_result_t;

#endif /* VLSX_CORE_NET_IPV6_TYPES_H */
