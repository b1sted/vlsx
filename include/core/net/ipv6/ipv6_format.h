/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV6_FORMAT_H
#define VLSX_CORE_NET_IPV6_FORMAT_H

#include <stddef.h>
#include <stdint.h>

#include "core/net/ipv6/ipv6_types.h"

void ipv6_format_no_leading_zeros(const ipv6_addr_t *addr, char *dest, size_t dest_size);
void ipv6_format_compressed(const ipv6_addr_t *addr, char *dest, size_t dest_size);
void ipv6_format_expanded(const ipv6_addr_t *addr, char *dest, size_t dest_size);
void ipv6_format_interface_id(const ipv6_addr_t *interface_id, uint8_t prefix, char *dest,
                              size_t dest_size);

#endif /* VLSX_CORE_NET_IPV6_FORMAT_H */
