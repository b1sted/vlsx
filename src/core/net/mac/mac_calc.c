/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/mac/mac_calc.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "common/result.h"
#include "core/net/mac/mac_format.h"
#include "core/net/mac/mac_parse.h"
#include "core/net/mac/mac_types.h"

mac_cast_t mac_cast_type(const mac_addr_t *addr)
{
    bool is_broadcast = true;

    if (addr == NULL) {
        return MAC_CAST_UNICAST;
    }

    for (size_t i = 0; i < MAC_OCTET_COUNT; i++) {
        if (addr->octets[i] != UINT8_C(0xFF)) {
            is_broadcast = false;
            break;
        }
    }

    if (is_broadcast) {
        return MAC_CAST_BROADCAST;
    }

    if ((addr->octets[0] & UINT8_C(0x01)) != 0U) {
        return MAC_CAST_MULTICAST;
    }

    return MAC_CAST_UNICAST;
}

mac_admin_t mac_admin_type(const mac_addr_t *addr)
{
    if (addr == NULL) {
        return MAC_ADMIN_UAA;
    }

    if ((addr->octets[0] & UINT8_C(0x02)) != 0U) {
        return MAC_ADMIN_LAA;
    }

    return MAC_ADMIN_UAA;
}

const char *mac_cast_name(mac_cast_t cast)
{
    switch (cast) {
    case MAC_CAST_UNICAST:
        return "Unicast";
    case MAC_CAST_MULTICAST:
        return "Multicast";
    case MAC_CAST_BROADCAST:
        return "Broadcast";
    }

    return "Unknown";
}

const char *mac_admin_name(mac_admin_t admin)
{
    switch (admin) {
    case MAC_ADMIN_UAA:
        return "UAA";
    case MAC_ADMIN_LAA:
        return "LAA";
    }

    return "Unknown";
}

bool mac_calculate(const char *text, mac_result_t *out, calc_status_t *status)
{
    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!mac_parse(text, &out->address, status)) {
        return false;
    }

    mac_format_colon(&out->address, out->colon, sizeof(out->colon));
    mac_format_oui(&out->address, out->oui, sizeof(out->oui));
    mac_format_nic(&out->address, out->nic, sizeof(out->nic));
    mac_format_cisco(&out->address, out->cisco, sizeof(out->cisco));
    mac_format_binary(&out->address, out->binary, sizeof(out->binary));
    mac_format_octet_binary(out->address.octets[0], out->first_octet_binary,
                            sizeof(out->first_octet_binary));

    out->cast = mac_cast_type(&out->address);
    out->admin = mac_admin_type(&out->address);

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}
