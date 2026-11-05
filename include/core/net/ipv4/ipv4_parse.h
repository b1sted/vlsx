/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV4_PARSE_H
#define VLSX_CORE_NET_IPV4_PARSE_H

#include <stdbool.h>
#include <stdint.h>

#include "common/result.h"
#include "core/net/ipv4/ipv4_types.h"

bool ipv4_parse_cidr(const char *input, ipv4_addr_t *base, uint8_t *prefix, calc_status_t *status);

#endif /* VLSX_CORE_NET_IPV4_PARSE_H */
