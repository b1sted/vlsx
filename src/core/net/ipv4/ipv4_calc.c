/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv4/ipv4_calc.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "core/net/ipv4/ipv4_class.h"
#include "core/net/ipv4/ipv4_parse.h"

void ipv4_addr_from_value(uint32_t value, ipv4_addr_t *addr)
{
    if (addr == NULL) {
        return;
    }

    addr->value = value;
    addr->octets[0] = (uint8_t)(value >> 24);
    addr->octets[1] = (uint8_t)(value >> 16);
    addr->octets[2] = (uint8_t)(value >> 8);
    addr->octets[3] = (uint8_t)value;
}

void ipv4_format_addr(const ipv4_addr_t *addr, char *dest, size_t dest_size)
{
    if (addr == NULL || dest == NULL || dest_size == 0) {
        return;
    }

    snprintf(dest, dest_size, "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, addr->octets[0],
             addr->octets[1], addr->octets[2], addr->octets[3]);
}

void ipv4_mask_from_prefix(uint8_t prefix, ipv4_addr_t *mask)
{
    if (mask == NULL) {
        return;
    }

    uint8_t effective_prefix = prefix;

    if (effective_prefix > 32) {
        effective_prefix = 32;
    }

    const uint32_t value =
        (effective_prefix == 0) ? 0 : (UINT32_C(0xFFFFFFFF) << (32 - effective_prefix));

    ipv4_addr_from_value(value, mask);
}

uint32_t ipv4_network_value(uint32_t address, uint32_t mask)
{
    return address & mask;
}

uint32_t ipv4_broadcast_value(uint32_t network, uint32_t mask)
{
    return network | ~mask;
}

uint32_t ipv4_host_capacity(uint8_t prefix)
{
    if (prefix > 32) {
        return 0;
    }

    if (prefix == 32) {
        return 1;
    }

    if (prefix == 31) {
        return 2;
    }

    return (UINT32_C(0xFFFFFFFF) >> prefix) - 1;
}

uint32_t ipv4_first_host_value(uint8_t prefix, uint32_t network)
{
    if (prefix == 32 || prefix == 31) {
        return network;
    }

    return network + 1;
}

uint32_t ipv4_last_host_value(uint8_t prefix, uint32_t base, uint32_t broadcast)
{
    if (prefix == 32) {
        return base;
    }

    if (prefix == 31) {
        return broadcast;
    }

    return broadcast - 1;
}

bool ipv4_calculate(const char *cidr_text, ipv4_result_t *out, calc_status_t *status)
{
    ipv4_addr_t base = {0};
    uint8_t prefix = 0;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!ipv4_parse_cidr(cidr_text, &base, &prefix, status)) {
        return false;
    }

    out->base = base;
    out->prefix = prefix;

    ipv4_mask_from_prefix(prefix, &out->mask);

    ipv4_addr_from_value(ipv4_network_value(base.value, out->mask.value), &out->network);
    ipv4_addr_from_value(ipv4_broadcast_value(out->network.value, out->mask.value),
                         &out->broadcast);
    ipv4_addr_from_value(ipv4_first_host_value(prefix, out->network.value), &out->first_host);
    ipv4_addr_from_value(ipv4_last_host_value(prefix, base.value, out->broadcast.value),
                         &out->last_host);

    out->host_count = ipv4_host_capacity(prefix);
    out->class = ipv4_classify(&out->base, prefix);

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}
