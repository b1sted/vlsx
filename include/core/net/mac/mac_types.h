/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_MAC_TYPES_H
#define VLSX_CORE_NET_MAC_TYPES_H

#include <stdint.h>

#define MAC_OCTET_COUNT 6
#define MAC_OCTET_BITS 8
#define MAC_ADDRESS_BITS (MAC_OCTET_COUNT * MAC_OCTET_BITS)

#define MAC_TEXT_MAX 18             /* "XX:XX:XX:XX:XX:XX" plus NUL */
#define MAC_GROUP_TEXT_MAX 9        /* "XX:XX:XX" plus NUL */
#define MAC_CISCO_TEXT_MAX 15       /* "XXXX.XXXX.XXXX" plus NUL */
#define MAC_BINARY_TEXT_MAX 54      /* 48 bits, five separators, and NUL */
#define MAC_OCTET_BINARY_TEXT_MAX 9 /* eight bits plus NUL */

typedef struct {
    uint8_t octets[MAC_OCTET_COUNT];
} mac_addr_t;

typedef enum {
    MAC_CAST_UNICAST,
    MAC_CAST_MULTICAST,
    MAC_CAST_BROADCAST,
} mac_cast_t;

typedef enum {
    MAC_ADMIN_UAA,
    MAC_ADMIN_LAA,
} mac_admin_t;

typedef struct {
    mac_addr_t address;
    mac_cast_t cast;
    mac_admin_t admin;
    char colon[MAC_TEXT_MAX];
    char oui[MAC_GROUP_TEXT_MAX];
    char nic[MAC_GROUP_TEXT_MAX];
    char cisco[MAC_CISCO_TEXT_MAX];
    char binary[MAC_BINARY_TEXT_MAX];
    char first_octet_binary[MAC_OCTET_BINARY_TEXT_MAX];
} mac_result_t;

#endif /* VLSX_CORE_NET_MAC_TYPES_H */
