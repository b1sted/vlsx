/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv6/ipv6_class.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/net/ipv6/ipv6_types.h"

static bool ipv6_is_unspecified(const ipv6_addr_t *addr);
static bool ipv6_is_loopback(const ipv6_addr_t *addr);
static bool ipv6_is_ipv4_mapped(const ipv6_addr_t *addr);
static bool ipv6_is_ipv4_compatible(const ipv6_addr_t *addr);

static bool ipv6_is_unspecified(const ipv6_addr_t *addr)
{
    for (size_t i = 0; i < IPV6_HEXTET_COUNT; i++) {
        if (addr->hextets[i] != 0) {
            return false;
        }
    }

    return true;
}

static bool ipv6_is_loopback(const ipv6_addr_t *addr)
{
    for (size_t i = 0; i < IPV6_HEXTET_COUNT - 1; i++) {
        if (addr->hextets[i] != 0) {
            return false;
        }
    }

    return addr->hextets[IPV6_HEXTET_COUNT - 1] == 1;
}

static bool ipv6_is_ipv4_mapped(const ipv6_addr_t *addr)
{
    for (size_t i = 0; i < 5; i++) {
        if (addr->hextets[i] != 0) {
            return false;
        }
    }

    return addr->hextets[5] == UINT16_C(0xFFFF);
}

static bool ipv6_is_ipv4_compatible(const ipv6_addr_t *addr)
{
    for (size_t i = 0; i < 6; i++) {
        if (addr->hextets[i] != 0) {
            return false;
        }
    }

    return true;
}

ipv6_type_t ipv6_classify(const ipv6_addr_t *addr)
{
    if (addr == NULL) {
        return IPV6_TYPE_RESERVED;
    }

    const uint16_t first = addr->hextets[0];
    const uint16_t second = addr->hextets[1];

    if (ipv6_is_unspecified(addr)) {
        return IPV6_TYPE_UNSPECIFIED;
    }

    if (ipv6_is_loopback(addr)) {
        return IPV6_TYPE_LOOPBACK;
    }

    if (ipv6_is_ipv4_mapped(addr)) {
        return IPV6_TYPE_IPV4_MAPPED;
    }

    if (ipv6_is_ipv4_compatible(addr)) {
        return IPV6_TYPE_IPV4_COMPATIBLE;
    }

    if ((first & UINT16_C(0xFF00)) == UINT16_C(0xFF00)) {
        return IPV6_TYPE_MULTICAST;
    }

    if ((first & UINT16_C(0xFFC0)) == UINT16_C(0xFE80)) {
        return IPV6_TYPE_LINK_LOCAL;
    }

    if ((first & UINT16_C(0xFFC0)) == UINT16_C(0xFEC0)) {
        return IPV6_TYPE_SITE_LOCAL;
    }

    if ((first & UINT16_C(0xFE00)) == UINT16_C(0xFC00)) {
        return IPV6_TYPE_UNIQUE_LOCAL;
    }

    if (first == UINT16_C(0x2002)) {
        return IPV6_TYPE_6TO4;
    }

    if (first == UINT16_C(0x2001) && second == 0) {
        return IPV6_TYPE_TEREDO;
    }

    if (first == UINT16_C(0x2001) && second == UINT16_C(0x0DB8)) {
        return IPV6_TYPE_DOCUMENTATION;
    }

    if ((first & UINT16_C(0xE000)) == UINT16_C(0x2000)) {
        return IPV6_TYPE_GLOBAL_UNICAST;
    }

    return IPV6_TYPE_RESERVED;
}

const char *ipv6_type_name(ipv6_type_t type)
{
    switch (type) {
    case IPV6_TYPE_UNSPECIFIED:
        return "Unspecified (Неопределённый адрес)";
    case IPV6_TYPE_LOOPBACK:
        return "Loopback (Петлевой адрес)";
    case IPV6_TYPE_IPV4_MAPPED:
        return "IPv4-Mapped (Отображённый IPv4-адрес)";
    case IPV6_TYPE_IPV4_COMPATIBLE:
        return "IPv4-Compatible (Совместимый с IPv4)";
    case IPV6_TYPE_LINK_LOCAL:
        return "Link-Local (Локальный адрес канала)";
    case IPV6_TYPE_UNIQUE_LOCAL:
        return "Unique Local (Уникальный локальный адрес)";
    case IPV6_TYPE_MULTICAST:
        return "Multicast (Многоадресный адрес)";
    case IPV6_TYPE_SITE_LOCAL:
        return "Site-Local (Локальный адрес сайта)";
    case IPV6_TYPE_DOCUMENTATION:
        return "Documentation (Документационный адрес)";
    case IPV6_TYPE_6TO4:
        return "6to4 (Туннельный 6to4)";
    case IPV6_TYPE_TEREDO:
        return "Teredo (Туннельный Teredo)";
    case IPV6_TYPE_GLOBAL_UNICAST:
        return "Global Unicast (Глобальный одноадресный)";
    case IPV6_TYPE_RESERVED:
        return "Reserved (Зарезервированный)";
    }

    return "Unknown (Неизвестный)";
}
