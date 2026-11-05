/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV4_CALC_H
#define VLSX_CORE_NET_IPV4_CALC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/result.h"
#include "core/net/ipv4/ipv4_types.h"

void ipv4_addr_from_value(uint32_t value, ipv4_addr_t *addr);
void ipv4_format_addr(const ipv4_addr_t *addr, char *dest, size_t dest_size);
void ipv4_mask_from_prefix(uint8_t prefix, ipv4_addr_t *mask);

uint32_t ipv4_network_value(uint32_t address, uint32_t mask);
uint32_t ipv4_broadcast_value(uint32_t network, uint32_t mask);
uint32_t ipv4_host_capacity(uint8_t prefix);
uint32_t ipv4_first_host_value(uint8_t prefix, uint32_t network);
uint32_t ipv4_last_host_value(uint8_t prefix, uint32_t base, uint32_t broadcast);

bool ipv4_calculate(const char *cidr_text, ipv4_result_t *out, calc_status_t *status);

#endif /* VLSX_CORE_NET_IPV4_CALC_H */
