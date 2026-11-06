/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_MAC_PARSE_H
#define VLSX_CORE_NET_MAC_PARSE_H

#include <stdbool.h>

#include "common/result.h"
#include "core/net/mac/mac_types.h"

bool mac_parse(const char *input, mac_addr_t *out, calc_status_t *status);
bool mac_validate(const char *input);
bool mac_accept(char ch);

#endif /* VLSX_CORE_NET_MAC_PARSE_H */
