/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_MAC_MULTICAST_H
#define VLSX_CORE_NET_MAC_MULTICAST_H

#include <stdbool.h>

#include "common/result.h"
#include "core/net/mac/mac_types.h"

bool mac_multicast_generate(const char *text, mac_multicast_result_t *out, calc_status_t *status);
bool mac_multicast_validate(const char *text);
bool mac_multicast_accept(char ch);

#endif /* VLSX_CORE_NET_MAC_MULTICAST_H */
