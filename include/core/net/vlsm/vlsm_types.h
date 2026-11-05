/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_VLSM_TYPES_H
#define VLSX_CORE_NET_VLSM_TYPES_H

#include <stddef.h>
#include <stdint.h>

#include "core/net/ipv4/ipv4_types.h"

#define VLSM_MAX_SUBNETS 16

typedef struct {
    uint32_t requested_hosts;
    uint32_t allocated_hosts;
    uint32_t unused_hosts;
    uint8_t prefix;
    ipv4_addr_t network;
    ipv4_addr_t broadcast;
    ipv4_addr_t first;
    ipv4_addr_t last;
} vlsm_entry_t;

typedef struct {
    ipv4_addr_t base_network;
    uint8_t base_prefix;
    uint32_t capacity;
    uint32_t allocated_hosts;
    uint32_t unallocated_hosts;
    double percent_usage;
    vlsm_entry_t entries[VLSM_MAX_SUBNETS];
    size_t entry_count;
} vlsm_result_t;

#endif /* VLSX_CORE_NET_VLSM_TYPES_H */
