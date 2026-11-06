/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_MAC_FORMAT_H
#define VLSX_CORE_NET_MAC_FORMAT_H

#include <stddef.h>

#include "core/net/mac/mac_types.h"

void mac_format_colon(const mac_addr_t *addr, char *dest, size_t dest_size);
void mac_format_cisco(const mac_addr_t *addr, char *dest, size_t dest_size);
void mac_format_oui(const mac_addr_t *addr, char *dest, size_t dest_size);
void mac_format_nic(const mac_addr_t *addr, char *dest, size_t dest_size);
void mac_format_binary(const mac_addr_t *addr, char *dest, size_t dest_size);
void mac_format_octet_binary(uint8_t octet, char *dest, size_t dest_size);

#endif /* VLSX_CORE_NET_MAC_FORMAT_H */
