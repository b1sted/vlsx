/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_DHCP_DHCP_CALC_H
#define VLSX_CORE_NET_DHCP_DHCP_CALC_H

#include <stdbool.h>
#include <stdint.h>

#include "common/result.h"

typedef struct {
    uint32_t lease_minutes;
    double t0_minutes;
    double t1_minutes;
    double t2_minutes;
} dhcp_timer_result_t;

bool dhcp_validate_lease(const char *text);
bool dhcp_accept_lease(char ch);

bool dhcp_timers_calculate(const char *lease_text, dhcp_timer_result_t *out, calc_status_t *status);

#endif /* VLSX_CORE_NET_DHCP_DHCP_CALC_H */
