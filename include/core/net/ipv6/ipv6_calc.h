/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV6_CALC_H
#define VLSX_CORE_NET_IPV6_CALC_H

#include <stdbool.h>
#include <stdint.h>

#include "common/result.h"
#include "core/net/ipv6/ipv6_types.h"

void ipv6_network_addr(const ipv6_addr_t *base, uint8_t prefix, ipv6_addr_t *network);
void ipv6_interface_id_addr(const ipv6_addr_t *base, uint8_t prefix, ipv6_addr_t *interface_id);

bool ipv6_calculate(const char *cidr_text, ipv6_result_t *out, calc_status_t *status);

#endif /* VLSX_CORE_NET_IPV6_CALC_H */
