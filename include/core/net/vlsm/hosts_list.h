/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_VLSM_HOSTS_LIST_H
#define VLSX_CORE_NET_VLSM_HOSTS_LIST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/result.h"

bool hosts_validate(const char *input);
bool hosts_accept(char ch);

bool hosts_parse(const char *input, uint32_t *hosts, size_t capacity, size_t *count,
                 calc_status_t *status);
void hosts_sort_descending(uint32_t *hosts, size_t count);

#endif /* VLSX_CORE_NET_VLSM_HOSTS_LIST_H */
