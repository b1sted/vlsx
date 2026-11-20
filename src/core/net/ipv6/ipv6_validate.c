/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv6/ipv6_validate.h"

#include <stdbool.h>
#include <stdint.h>

#include "common/result.h"
#include "core/net/ipv6/ipv6_parse.h"
#include "core/net/ipv6/ipv6_types.h"

bool ipv6_validate(const char *input)
{
    ipv6_addr_t address = {0};
    uint8_t prefix = 0;
    calc_status_t status = {0};

    return ipv6_parse_cidr(input, &address, &prefix, &status);
}

bool ipv6_accept(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') ||
           ch == ':' || ch == '.' || ch == '/';
}
