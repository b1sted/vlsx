/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/vlsm/vlsm_alloc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common/result.h"
#include "common/utils.h"
#include "core/net/ipv4/ipv4_calc.h"
#include "core/net/ipv4/ipv4_parse.h"
#include "core/net/vlsm/hosts_list.h"

bool vlsm_plan(const char *cidr_text, const char *hosts_text, vlsm_result_t *out,
               calc_status_t *status)
{
    vlsm_result_t result = {0};
    uint32_t requested[VLSM_MAX_SUBNETS] = {0};
    size_t request_count = 0;
    ipv4_addr_t base = {0};
    uint8_t prefix = 0;
    uint64_t total_requested = 0;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!ipv4_parse_cidr(cidr_text, &base, &prefix, status)) {
        return false;
    }

    if (prefix == 0) {
        calc_status_fail(status, CALC_ERR_RANGE,
                         "Invalid parent mask.\nPrefix /0 is not allowed for "
                         "VLSM.");
        return false;
    }

    const uint32_t capacity = ipv4_host_capacity(prefix);

    if (!hosts_parse(hosts_text, requested, VLSM_MAX_SUBNETS, &request_count, status)) {
        return false;
    }

    for (size_t i = 0; i < request_count; i++) {
        total_requested += requested[i];
    }

    if (total_requested > UINT32_MAX) {
        calc_status_fail(status, CALC_ERR_CAPACITY,
                         "Invalid input.\nRequested hosts exceed total IPv4 address space.");
        return false;
    }

    if (total_requested > capacity) {
        calc_status_fail(status, CALC_ERR_CAPACITY,
                         "Invalid input.\nRequested hosts exceed capacity of "
                         "the subnet mask.");
        return false;
    }

    hosts_sort_descending(requested, request_count);

    ipv4_addr_t base_mask = {0};
    ipv4_addr_t base_network = {0};

    ipv4_mask_from_prefix(prefix, &base_mask);

    result.base_network.value = ipv4_network_value(base.value, base_mask.value);
    ipv4_addr_from_value(result.base_network.value, &base_network);
    result.base_network = base_network;
    result.base_prefix = prefix;
    result.capacity = capacity;
    result.entry_count = request_count;

    uint32_t current_base = base_network.value;
    uint8_t current_prefix = prefix;
    const uint64_t parent_last =
        (uint64_t)base_network.value + ((UINT64_C(1) << (32U - (uint32_t)prefix)) - UINT64_C(1));

    uint64_t allocated_total = 0;

    for (size_t i = 0; i < request_count; i++) {
        const uint64_t requested_with_overhead = (uint64_t)requested[i] + UINT64_C(2);

        if (requested_with_overhead > UINT32_MAX) {
            calc_status_fail(status, CALC_ERR_CAPACITY,
                             "VLSM allocation failed.\nRequired subnet size exceeds the "
                             "remaining network capacity.");
            return false;
        }

        const uint32_t provisioned = util_round_up_pow2((uint32_t)requested_with_overhead);

        if (provisioned == 0) {
            calc_status_fail(status, CALC_ERR_CAPACITY,
                             "VLSM allocation failed.\nRequired subnet size exceeds the "
                             "remaining network capacity.");
            return false;
        }

        const uint8_t allocated_prefix = (uint8_t)(32U - (uint32_t)util_pow2_exponent(provisioned));

        if (allocated_prefix < current_prefix) {
            calc_status_fail(status, CALC_ERR_SYNTAX,
                             "Internal VLSM state corrupted.\nSubnets are not "
                             "in sorted order.");
            return false;
        }

        const uint64_t subnet_last = (uint64_t)current_base + (uint64_t)provisioned - UINT64_C(1);

        if (subnet_last > parent_last) {
            calc_status_fail(status, CALC_ERR_CAPACITY,
                             "VLSM allocation failed.\nRequired subnet size exceeds the "
                             "remaining network capacity.");
            return false;
        }

        vlsm_entry_t *entry = &result.entries[i];

        entry->requested_hosts = requested[i];
        entry->allocated_hosts = provisioned - UINT32_C(2);
        entry->unused_hosts = entry->allocated_hosts - requested[i];
        entry->prefix = allocated_prefix;

        ipv4_addr_from_value(current_base, &entry->network);

        ipv4_addr_t subnet_mask = {0};
        ipv4_mask_from_prefix(allocated_prefix, &subnet_mask);

        ipv4_addr_from_value(ipv4_broadcast_value(current_base, subnet_mask.value),
                             &entry->broadcast);
        ipv4_addr_from_value(ipv4_first_host_value(allocated_prefix, current_base), &entry->first);
        ipv4_addr_from_value(
            ipv4_last_host_value(allocated_prefix, current_base, entry->broadcast.value),
            &entry->last);

        allocated_total += entry->allocated_hosts;

        current_base = entry->broadcast.value + UINT32_C(1);
        current_prefix = allocated_prefix;
    }

    result.allocated_hosts = (uint32_t)allocated_total;
    result.unallocated_hosts =
        (capacity > result.allocated_hosts) ? capacity - result.allocated_hosts : 0;
    result.percent_usage =
        (capacity > 0) ? ((double)result.allocated_hosts / (double)capacity) * 100.0 : 0.0;

    *out = result;
    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}
