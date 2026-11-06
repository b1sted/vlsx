/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv6/ipv6_calc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common/result.h"
#include "core/net/ipv6/ipv6_class.h"
#include "core/net/ipv6/ipv6_format.h"
#include "core/net/ipv6/ipv6_parse.h"
#include "core/net/ipv6/ipv6_types.h"

void ipv6_network_addr(const ipv6_addr_t *base, uint8_t prefix, ipv6_addr_t *network)
{
    uint8_t effective_prefix = prefix;

    if (network == NULL) {
        return;
    }

    memset(network, 0, sizeof(*network));

    if (base == NULL) {
        return;
    }

    if (effective_prefix > 128) {
        effective_prefix = 128;
    }

    const size_t full_hextets = (size_t)effective_prefix / 16;
    const unsigned int remaining_bits = (unsigned int)effective_prefix % 16;

    for (size_t i = 0; i < IPV6_HEXTET_COUNT; i++) {
        if (i < full_hextets) {
            network->hextets[i] = base->hextets[i];
        } else if (i == full_hextets && remaining_bits > 0) {
            const uint16_t mask = (uint16_t)(UINT16_C(0xFFFF) << (16U - remaining_bits));
            network->hextets[i] = (uint16_t)(base->hextets[i] & mask);
        }
    }
}

void ipv6_interface_id_addr(const ipv6_addr_t *base, uint8_t prefix, ipv6_addr_t *interface_id)
{
    uint8_t effective_prefix = prefix;

    if (interface_id == NULL) {
        return;
    }

    memset(interface_id, 0, sizeof(*interface_id));

    if (base == NULL) {
        return;
    }

    if (effective_prefix > 128) {
        effective_prefix = 128;
    }

    const size_t full_hextets = (size_t)effective_prefix / 16;
    const unsigned int remaining_bits = (unsigned int)effective_prefix % 16;

    for (size_t i = 0; i < IPV6_HEXTET_COUNT; i++) {
        if (i < full_hextets) {
            continue;
        }

        if (i == full_hextets && remaining_bits > 0) {
            const uint16_t mask = (uint16_t)((UINT32_C(1) << (16U - remaining_bits)) - 1U);
            interface_id->hextets[i] = (uint16_t)(base->hextets[i] & mask);
        } else {
            interface_id->hextets[i] = base->hextets[i];
        }
    }
}

bool ipv6_calculate(const char *cidr_text, ipv6_result_t *out, calc_status_t *status)
{
    ipv6_addr_t base = {0};
    uint8_t prefix = 0;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!ipv6_parse_cidr(cidr_text, &base, &prefix, status)) {
        return false;
    }

    out->base = base;
    out->prefix = prefix;

    ipv6_network_addr(&out->base, out->prefix, &out->network);
    ipv6_interface_id_addr(&out->base, out->prefix, &out->interface_id);

    const char *slash = strchr(cidr_text, '/');
    const size_t original_length =
        (slash != NULL) ? (size_t)(slash - cidr_text) : strlen(cidr_text);

    if (original_length >= sizeof(out->original)) {
        calc_status_fail(status, CALC_ERR_CAPACITY, "IPv6 address is too long");
        return false;
    }

    memcpy(out->original, cidr_text, original_length);
    out->original[original_length] = '\0';

    ipv6_format_no_leading_zeros(&out->base, out->no_leading_zeros, sizeof(out->no_leading_zeros));
    ipv6_format_compressed(&out->base, out->compressed, sizeof(out->compressed));
    ipv6_format_expanded(&out->base, out->expanded, sizeof(out->expanded));
    ipv6_format_compressed(&out->network, out->network_text, sizeof(out->network_text));
    ipv6_format_interface_id(&out->interface_id, out->prefix, out->interface_id_text,
                             sizeof(out->interface_id_text));
    out->type = ipv6_classify(&out->base);

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}
