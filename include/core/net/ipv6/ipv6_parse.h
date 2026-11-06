/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV6_PARSE_H
#define VLSX_CORE_NET_IPV6_PARSE_H

#include <stdbool.h>
#include <stdint.h>

#include "common/result.h"
#include "core/net/ipv6/ipv6_types.h"

bool ipv6_parse_cidr(const char *input, ipv6_addr_t *base, uint8_t *prefix, calc_status_t *status);

#endif /* VLSX_CORE_NET_IPV6_PARSE_H */
