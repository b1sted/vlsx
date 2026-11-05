/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_VLSM_ALLOC_H
#define VLSX_CORE_NET_VLSM_ALLOC_H

#include <stdbool.h>

#include "common/result.h"
#include "core/net/vlsm/vlsm_types.h"

bool vlsm_plan(const char *cidr_text, const char *hosts_text, vlsm_result_t *out,
               calc_status_t *status);

#endif /* VLSX_CORE_NET_VLSM_ALLOC_H */
