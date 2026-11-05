/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv4/ipv4_parse.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "core/net/ipv4/ipv4_validate.h"

bool ipv4_parse_cidr(const char *input, ipv4_addr_t *base, uint8_t *prefix, calc_status_t *status)
{
    int octets[4] = {0, 0, 0, 0};
    int mask = 0;
    int consumed = 0;

    if (base == NULL || prefix == NULL || status == NULL) {
        return false;
    }

    memset(base, 0, sizeof(*base));
    *prefix = 0;
    calc_status_clear(status);

    if (!ipv4_validate(input)) {
        calc_status_fail(status, CALC_ERR_SYNTAX,
                         "Invalid input.\nPlease correct it and try again.");
        return false;
    }

    if (sscanf(input, "%d.%d.%d.%d/%d%n", &octets[0], &octets[1], &octets[2], &octets[3], &mask,
               &consumed) != 5 ||
        input[consumed] != '\0') {
        calc_status_fail(status, CALC_ERR_SYNTAX, "Invalid input.\nExpected format: A.B.C.D/mask");
        return false;
    }

    for (size_t i = 0; i < 4; i++) {
        if (octets[i] < 0 || octets[i] > 255) {
            calc_status_failf(status, CALC_ERR_RANGE,
                              "Invalid input.\nThe value of octet %d is out of "
                              "range: [0, 255]",
                              (int)(i + 1));
            return false;
        }

        const uint8_t octet = (uint8_t)octets[i];

        base->value = (base->value << 8) | (uint32_t)octet;
        base->octets[i] = octet;
    }

    if (mask < 0 || mask > 32) {
        calc_status_failf(status, CALC_ERR_RANGE,
                          "Invalid input.\nMask prefix %d is out of range: "
                          "[0, 32]",
                          mask);
        return false;
    }

    *prefix = (uint8_t)mask;
    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}
