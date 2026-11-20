/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv4/ipv4_class.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t address;
    uint32_t mask;
    uint8_t min_prefix;
    ipv4_class_t cls;
} ipv4_class_rule_t;

static const ipv4_class_rule_t ipv4_class_rules[] = {
    /* Specific single addresses */
    {UINT32_C(0x00000000), UINT32_C(0xFFFFFFFF), 0, IPV4_CLASS_DEFAULT_ROUTE},
    {UINT32_C(0xFFFFFFFF), UINT32_C(0xFFFFFFFF), 0, IPV4_CLASS_LIMITED_BROADCAST},

    /* Special ranges */
    {UINT32_C(0x00000000), UINT32_C(0xFF000000), 1, IPV4_CLASS_THIS_NETWORK},
    {UINT32_C(0x64400000), UINT32_C(0xFFC00000), 0, IPV4_CLASS_CGNAT},
    {UINT32_C(0x7F000000), UINT32_C(0xFF000000), 0, IPV4_CLASS_LOOPBACK},
    {UINT32_C(0xA9FE0000), UINT32_C(0xFFFF0000), 0, IPV4_CLASS_APIPA},
    {UINT32_C(0xC0000000), UINT32_C(0xFFFFFF00), 0, IPV4_CLASS_IETF_PROTOCOL},
    {UINT32_C(0xC0000200), UINT32_C(0xFFFFFF00), 0, IPV4_CLASS_TEST_NET_1},
    {UINT32_C(0xC6120000), UINT32_C(0xFFFE0000), 0, IPV4_CLASS_BENCHMARKING},
    {UINT32_C(0xC6336400), UINT32_C(0xFFFFFF00), 0, IPV4_CLASS_TEST_NET_2},
    {UINT32_C(0xCB007100), UINT32_C(0xFFFFFF00), 0, IPV4_CLASS_TEST_NET_3},

    /* Private networks (RFC 1918) */
    {UINT32_C(0x0A000000), UINT32_C(0xFF000000), 0, IPV4_CLASS_PRIVATE_A},
    {UINT32_C(0xAC100000), UINT32_C(0xFFF00000), 0, IPV4_CLASS_PRIVATE_B},
    {UINT32_C(0xC0A80000), UINT32_C(0xFFFF0000), 0, IPV4_CLASS_PRIVATE_C},

    /* Broadcasting and experiments */
    {UINT32_C(0xE0000000), UINT32_C(0xF0000000), 0, IPV4_CLASS_MULTICAST},
    {UINT32_C(0xF0000000), UINT32_C(0xF0000000), 0, IPV4_CLASS_EXPERIMENTAL},
};

ipv4_class_t ipv4_classify(const ipv4_addr_t *addr, uint8_t prefix)
{
    if (addr == NULL) {
        return IPV4_CLASS_C_PUBLIC;
    }

    const size_t rule_count = sizeof(ipv4_class_rules) / sizeof(ipv4_class_rules[0]);

    for (size_t i = 0; i < rule_count; i++) {
        const ipv4_class_rule_t *rule = &ipv4_class_rules[i];

        if ((addr->value & rule->mask) != rule->address) {
            continue;
        }

        if (rule->min_prefix > 0 && prefix == 0) {
            continue;
        }

        return rule->cls;
    }

    if (addr->octets[0] <= 127) {
        return IPV4_CLASS_A_PUBLIC;
    }

    if (addr->octets[0] <= 191) {
        return IPV4_CLASS_B_PUBLIC;
    }

    return IPV4_CLASS_C_PUBLIC;
}

const char *ipv4_class_name(ipv4_class_t cls)
{
    switch (cls) {
    case IPV4_CLASS_A_PUBLIC:
        return "A (Public)";
    case IPV4_CLASS_B_PUBLIC:
        return "B (Public)";
    case IPV4_CLASS_C_PUBLIC:
        return "C (Public)";
    case IPV4_CLASS_DEFAULT_ROUTE:
        return "N/A (Default Route)";
    case IPV4_CLASS_LIMITED_BROADCAST:
        return "N/A (Limited Broadcast)";
    case IPV4_CLASS_THIS_NETWORK:
        return "N/A (This Network)";
    case IPV4_CLASS_CGNAT:
        return "A (Shared / CGNAT)";
    case IPV4_CLASS_LOOPBACK:
        return "A (Loopback)";
    case IPV4_CLASS_APIPA:
        return "B (APIPA (Link-Local))";
    case IPV4_CLASS_IETF_PROTOCOL:
        return "C (IETF Protocol Assignments)";
    case IPV4_CLASS_TEST_NET_1:
        return "C (TEST-NET-1)";
    case IPV4_CLASS_BENCHMARKING:
        return "C (Benchmarking)";
    case IPV4_CLASS_TEST_NET_2:
        return "C (TEST-NET-2)";
    case IPV4_CLASS_TEST_NET_3:
        return "C (TEST-NET-3)";
    case IPV4_CLASS_PRIVATE_A:
        return "A (Private)";
    case IPV4_CLASS_PRIVATE_B:
        return "B (Private)";
    case IPV4_CLASS_PRIVATE_C:
        return "C (Private)";
    case IPV4_CLASS_MULTICAST:
        return "D (Multicast)";
    case IPV4_CLASS_EXPERIMENTAL:
        return "E (Experimental / Reversed)";
    default:
        return "Unknown";
    }
}
