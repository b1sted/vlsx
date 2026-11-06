/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_MAC_CALC_H
#define VLSX_CORE_NET_MAC_CALC_H

#include <stdbool.h>

#include "common/result.h"
#include "core/net/mac/mac_types.h"

mac_cast_t mac_cast_type(const mac_addr_t *addr);
mac_admin_t mac_admin_type(const mac_addr_t *addr);
const char *mac_cast_name(mac_cast_t cast);
const char *mac_admin_name(mac_admin_t admin);

bool mac_calculate(const char *text, mac_result_t *out, calc_status_t *status);

#endif /* VLSX_CORE_NET_MAC_CALC_H */
